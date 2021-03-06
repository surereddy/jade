/*
 * user_handler.c
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */


#include <event.h>
#include <string.h>

#include "slog.h"
#include "common.h"
#include "utils.h"

#include "http_handler.h"
#include "resource_handler.h"
#include "sip_handler.h"

#include "user_handler.h"

extern app* g_app;

#define DEF_DB_TABLE_USER_CONTACT     "user_contact"
#define DEF_DB_TABLE_USER_USERINFO    "user_userinfo"
#define DEF_DB_TABLE_USER_GROUP       "user_group"
#define DEF_DB_TABLE_USER_PERMISSION  "user_permission"
#define DEF_DB_TABLE_USER_AUTHTOKEN   "user_authtoken"


#define DEF_AUTHTOKEN_TIMEOUT   3600

static struct event* g_ev_validate_authtoken = NULL;

static bool init_user_database_contact(void);
static bool init_user_database_permission(void);
static bool init_user_database_authtoken(void);
static bool init_user_database_userinfo(void);


static char* create_authtoken(const char* username, const char* password);

static bool create_user_userinfo_info(const json_t* j_data);


static bool create_userinfo(json_t* j_data);
static bool update_userinfo(const char* uuid, json_t* j_data);
static bool create_permission(const char* user_uuid, const char* permission);
static bool create_contact(json_t* j_data);
static bool update_contact(const char* uuid, json_t* j_data);

static bool is_user_exist(const char* user_uuid);
static bool is_user_exsit_by_username(const char* username);
static bool is_valid_type_target(const char* type, const char* target);


static void cb_user_validate_authtoken(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);


bool init_user_handler(void)
{
  int ret;
  json_t* j_tmp;
  struct timeval tm_slow;

	slog(LOG_INFO, "Fired init_user_handler.");

	ret = init_user_database_contact();
	if(ret == false) {
	  slog(LOG_ERR, "Could not initiate database for contact.");
	  return false;
	}

	ret = init_user_database_permission();
	if(ret == false) {
	  slog(LOG_ERR, "Could not initiate database for permission.");
	  return false;
	}

	ret = init_user_database_authtoken();
	if(ret == false) {
	  slog(LOG_ERR, "Could not initiate database for authtoken.");
	  return false;
	}

	ret = init_user_database_userinfo();
	if(ret == false) {
	  slog(LOG_ERR, "Could not initiate database for userinfo.");
	  return false;
	}

	// test code..

	// create default user admin
	j_tmp = json_pack("{s:s, s:s}",
	    "username", "admin",
	    "password", "admin"
	    );
	create_userinfo(j_tmp);
	json_decref(j_tmp);


	// create default permission admin
	j_tmp = get_user_userinfo_info_by_username_pass("admin", "admin");
	if(j_tmp != NULL) {
	  create_permission(json_string_value(json_object_get(j_tmp, "uuid")), DEF_USER_PERM_ADMIN);
	  create_permission(json_string_value(json_object_get(j_tmp, "uuid")), DEF_USER_PERM_USER);
	  json_decref(j_tmp);
	}


	// register event
  tm_slow.tv_sec = 10;
  tm_slow.tv_usec = 0;
	g_ev_validate_authtoken = event_new(g_app->evt_base, -1, EV_TIMEOUT | EV_PERSIST, cb_user_validate_authtoken, NULL);
  event_add(g_ev_validate_authtoken, &tm_slow);

	return true;
}

void term_user_handler(void)
{
	slog(LOG_INFO, "Fired term_user_handler.");

	event_del(g_ev_validate_authtoken);
	event_free(g_ev_validate_authtoken);

	return;
}

bool reload_user_handler(void)
{
	slog(LOG_INFO, "Fired reload_user");

	term_user_handler();
	init_user_handler();

	return true;
}

/**
 * Initiate contact database.
 * @return
 */
static bool init_user_database_contact(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_USER_CONTACT " ("

    "   uuid        varchar(255),"
    "   user_uuid   varchar(255),"

    "   type    varchar(255),"    // sip_peer, pjsip_endpoint, ...
    "   target  varchar(255),"    // peer name, endpoint name, ...

    "   name    varchar(255),"
    "   detail  varchar(1023),"

    // timestamp. UTC."
    "   tm_create         datetime(6),"   // create time
    "   tm_update         datetime(6),"   // update time.

    "   primary key(uuid)"
    ");";

  // execute
  ret = exec_jade_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_CONTACT);
    return false;
  }

  return true;
}

/**
 * Initiate permission database.
 * @return
 */
static bool init_user_database_permission(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_USER_PERMISSION " ("

    "   user_uuid   varchar(255),"
    "   permission  varchar(255)"

    ");";

  // execute
  ret = exec_jade_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_PERMISSION);
    return false;
  }

  return true;
}

/**
 * Initiate authtoken database.
 * @return
 */
static bool init_user_database_authtoken(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_USER_AUTHTOKEN " ("

    // identity
    "   uuid        varchar(255),"

    "   user_uuid   varchar(255),"    // user uuid

    // timestamp. UTC."
    "   tm_create         datetime(6),"   // create time
    "   tm_update         datetime(6),"   // update time.

    "   primary key(uuid)"
    ");";

  // execute
  ret = exec_jade_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_AUTHTOKEN);
    return false;
  }

  return true;
}

/**
 * Initiate userinfo database.
 * @return
 */
static bool init_user_database_userinfo(void)
{
  int ret;
  const char* create_table;

  create_table =
      "create table if not exists " DEF_DB_TABLE_USER_USERINFO " ("

      // identity
      "   uuid        varchar(255),"
      "   username    varchar(255),"
      "   password    varchar(255),"

      // info
      "   name      varchar(255),"

      // timestamp. UTC."
      "   tm_create         datetime(6),"   // create time
      "   tm_update         datetime(6),"   // update time.

      "   primary key(uuid)"
      ");";

  // execute
  ret = exec_jade_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_USERINFO);
    return false;
  }

  return true;
}



/**
 *  @brief  Check the user_authtoken and validate
 */
static void cb_user_validate_authtoken(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_auths;
  json_t* j_auth;
  const char* last_update;
  const char* uuid;
  int idx;
  char* timestamp;
  time_t time_over;
  time_t time_update;

  // get all authtoken info
  j_auths = get_user_authtokens_all();
  if(j_auths == NULL) {
    slog(LOG_ERR, "Could not get authtoken info.");
    return;
  }

  // get curtime
  timestamp = get_utc_timestamp();
  time_over = get_unixtime_from_utc_timestamp(timestamp);
  sfree(timestamp);
  time_over -= DEF_AUTHTOKEN_TIMEOUT;

  json_array_foreach(j_auths, idx, j_auth) {

    uuid = json_string_value(json_object_get(j_auth, "uuid"));

    // get last updated time
    last_update = json_string_value(json_object_get(j_auth, "tm_update"));
    if(last_update == NULL) {
      slog(LOG_ERR, "Could not get tm_update info. Remove authtoken. uuid[%s]", uuid);
      delete_user_authtoken_info(uuid);
      continue;
    }

    // convert
    time_update = get_unixtime_from_utc_timestamp(last_update);
    if(time_update == 0) {
      slog(LOG_ERR, "Could not convert tm_update info. Remove authtoken. uuid[%s]", uuid);
      delete_user_authtoken_info(uuid);
      continue;
    }

    // check timeout
    if(time_update < time_over) {
      slog(LOG_NOTICE, "The authtoken is timed out. Remove authtoken. uuid[%s]", uuid);
      delete_user_authtoken_info(uuid);
      continue;
    }
  }

  json_decref(j_auths);
  return;
}

/**
 * htp request handler.
 * request: POST ^/user/login$
 * @param req
 * @param data
 */
void htp_post_user_login(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* username;
  char* password;
  char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_entries.");

  // get username/pass
  ret = http_get_htp_id_pass(req, &username, &password);
  if(ret == false) {
    sfree(username);
    sfree(password);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create authtoken
  authtoken = create_authtoken(username, password);
  sfree(username);
  sfree(password);
  if(authtoken == NULL) {
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  j_tmp = json_pack("{s:s}",
      "authtoken",  authtoken
      );
  sfree(authtoken);

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: DELETE ^/user/login$
 * @param req
 * @param data
 */
void htp_delete_user_login(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_login.");

  // get authtoken
  authtoken = evhtp_kv_find(req->uri->query, "authtoken");
  if(authtoken == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete authtoken
  ret = delete_user_authtoken_info(authtoken);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_res = http_create_default_result(EVHTP_RES_OK);
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/user/contacts request handler.
 * @param req
 * @param data
 */
void htp_get_user_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_contacts.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_user_contacts_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/user/contacts
 * @param req
 * @param data
 */
void htp_post_user_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_user_contacts.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create contact
  ret = create_contact(j_data);
  json_decref(j_data);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_contacts_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_user_contact_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find contact info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void htp_put_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_user_contacts_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "Could not get data info.");
    sfree(detail);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update
  ret = update_contact(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * DELETE ^/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_contacts_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_user_contact_info(detail);
  sfree(detail);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/user/users
 * @param req
 * @param data
 */
void htp_post_user_users(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_user_users.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create user
  ret = create_userinfo(j_data);
  json_decref(j_data);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET ^/user/users
 * @param req
 * @param data
 */
void htp_get_user_users(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_users.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_user_userinfos_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_users_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_user_userinfo_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find userinfo info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void htp_put_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_user_users_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "Could not get data info.");
    sfree(detail);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update
  ret = update_userinfo(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * DELETE ^/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_users_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, DEF_USER_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_user_userinfo_info(detail);
  sfree(detail);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}


static char* create_authtoken(const char* username, const char* password)
{
  json_t* j_user;
  json_t* j_auth;
  char* token;
  char* timestamp;

  if((username == NULL) || (password == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired create_authtoken. username[%s], password[%s]", username, "*");

  // get user info
  j_user = get_user_userinfo_info_by_username_pass(username, password);
  if(j_user == NULL) {
    slog(LOG_INFO, "Could not find correct userinfo of given data. username[%s], password[%s]", username, "*");
    return NULL;
  }

  // create
  token = gen_uuid();
  timestamp = get_utc_timestamp();
  j_auth = json_pack("{s:s, s:s, s:s, s:s}",
      "uuid",       token,
      "user_uuid",  json_string_value(json_object_get(j_user, "uuid")),

      "tm_create",  timestamp,
      "tm_update",  timestamp
      );
  sfree(timestamp);
  json_decref(j_user);

  // create auth info
  create_user_authtoken_info(j_auth);
  json_decref(j_auth);

  return token;
}

static bool create_permission(const char* user_uuid, const char* permission)
{
  json_t* j_tmp;
  int ret;

  if((user_uuid == NULL) || (permission == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_pack("{s:s, s:s}",
      "user_uuid",  user_uuid,
      "permission", permission
      );

  ret = create_user_permission_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool create_contact(json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  char* uuid;
  const char* user_uuid;
  const char* type;
  const char* target;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_contact.");

  // validate user_uuid
  user_uuid = json_string_value(json_object_get(j_data, "user_uuid"));
  ret = is_user_exist(user_uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "User is not exist. user_uuid[%s]", user_uuid);
    return false;
  }

  // validate type target
  type = json_string_value(json_object_get(j_data, "type"));
  target = json_string_value(json_object_get(j_data, "target"));
  ret = is_valid_type_target(type, target);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not pass the type target validation. type[%s], target[%s]", type?:"", target?:"");
    return false;
  }

  // create contact info
  timestamp = get_utc_timestamp();
  uuid = gen_uuid();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:s "
      "}",

      "uuid",         uuid,
      "user_uuid",    user_uuid,

      "type",         type,
      "target",       target,
      "name",         json_string_value(json_object_get(j_data, "name"))? : "",
      "detail",       json_string_value(json_object_get(j_data, "detail"))? : "",

      "tm_create",    timestamp
      );
  sfree(timestamp);
  sfree(uuid);

  // create resource
  ret = create_user_contact_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create user contact info.");
    return false;
  }

  return true;
}

static bool update_contact(const char* uuid, json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  int ret;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_contact. uuid[%s]", uuid);

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s "
      "}",

      "uuid",       uuid,
      "user_uuid",  json_string_value(json_object_get(j_data, "user_uuid"))? : "",

      "type",     json_string_value(json_object_get(j_data, "type"))? : "",
      "target",   json_string_value(json_object_get(j_data, "target"))? : "",

      "name",     json_string_value(json_object_get(j_data, "name"))? : "",
      "detail",   json_string_value(json_object_get(j_data, "detail"))? : "",

      "tm_update", timestamp
      );
  sfree(timestamp);

  ret = update_user_contact_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update contact info.");
    return false;
  }

  return true;
}

/**
 * Create user info.
 * @param j_data
 * @return
 */
static bool create_userinfo(json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  char* uuid;
  const char* username;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user.");

  // validate info.
  username = json_string_value(json_object_get(j_data, "username"));
  if(username == NULL) {
    slog(LOG_ERR, "Could not get username info.");
    return false;
  }

  // check exist
  ret = is_user_exsit_by_username(username);
  if(ret == true) {
    slog(LOG_ERR, "The given username is already exist. username[%s]", username);
    return false;
  }

  // create contact info
  timestamp = get_utc_timestamp();
  uuid = gen_uuid();
  j_tmp = json_pack("{"
      "s:s, "
      "s:s, s:s, "
      "s:s, "
      "s:s "
      "}",

      "uuid",         uuid,

      "username",     username,
      "password",     json_string_value(json_object_get(j_data, "password"))? : "",

      "name",         json_string_value(json_object_get(j_data, "name"))? : "",

      "tm_create",    timestamp
      );
  sfree(timestamp);
  sfree(uuid);

  // create resource
  ret = create_user_userinfo_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create user contact info.");
    return false;
  }

  return true;
}

/**
 * Update userinfo.
 * @param uuid
 * @param j_data
 * @return
 */
static bool update_userinfo(const char* uuid, json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  int ret;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_contact. uuid[%s]", uuid);

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, "
      "s:s, s:s, s:s, "
      "s:s "
      "}",

      "uuid",       uuid,

      "username",  json_string_value(json_object_get(j_data, "username"))? : "",
      "password",  json_string_value(json_object_get(j_data, "password"))? : "",
      "name",  json_string_value(json_object_get(j_data, "name"))? : "",

      "tm_update", timestamp
      );
  sfree(timestamp);

  ret = update_user_userinfo_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update contact info.");
    return false;
  }

  return true;
}

static bool is_user_exist(const char* user_uuid)
{
  json_t* j_tmp;

  if(user_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = get_user_userinfo_info(user_uuid);
  if(j_tmp == NULL) {
    return false;
  }

  json_decref(j_tmp);
  return true;
}

static bool is_user_exsit_by_username(const char* username)
{
  json_t* j_tmp;

  if(username == NULL) {
    slog(LOG_WARNING, "Wrong input paramter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired is_user_exsit_by_username. username[%s]", username);

  j_tmp = get_user_userinfo_info_by_username(username);
  if(j_tmp == NULL) {
    return false;
  }

  json_decref(j_tmp);
  return true;
}

static bool is_valid_type_target(const char* type, const char* target)
{
  json_t* j_tmp;

  if((type == NULL) || (target == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // validate
  if(strcmp(type, DEF_USER_CONTACT_TYPE_PEER) == 0) {
    // peer type

    j_tmp = get_sip_peer_info(target);
    if(j_tmp == NULL) {
      return false;
    }

    json_decref(j_tmp);
    return true;
  }
  else if(strcmp(type, DEF_USER_CONTACT_TYPE_ENDPOINT) == 0) {
    // pjsip type

    j_tmp = get_pjsip_endpoint_info(target);
    if(j_tmp == NULL) {
      return false;
    }

    json_decref(j_tmp);
    return true;
  }
  else {
    // wrong type

    return false;
  }

  // should not reach to here
  slog(LOG_ERR, "Should not reach to here. Something was wrong.");
  return false;
}

/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* get_user_userinfo_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_info. key[%s]", key);

  j_res = get_jade_detail_item_key_string("user_userinfo", "uuid", key);

  return j_res;
}

/**
 * Get all user_userinfo detail info.
 * @return
 */
json_t* get_user_userinfos_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_user_userinfos_all.");

  j_res = get_jade_items(DEF_DB_TABLE_USER_USERINFO, "*");

  return j_res;
}


/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* get_user_userinfo_info_by_username(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_info_by_username. key[%s]", key);

  j_res = get_jade_detail_item_key_string("user_userinfo", "username", key);

  return j_res;
}

/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* get_user_userinfo_info_by_username_pass(const char* username, const char* pass)
{
  json_t* j_res;
  json_t* j_obj;

  if((username == NULL) || (pass == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_info_by_username_pass. username[%s], pass[%s]", username, "*");

  j_obj = json_pack("{s:s, s:s}",
      "username",   username,
      "password",   pass
      );

  j_res = get_jade_detail_item_by_obj("user_userinfo", j_obj);
  json_decref(j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* get_user_userinfo_by_authtoken(const char* authtoken)
{
  json_t* j_auth;
  json_t* j_user;
  const char* user_uuid;

  if(authtoken == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_by_authtoken. authtoken[%s]", authtoken);

  j_auth = get_user_authtoken_info(authtoken);
  if(j_auth == NULL) {
    slog(LOG_ERR, "Could not get authtoken info.");
    return NULL;
  }

  user_uuid = json_string_value(json_object_get(j_auth, "user_uuid"));
  if(user_uuid == NULL) {
    slog(LOG_ERR, "Could not get user_uuid.");
    json_decref(j_auth);
    return NULL;
  }

  j_user = get_user_userinfo_info(user_uuid);
  json_decref(j_auth);
  if(j_user == NULL) {
    slog(LOG_ERR, "Could not get user info.");
    return NULL;
  }

  return j_user;
}

static bool create_user_userinfo_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_userinfo_info.");

  // insert userinfo info
  ret = insert_jade_item("user_userinfo", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_userinfo.");
    return false;
  }

  return true;
}

bool update_user_userinfo_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_userinfo_info.");

  // update info
  ret = update_jade_item("user_userinfo", "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update user_userinfo info.");
    return false;
  }

  return true;
}

/**
 * Delete user_userinfo info.
 * @return
 */
bool delete_user_userinfo_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_userinfo_info. key[%s]", key);

  ret = delete_jade_items_string("user_userinfo", "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete dp_dialplan info. key[%s]", key);
    return false;
  }

  return true;
}

/**
 * Get corresponding user_authtoken all detail info.
 * @return
 */
json_t* get_user_authtokens_all(void)
{
  json_t* j_res;

  j_res = get_jade_items("user_authtoken", "*");
  return j_res;
}

/**
 * Get corresponding user_authtoken detail info.
 * @return
 */
json_t* get_user_authtoken_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_authtoken_info. key[%s]", key);

  j_res = get_jade_detail_item_key_string("user_authtoken", "uuid", key);

  return j_res;
}

bool create_user_authtoken_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_authtoken_info.");

  // insert authtoken info
  ret = insert_jade_item("user_authtoken", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_authtoken.");
    return false;
  }

  return true;
}

bool update_user_authtoken_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_user_authtoken_info.");

  // update info
  ret = update_jade_item("user_authtoken", "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update user_authtoken info.");
    return false;
  }

  return true;
}

bool update_user_authtoken_tm_update(const char* uuid)
{
  json_t* j_token;
  char* timestamp;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_token = get_user_authtoken_info(uuid);
  if(j_token == NULL) {
    slog(LOG_NOTICE, "Could not get authtoken info. uuid[%s]", uuid);
    return false;
  }

  timestamp = get_utc_timestamp();
  json_object_set_new(j_token, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = update_user_authtoken_info(j_token);
  json_decref(j_token);
  if(ret == false) {
    slog(LOG_ERR, "Could not update authtoken info. uuid[%s]", uuid);
    return false;
  }

  return true;
}


/**
 * Delete user_authtoken info.
 * @param key
 * @return
 */
bool delete_user_authtoken_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_authtoken_info. key[%s]", key);

  ret = delete_jade_items_string("user_authtoken", "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete user_authtoken info. key[%s]", key);
    return false;
  }

  return true;
}

bool create_user_permission_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_permission_info.");

  // insert authtoken info
  ret = insert_jade_item("user_permission", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_authtoken.");
    return false;
  }

  return true;
}

/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* get_user_permission_info_by_useruuid_perm(const char* useruuid, const char* perm)
{
  json_t* j_res;
  json_t* j_obj;

  if((useruuid == NULL) || (perm == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_permission_info_by_useruuid_perm. useruuid[%s], perm[%s]", useruuid, perm);

  j_obj = json_pack("{s:s, s:s}",
      "user_uuid",    useruuid,
      "permission",   perm
      );

  j_res = get_jade_detail_item_by_obj("user_permission", j_obj);
  json_decref(j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* get_user_contacts_all(void)
{
  json_t* j_res;

  j_res = get_jade_items("user_contact", "*");
  return j_res;
}

/**
 * Returns conntact info array of given user_uuid.
 * @param user_uuid
 * @return
 */
json_t* get_user_contacts_by_user_uuid(const char* user_uuid)
{
  json_t* j_res;
  json_t* j_obj;

  j_obj = json_pack("{s:s}",
      "user_uuid",  user_uuid
      );

  j_res = get_jade_detail_items_by_obj("user_contact", j_obj);
  json_decref(j_obj);

  return j_res;
}

/**
 * Get corresponding user_contact detail info.
 * @return
 */
json_t* get_user_contact_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_contact_info. key[%s]", key);

  j_res = get_jade_detail_item_key_string("user_contact", "uuid", key);

  return j_res;
}

bool create_user_contact_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_contact_info.");

  // insert info
  ret = insert_jade_item("user_contact", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_contact.");
    return false;
  }

  return true;
}

bool update_user_contact_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_user_contact_info.");

  // update info
  ret = update_jade_item("user_contact", "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update user_contact info.");
    return false;
  }

  return true;
}

/**
 * Delete user_contact info.
 * @param key
 * @return
 */
bool delete_user_contact_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_contact_info. key[%s]", key);

  ret = delete_jade_items_string("user_contact", "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete user_contact info. key[%s]", key);
    return false;
  }

  return true;
}


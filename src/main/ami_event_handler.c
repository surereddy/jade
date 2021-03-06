/*
 * ami_event_handler.c
 *
 *  Created on: Mar 4, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <string.h>
#include <jansson.h>

#include "utils.h"
#include "slog.h"
#include "common.h"
#include "ami_handler.h"
#include "action_handler.h"
#include "ami_response_handler.h"
#include "resource_handler.h"

#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"
#include "dialplan_handler.h"
#include "park_handler.h"
#include "queue_handler.h"
#include "pjsip_handler.h"
#include "sip_handler.h"

static void ami_response_handler(json_t* j_msg);


static void ami_event_agentlogin(json_t* j_msg);
static void ami_event_agentlogoff(json_t* j_msg);
static void ami_event_agents(json_t* j_msg);
static void ami_event_asyncagiend(json_t* j_msg);
static void ami_event_asyncagiexec(json_t* j_msg);
static void ami_event_asyncagistart(json_t* j_msg);
static void ami_event_aordetail(json_t* j_msg);
static void ami_event_authdetail(json_t* j_msg);
static void ami_event_contactstatus(json_t* j_msg);
static void ami_event_contactstatusdetail(json_t* j_msg);
static void ami_event_coreshowchannel(json_t* j_msg);
static void ami_event_devicestatechange(json_t* j_msg);
static void ami_event_dialbegin(json_t* j_msg);
static void ami_event_dialend(json_t* j_msg);
static void ami_event_endpointdetail(json_t* j_msg);
static void ami_event_endpointlist(json_t* j_msg);
static void ami_event_hangup(json_t* j_msg);
static void ami_event_newchannel(json_t* j_msg);
static void ami_event_newexten(json_t* j_msg);
static void ami_event_newstate(json_t* j_msg);
static void ami_event_originateresponse(json_t* j_msg);
static void ami_event_parkedcall(json_t* j_msg);
static void ami_event_parkedcallgiveup(json_t* j_msg);
static void ami_event_parkedcallswap(json_t* j_msg);
static void ami_event_parkedcalltimeout(json_t* j_msg);
static void ami_event_parkinglot(json_t* j_msg);
static void ami_event_peerentry(json_t* j_msg);
static void ami_event_peerstatus(json_t* j_msg);
static void ami_event_queuecallerabandon(json_t* j_msg);
static void ami_event_queuecallerjoin(json_t* j_msg);
static void ami_event_queuecallerleave(json_t* j_msg);
static void ami_event_queueentry(json_t* j_msg);
static void ami_event_queuemember(json_t* j_msg);
static void ami_event_queuememberadded(json_t* j_msg);
static void ami_event_queuememberstatus(json_t* j_msg);
static void ami_event_queuememberpause(json_t* j_msg);
static void ami_event_queuememberpenalty(json_t* j_msg);
static void ami_event_queuememberremoved(json_t* j_msg);
static void ami_event_queuememberringinuse(json_t* j_msg);
static void ami_event_queueparams(json_t* j_msg);
static void ami_event_registryentry(json_t* j_msg);
static void ami_event_rename(json_t* j_msg);
static void ami_event_reload(json_t* j_msg);
static void ami_event_unparkedcall(json_t* j_msg);
static void ami_event_varset(json_t* j_msg);
static void ami_event_voicemailuserentry(json_t* j_msg);


// action response handlers
//static ACTION_RES ami_response_handler_databaseshowall(json_t* j_action, json_t* j_msg);

/**
 * Event message handler
 * @param msg
 */
void ami_message_handler(const char* msg)
{
  json_t* j_msg;
  char* tmp;
  const char* event;
  const char* action_id;

  if(msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_message_handler.");

  // message parse
  j_msg = parse_ami_msg(msg);
  if(j_msg == NULL) {
    slog(LOG_NOTICE, "Could not parse message. msg[%s]", msg);
    return;
  }

  // get action id
  // if there's action id,
  // consider the response of action request.
  action_id = json_string_value(json_object_get(j_msg, "ActionID"));
  if(action_id != NULL) {
    ami_response_handler(j_msg);
    json_decref(j_msg);
    return;
  }

  event = json_string_value(json_object_get(j_msg, "Event"));
  if(event == NULL) {
    json_decref(j_msg);
    return;
  }
  slog(LOG_DEBUG, "Get event info. event[%s]", event);

  if(strcmp(event, "AgentCalled") == 0) {
    // need to do something later
  }
  else if(strcasecmp(event, "AgentLogin") == 0) {
    ami_event_agentlogin(j_msg);
  }
  else if(strcasecmp(event, "AgentLogoff") == 0) {
    ami_event_agentlogoff(j_msg);
  }
  else if(strcasecmp(event, "Agents") == 0) {
    ami_event_agents(j_msg);
  }
  else if(strcasecmp(event, "AsyncAGIEnd") == 0) {
    ami_event_asyncagiend(j_msg);
  }
  else if(strcasecmp(event, "AsyncAGIExec") == 0) {
    ami_event_asyncagiexec(j_msg);
  }
  else if(strcasecmp(event, "AsyncAGIStart") == 0) {
    ami_event_asyncagistart(j_msg);
  }
  else if(strcasecmp(event, "AorDetail") == 0) {
    ami_event_aordetail(j_msg);
  }
  else if(strcasecmp(event, "AuthDetail") == 0) {
    ami_event_authdetail(j_msg);
  }
  else if(strcasecmp(event, "ContactStatus") == 0) {
    ami_event_contactstatus(j_msg);
  }
  else if(strcasecmp(event, "ContactStatusDetail") == 0) {
    ami_event_contactstatusdetail(j_msg);
  }
  else if(strcasecmp(event, "CoreShowChannel") == 0) {
    ami_event_coreshowchannel(j_msg);
  }
  else if(strcasecmp(event, "DeviceStateChange") == 0) {
    ami_event_devicestatechange(j_msg);
  }
  else if(strcasecmp(event, "DialBegin") == 0) {
    ami_event_dialbegin(j_msg);
  }
  else if(strcasecmp(event, "DialEnd") == 0) {
    ami_event_dialend(j_msg);
  }
  else if(strcasecmp(event, "EndpointDetail") == 0) {
    ami_event_endpointdetail(j_msg);
  }
  else if(strcasecmp(event, "EndpointList") == 0) {
    ami_event_endpointlist(j_msg);
  }
  else if(strcasecmp(event, "Hangup") == 0) {
    ami_event_hangup(j_msg);
  }
  else if(strcasecmp(event, "NewChannel") == 0) {
    ami_event_newchannel(j_msg);
  }
  else if(strcasecmp(event, "Newexten") == 0) {
    ami_event_newexten(j_msg);
  }
  else if(strcasecmp(event, "Newstate") == 0) {
    ami_event_newstate(j_msg);
  }
  else if(strcasecmp(event, "OriginateResponse") == 0) {
    ami_event_originateresponse(j_msg);
  }
  else if(strcasecmp(event, "ParkedCall") == 0) {
    ami_event_parkedcall(j_msg);
  }
  else if(strcasecmp(event, "ParkedCallGiveUp") == 0) {
    ami_event_parkedcallgiveup(j_msg);
  }
  else if(strcasecmp(event, "ParkedCallSwap") == 0) {
    ami_event_parkedcallswap(j_msg);
  }
  else if(strcasecmp(event, "ParkedCallTimeOut") == 0) {
    ami_event_parkedcalltimeout(j_msg);
  }
  else if(strcasecmp(event, "ParkingLot") == 0) {
    ami_event_parkinglot(j_msg);
  }
  else if(strcasecmp(event, "PeerEntry") == 0) {
    ami_event_peerentry(j_msg);
  }
  else if(strcasecmp(event, "PeerStatus") == 0) {
    ami_event_peerstatus(j_msg);
  }
  else if(strcasecmp(event, "QueueCallerAbandon") == 0) {
    ami_event_queuecallerabandon(j_msg);
  }
  else if(strcasecmp(event, "QueueCallerJoin") == 0) {
    ami_event_queuecallerjoin(j_msg);
  }
  else if(strcasecmp(event, "QueueCallerLeave") == 0) {
    ami_event_queuecallerleave(j_msg);
  }
  else if(strcasecmp(event, "QueueEntry") == 0) {
    ami_event_queueentry(j_msg);
  }
  else if(strcasecmp(event, "QueueMember") == 0) {
    ami_event_queuemember(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberAdded") == 0) {
    ami_event_queuememberadded(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberStatus") == 0) {
    ami_event_queuememberstatus(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberPause") == 0) {
    ami_event_queuememberpause(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberPenalty") == 0) {
    ami_event_queuememberpenalty(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberRemoved") == 0) {
    ami_event_queuememberremoved(j_msg);
  }
  else if(strcasecmp(event, "QueueMemberRinginuse") == 0) {
    ami_event_queuememberringinuse(j_msg);
  }
  else if(strcasecmp(event, "QueueParams") == 0) {
    ami_event_queueparams(j_msg);
  }
  else if(strcasecmp(event, "RegistryEntry") == 0) {
    ami_event_registryentry(j_msg);
  }
  else if(strcasecmp(event, "Rename") == 0) {
    ami_event_rename(j_msg);
  }
  else if(strcasecmp(event, "Reload") == 0) {
    ami_event_reload(j_msg);
  }
  else if(strcasecmp(event, "UnParkedCall") == 0) {
    ami_event_unparkedcall(j_msg);
  }
  else if(strcasecmp(event, "VarSet") == 0) {
    ami_event_varset(j_msg);
  }
  else if(strcasecmp(event, "VoicemailUserEntry") == 0) {
    ami_event_voicemailuserentry(j_msg);
  }
  else {
    tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
    slog(LOG_DEBUG, "Could not find correct message parser. msg[%s]", tmp);
    sfree(tmp);
  }

  json_decref(j_msg);

  return;
}

static void ami_response_handler(json_t* j_msg)
{
  const char* action_id;
  const char* type;
  json_t* j_action;
  ACTION_RES res_action;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_response_handler.");

  action_id = json_string_value(json_object_get(j_msg, "ActionID"));
  if(action_id == NULL) {
    slog(LOG_NOTICE, "Could not get ActionID.");
    return;
  }

  // get action
  j_action = get_action(action_id);
  if(j_action == NULL) {
    slog(LOG_NOTICE, "Could not get action info. id[%s]", action_id);
    return;
  }

  type = json_string_value(json_object_get(j_action, "type"));
  if(type == NULL) {
    slog(LOG_ERR, "Could not get action type info. id[%s]", action_id);
    json_decref(j_action);

    // wrong action. should be deleted.
    delete_action(action_id);
    return;
  }

  //// Action response parse.

  if(strcasecmp(type, "coresettings") == 0) {
    res_action = ami_response_handler_coresettings(j_action, j_msg);
  }
  else if(strcasecmp(type, "corestatus") == 0) {
    res_action = ami_response_handler_corestatus(j_action, j_msg);
  }
  else if(strcasecmp(type, "modulecheck") == 0) {
    res_action = ami_response_handler_modulecheck(j_action, j_msg);
  }
  else if(strcasecmp(type, "moduleload") == 0) {
    res_action = ami_response_handler_moduleload(j_action, j_msg);
  }

  // outbound
  else if(strcasecmp(type, "ob.originate") == 0) {
    res_action = ob_ami_response_handler_originate(j_action, j_msg);
  }
  else if(strcasecmp(type, "ob.status") == 0) {
    res_action = ob_ami_response_handler_status(j_action, j_msg);
  }
  else {
    slog(LOG_ERR, "Could not find correct action response handler. action_id[%s], type[%s]", action_id, type);
    res_action = ACTION_RES_ERROR;
  }
  // End action response parse.
  json_decref(j_action);


  // result check
  if(res_action == ACTION_RES_CONTINUE) {
    slog(LOG_DEBUG, "The action response is not finished. Waiting for next event. action_id[%s]", action_id);
    // continue
    return;
  }
  slog(LOG_DEBUG, "The action response finished. action_id[%s], res[%d]", action_id, res_action);

  // delete action
  delete_action(action_id);
  return;
}

/**
 * AMI event handler.
 * Event: PeerEntry
 * @param j_msg
 */
static void ami_event_peerentry(json_t* j_msg)
{
  json_t* j_tmp;
  char* address;
  char* timestamp;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_peerentry.");

  // create address
  asprintf(&address, "%s:%s",
      json_string_value(json_object_get(j_msg, "IPaddress")),
      json_string_value(json_object_get(j_msg, "IPport"))
      );

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
    "s:s, s:s, "
    "s:s, s:s, s:s, "
    "s:s, s:s, s:s, s:s, s:s, s:s, s:s, "
    "s:s, s:s, s:s, "
    "s:s"
    "}",

    "peer",     json_string_value(json_object_get(j_msg, "ObjectName"))? : "",
    "address",  address,

    "channel_type", json_string_value(json_object_get(j_msg, "Channeltype"))? : "",
    "chan_object_type", json_string_value(json_object_get(j_msg, "ChanObjectType"))? : "",
    "monitor_status",   json_string_value(json_object_get(j_msg, "Status"))? : "",

    "dynamic",          json_string_value(json_object_get(j_msg, "Dynamic"))? : "",
    "auto_force_port",  json_string_value(json_object_get(j_msg, "AutoForcerport"))? : "",
    "force_port",       json_string_value(json_object_get(j_msg, "Forcerport"))? : "",
    "auto_comedia",     json_string_value(json_object_get(j_msg, "AutoComedia"))? : "",
    "comedia",          json_string_value(json_object_get(j_msg, "Comedia"))? : "",
    "video_support",    json_string_value(json_object_get(j_msg, "VideoSupport"))? : "",
    "text_support",     json_string_value(json_object_get(j_msg, "TextSupport"))? : "",

    "acl",              json_string_value(json_object_get(j_msg, "ACL"))? : "",
    "realtime_device",  json_string_value(json_object_get(j_msg, "RealtimeDevice"))? : "",
    "description",      json_string_value(json_object_get(j_msg, "Description"))? : "",

    "tm_update",  timestamp
  );
  sfree(timestamp);
  sfree(address);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create info
  ret = create_sip_peer_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert destination.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: PeerStatus
 * @param j_msg
 */
static void ami_event_peerstatus(json_t* j_msg)
{
  int ret;
  const char* tmp_const;
  char* peer;
  json_t* j_tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  // get peer
  tmp_const = json_string_value(json_object_get(j_msg, "Peer"));
  peer = strdup(tmp_const + 4); // "SIP/"
  if(peer == NULL) {
    slog(LOG_ERR, "Could not get peer.");
    return;
  }

  // create update info
  j_tmp = json_pack("{s:s, s:s, s:s, s:s}",
      "peer",           peer,
      "status",         json_string_value(json_object_get(j_msg, "PeerStatus"))? : "",
      "address",        json_string_value(json_object_get(j_msg, "Address"))? : "",
      "channel_type",   json_string_value(json_object_get(j_msg, "ChannelType"))? : ""
      );
  sfree(peer);

  // update info
  ret = update_sip_peer_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update sip peer info.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueParams
 * @param j_msg
 */
static void ami_event_queueparams(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queueparams.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, "
      "s:i, s:s, s:i, s:i, s:i, s:i, s:i, "
      "s:i, s:f, s:i, "
      "s:s"
      "}",

      "name",       json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "max",        json_string_value(json_object_get(j_msg, "Max"))? atoi(json_string_value(json_object_get(j_msg, "Max"))) : 0,
      "strategy",   json_string_value(json_object_get(j_msg, "Strategy"))? : "",
      "calls",      json_string_value(json_object_get(j_msg, "Calls"))? atoi(json_string_value(json_object_get(j_msg, "Calls"))) : 0,
      "hold_time",  json_string_value(json_object_get(j_msg, "Holdtime"))? atoi(json_string_value(json_object_get(j_msg, "Holdtime"))) : 0,
      "talk_time",  json_string_value(json_object_get(j_msg, "TalkTime"))? atoi(json_string_value(json_object_get(j_msg, "TalkTime"))) : 0,
      "completed",  json_string_value(json_object_get(j_msg, "Completed"))? atoi(json_string_value(json_object_get(j_msg, "Completed"))) : 0,
      "abandoned",  json_string_value(json_object_get(j_msg, "Abandoned"))? atoi(json_string_value(json_object_get(j_msg, "Abandoned"))) : 0,

      "service_level",      json_string_value(json_object_get(j_msg, "ServiceLevel"))? atoi(json_string_value(json_object_get(j_msg, "ServiceLevel"))) : 0,
      "service_level_perf", json_string_value(json_object_get(j_msg, "ServicelevelPerf"))? atof(json_string_value(json_object_get(j_msg, "ServicelevelPerf"))) : 0.0,
      "weight",             json_string_value(json_object_get(j_msg, "Weight"))? atoi(json_string_value(json_object_get(j_msg, "Weight"))) : 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create queue info
  ret = create_queue_param_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_param.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMember
 * @param j_msg
 */
static void ami_event_queuemember(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* id;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuemember.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "Name"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Location"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create and set id
  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_tmp, "name")),
      json_string_value(json_object_get(j_tmp, "queue_name"))
      );
  json_object_set_new(j_tmp, "id", json_string(id));
  sfree(id);

  ret = create_queue_member_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberAdded
 * @param j_msg
 */
static void ami_event_queuememberadded(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* id;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberadded.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create and set id
  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_tmp, "name")),
      json_string_value(json_object_get(j_tmp, "queue_name"))
      );
  json_object_set_new(j_tmp, "id", json_string(id));
  sfree(id);

  ret = create_queue_member_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberPause
 * @param j_msg
 */
static void ami_event_queuememberpause(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* id;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberpause.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create and set id
  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_tmp, "name")),
      json_string_value(json_object_get(j_tmp, "queue_name"))
      );
  json_object_set_new(j_tmp, "id", json_string(id));
  sfree(id);

  ret = create_queue_member_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberPenalty
 * @param j_msg
 */
static void ami_event_queuememberpenalty(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* id;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberpenalty.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create and set id
  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_tmp, "name")),
      json_string_value(json_object_get(j_tmp, "queue_name"))
      );
  json_object_set_new(j_tmp, "id", json_string(id));
  sfree(id);

  ret = create_queue_member_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberRemoved
 * @param j_msg
 */
static void ami_event_queuememberremoved(json_t* j_msg)
{
  char* id;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuecallerabandon.");

  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_msg, "MemberName")),
      json_string_value(json_object_get(j_msg, "Queue"))
      );

  ret = delete_queue_member_info(id);
  sfree(id);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberRinginuse
 * @param j_msg
 */
static void ami_event_queuememberringinuse(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* id;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuememberringinuse.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, s:i, "
      "s:s, s:i, "
      "s:s"
      "}",

      "name",         json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "queue_name",   json_string_value(json_object_get(j_msg, "Queue"))? : "",

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))) : 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "CallsTaken"))) : 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))) : 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))) : 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))) : 0,
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))) : 0,
      "paused",     json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))) : 0,

      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",
      "ring_inuse",     json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create and set id
  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_tmp, "name")),
      json_string_value(json_object_get(j_tmp, "queue_name"))
      );
  json_object_set_new(j_tmp, "id", json_string(id));
  sfree(id);

  ret = create_queue_member_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}


/**
 * AMI event handler.
 * Event: QueueEntry
 * @param j_msg
 */
static void ami_event_queueentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queueentry.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:i, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:i, "
      "s:s"
      "}",

      "queue_name", json_string_value(json_object_get(j_msg, "Queue"))? : "",
      "position",   json_string_value(json_object_get(j_msg, "Position"))? atoi(json_string_value(json_object_get(j_msg, "Position"))) : 0,

      "channel",              json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "unique_id",            json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "wait",   json_string_value(json_object_get(j_msg, "Wait"))? atoi(json_string_value(json_object_get(j_msg, "Wait"))) : 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create queue entry
  ret = create_queue_entry_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueCallerAbandon
 * @param j_msg
 */
static void ami_event_queuecallerabandon(json_t* j_msg)
{
  int ret;
  const char* tmp_const;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuecallerabandon.");

  tmp_const = json_string_value(json_object_get(j_msg, "Uniqueid"));
  ret = delete_queue_entry_info(tmp_const);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue_entry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueCallerJoin
 * @param j_msg
 */
static void ami_event_queuecallerjoin(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_queuecallerjoin.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:i, s:s, s:s, s:s, s:s, s:s, "
      "s:s"
      "}",

      "queue_name",           json_string_value(json_object_get(j_msg, "Queue"))? : "",
      "channel",              json_string_value(json_object_get(j_msg, "Channel"))? : "",

      "position",             json_string_value(json_object_get(j_msg, "Position"))? atoi(json_string_value(json_object_get(j_msg, "Position"))) : 0,
      "unique_id",            json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create queue entry
  ret = create_queue_entry_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert queue_entry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueCallerLeave
 * @param j_msg
 */
static void ami_event_queuecallerleave(json_t* j_msg)
{
  int ret;
  const char* tmp_const;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_queuecallerleave.");

  tmp_const = json_string_value(json_object_get(j_msg, "Uniqueid"));
  ret = delete_queue_entry_info(tmp_const);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete queue_entry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: QueueMemberStatus
 * @param j_msg
 */
static void ami_event_queuememberstatus(json_t* j_msg)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;
  char* id;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_queuememberstatus.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:i, "
      "s:s, s:s, s:s, s:i, s:i, "
      "s:i, s:i, s:i, s:i, "
      "s:i, s:s, "
      "s:s"
      "}",

      "queue_name", json_string_value(json_object_get(j_msg, "Queue"))? : "",
      "name",       json_string_value(json_object_get(j_msg, "MemberName"))? : "",
      "status",     json_string_value(json_object_get(j_msg, "Status"))? atoi(json_string_value(json_object_get(j_msg, "Status"))): 0,

      "location",         json_string_value(json_object_get(j_msg, "Interface"))? : "",
      "state_interface",  json_string_value(json_object_get(j_msg, "StateInterface"))? : "",
      "membership",       json_string_value(json_object_get(j_msg, "Membership"))? : "",
      "penalty",          json_string_value(json_object_get(j_msg, "Penalty"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))): 0,
      "calls_taken",      json_string_value(json_object_get(j_msg, "CallsTaken"))? atoi(json_string_value(json_object_get(j_msg, "Penalty"))): 0,

      "last_call",  json_string_value(json_object_get(j_msg, "LastCall"))? atoi(json_string_value(json_object_get(j_msg, "LastCall"))): 0,
      "ring_inuse", json_string_value(json_object_get(j_msg, "Ringinuse"))? atoi(json_string_value(json_object_get(j_msg, "Ringinuse"))): 0,
      "last_pause", json_string_value(json_object_get(j_msg, "LastPause"))? atoi(json_string_value(json_object_get(j_msg, "LastPause"))): 0,
      "in_call",    json_string_value(json_object_get(j_msg, "InCall"))? atoi(json_string_value(json_object_get(j_msg, "InCall"))): 0,

      "paused",         json_string_value(json_object_get(j_msg, "Paused"))? atoi(json_string_value(json_object_get(j_msg, "Paused"))): 0,
      "paused_reason",  json_string_value(json_object_get(j_msg, "PausedReason"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }
  slog(LOG_INFO, "Update queue member status. queue_name[%s], member_name[%s], status[%lld]",
      json_string_value(json_object_get(j_tmp, "queue_name")),
      json_string_value(json_object_get(j_tmp, "name")),
      json_integer_value(json_object_get(j_tmp, "status"))
      );

  // create and set id
  asprintf(&id, "%s@%s",
      json_string_value(json_object_get(j_tmp, "name")),
      json_string_value(json_object_get(j_tmp, "queue_name"))
      );
  json_object_set_new(j_tmp, "id", json_string(id));
  sfree(id);

  ret = update_queue_member_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to queue_member.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: DialBegin
 * @param j_msg
 */
static void ami_event_dialbegin(json_t* j_msg)
{
//  int hangup;
//  int ret;
//  char* sql;
//  char* tmp;
//  const char* uuid;
//  const char* tmp_const;
//  const char* hangup_detail;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_dialbegin.");

//  // check event
//  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
//  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
//  sfree(tmp);
//
//  // update ob_dialing info
//  uuid = json_string_value(json_object_get(j_msg, "DestUniqueid"));
//  update_ob_dialing_status(uuid, E_DIALING_DIAL_BEGIN);


  return;
}

/**
 * AMI event handler.
 * Event: DialEnd
 * @param j_msg
 */
static void ami_event_dialend(json_t* j_msg)
{
  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_dialend.");

//  // check event
//  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
//  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
//  sfree(tmp);
//
//  // update ob_dialing info
//  uuid = json_string_value(json_object_get(j_msg, "DestUniqueid"));
//  update_ob_dialing_status(uuid, E_DIALING_DIAL_END);

  return;
}

/**
 * AMI event handler.
 * Event: OriginateResponse
 * @param j_msg
 */
static void ami_event_originateresponse(json_t* j_msg)
{
  char* tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_originateresponse.");

  // check event
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
  sfree(tmp);

  return;
}

/**
 * AMI event handler.
 * Event: Hangup
 * @param j_msg
 */
static void ami_event_hangup(json_t* j_msg)
{
  int hangup;
  int ret;
  const char* unique_id;
  const char* tmp_const;
  const char* hangup_detail;
  char* timestamp;
  json_t* j_tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_hangup.");

  unique_id = json_string_value(json_object_get(j_msg, "Uniqueid"));
  if(unique_id == NULL) {
    slog(LOG_ERR, "Could not get unique id info.");
    return;
  }

  // update channel info
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:i, s:s, "
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "hangup_cause", json_string_value(json_object_get(j_msg, "Cause"))? atoi(json_string_value(json_object_get(j_msg, "Cause"))): 0,
      "hangup_cause_desc", json_string_value(json_object_get(j_msg, "Cause-txt"))? : "Unknown",

      "tm_update",  timestamp
      );
  sfree(timestamp);

  // update info
  ret = update_core_channel_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update channel info.");
    return;
  }

  // delete channel info.
  ret = delete_core_channel_info(unique_id);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete channel info. unique_id[%s]", unique_id);
    return;
  }

  ///// for ob_dialing

  // get values
  hangup_detail = json_string_value(json_object_get(j_msg, "Cause-txt"));
  tmp_const = json_string_value(json_object_get(j_msg, "Cause"))? : 0;
  hangup = atoi(tmp_const);

  // update ob_dialing channel
  update_ob_dialing_hangup(unique_id, hangup, hangup_detail);

  return;
}

/**
 * AMI event handler.
 * Event: NewChannel
 * @param j_msg
 */
static void ami_event_newchannel(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_newchannel.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:{}"
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "variables",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create info
  ret = create_core_channel_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to channel.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: RegistryEntry
 * @param j_msg
 */
static void ami_event_registryentry(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* account;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_registryentry.");

  // create db data
  asprintf(&account, "%s@%s:%d",
      json_string_value(json_object_get(j_msg, "Username"))? : "",
      json_string_value(json_object_get(j_msg, "Host"))? : "",
      json_string_value(json_object_get(j_msg, "Port"))? atoi(json_string_value(json_object_get(j_msg, "Port"))) : 0
      );
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:i, s:s,"
      "s:s, s:i, s:i, s:s, s:i"
      "s:s"
      "}",
      "account",  account,
      "host",     json_string_value(json_object_get(j_msg, "Host"))? : "",
      "port",     json_string_value(json_object_get(j_msg, "Port"))? atoi(json_string_value(json_object_get(j_msg, "Port"))): 0,
      "username", json_string_value(json_object_get(j_msg, "Username"))? : "",

      "domain",             json_string_value(json_object_get(j_msg, "Domain")),
      "domain_port",        json_string_value(json_object_get(j_msg, "DomainPort"))? atoi(json_string_value(json_object_get(j_msg, "DomainPort"))): 0,
      "refresh",            json_string_value(json_object_get(j_msg, "Refresh"))? atoi(json_string_value(json_object_get(j_msg, "Refresh"))): 0,
      "state",              json_string_value(json_object_get(j_msg, "State")),
      "registration_time",  json_string_value(json_object_get(j_msg, "RegistrationTime"))? atoi(json_string_value(json_object_get(j_msg, "RegistrationTime"))): 0,

      "tm_update",  timestamp
      );
  sfree(account);
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create info
  ret = create_sip_registry_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to registry.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Rename
 * @param j_msg
 */
static void ami_event_rename(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  const char* unique_id;
  const char* chan_name;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_rename.");

  // get update info
  unique_id = json_string_value(json_object_get(j_msg, "Uniqueid"));
  chan_name = json_string_value(json_object_get(j_msg, "Newname"));
  if((unique_id == NULL) || (chan_name == NULL)) {
    slog(LOG_NOTICE, "Could not get correct update info. unique_id[%s], chan_name[%s]",
        unique_id? : "",
        chan_name? : ""
        );
    return;
  }

  // get channel info
  j_tmp = get_core_channel_info(unique_id);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get correct channel info.");
    return;
  }

  // update new channel info
  timestamp = get_utc_timestamp();
  json_object_set_new(j_tmp, "channel", json_string(chan_name));
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  sfree(timestamp);

  // update channel info.
  ret = update_core_channel_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update channel info.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Agents
 * @param j_msg
 */
static void ami_event_agents(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_agents.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:i, "
      "s:s, s:i, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "id",               json_string_value(json_object_get(j_msg, "Agent"))? : "",
      "name",             json_string_value(json_object_get(j_msg, "Name"))? : "",
      "status",           json_string_value(json_object_get(j_msg, "Status"))? : "",
      "logged_in_time",   json_string_value(json_object_get(j_msg, "LoggedInTime"))? atoi(json_string_value(json_object_get(j_msg, "LoggedInTime"))) : 0,

      "channel_name",       json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))) : 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",  json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name", json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",

      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",     json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code", json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",  json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",    json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority", json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create info
  ret = create_agent_agent_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to agent.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AgentLogin
 * @param j_msg
 */
static void ami_event_agentlogin(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_agentlogin.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "id",               json_string_value(json_object_get(j_msg, "Agent"))? : "",
      "status",           "AGENT_IDLE",

      "channel_name",       json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))) : 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",  json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name", json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",

      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",     json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code", json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",  json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",    json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority", json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // update info
  ret = update_agent_agent_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update agent info. agent[%s]", json_string_value(json_object_get(j_msg, "Agent")));
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AgentLogoff
 * @param j_msg
 */
static void ami_event_agentlogoff(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_agentlogoff.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "id",               json_string_value(json_object_get(j_msg, "Agent"))? : "",
      "status",           "AGENT_LOGGEDOFF",

      "channel_name",       json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))) : 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",  json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name", json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",

      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",     json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code", json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",  json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",    json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority", json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = update_agent_agent_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update agent info. agent[%s]", json_string_value(json_object_get(j_msg, "Agent")));
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ParkingLot
 * @param j_msg
 */
static void ami_event_parkinglot(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  char* tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_parkinglot.");

  // check event
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Event message. msg[%s]", tmp);
  sfree(tmp);

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, "
      "s:s, s:s, "
      "s:i, "
      "s:s"
      "}",

      "name", json_string_value(json_object_get(j_msg, "Name"))? : "",

      "start_space",  json_string_value(json_object_get(j_msg, "StartSpace"))? : "",
      "stop_space",   json_string_value(json_object_get(j_msg, "StopSpace"))? : "",

      "timeout",  json_string_value(json_object_get(j_msg, "Timeout"))? atoi(json_string_value(json_object_get(j_msg, "Timeout"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create parking lot
  ret = create_park_parkinglot_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create parking_lot.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ParkedCall
 * @param j_msg
 */
static void ami_event_parkedcall(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_parkedcall.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:i, s:i, "
      "s:s"
      "}",

      // parked channel info
      "parkee_channel",             json_string_value(json_object_get(j_msg, "ParkeeChannel"))? : "",
      "parkee_channel_state",       json_string_value(json_object_get(j_msg, "ParkeeChannelState"))? : "",
      "parkee_channel_state_desc",  json_string_value(json_object_get(j_msg, "ParkeeChannelStateDesc"))? : "",

      // parked channel caller info
      "parkee_caller_id_num",   json_string_value(json_object_get(j_msg, "ParkeeCallerIDNum"))? : "",
      "parkee_caller_id_name",  json_string_value(json_object_get(j_msg, "ParkeeCallerIDName"))? : "",

      // parked channel connected line info
      "parkee_connected_line_num",  json_string_value(json_object_get(j_msg, "ParkeeConnectedLineNum"))? : "",
      "parkee_connected_line_name", json_string_value(json_object_get(j_msg, "ParkeeConnectedLineName"))? : "",

      // parked channel account info
      "parkee_account_code",  json_string_value(json_object_get(j_msg, "ParkeeAccountCode"))? : "",

      // parked channel dialplan info
      "parkee_context",   json_string_value(json_object_get(j_msg, "ParkeeContext"))? : "",
      "parkee_exten",     json_string_value(json_object_get(j_msg, "ParkeeExten"))? : "",
      "parkee_priority",  json_string_value(json_object_get(j_msg, "ParkeePriority"))? : "",

      // parked channel id info
      "parkee_unique_id",   json_string_value(json_object_get(j_msg, "ParkeeUniqueid"))? : "",
      "parkee_linked_id",   json_string_value(json_object_get(j_msg, "ParkeeLinkedid"))? : "",

      // parked channel parker info
      "parker_dial_string", json_string_value(json_object_get(j_msg, "ParkerDialString"))? : "",

      // parking lot info
      "parking_lot",      json_string_value(json_object_get(j_msg, "Parkinglot"))? : "",
      "parking_space",    json_string_value(json_object_get(j_msg, "ParkingSpace"))? : "",
      "parking_timeout",  json_string_value(json_object_get(j_msg, "ParkingTimeout"))? atoi(json_string_value(json_object_get(j_msg, "ParkingTimeout"))): 0,
      "parking_duration", json_string_value(json_object_get(j_msg, "ParkingDuration"))? atoi(json_string_value(json_object_get(j_msg, "ParkingDuration"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = create_park_parkedcall_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parked_call.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ParkedCallSwap
 * @param j_msg
 */
static void ami_event_parkedcallswap(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_parkedcallswap.");

  // insert new parked call
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:i, s:i, "
      "s:s"
      "}",

      // parked channel info
      "parkee_channel",             json_string_value(json_object_get(j_msg, "ParkeeChannel"))? : "",
      "parkee_channel_state",       json_string_value(json_object_get(j_msg, "ParkeeChannelState"))? : "",
      "parkee_channel_state_desc",  json_string_value(json_object_get(j_msg, "ParkeeChannelStateDesc"))? : "",

      // parked channel caller info
      "parkee_caller_id_num",   json_string_value(json_object_get(j_msg, "ParkeeCallerIDNum"))? : "",
      "parkee_caller_id_name",  json_string_value(json_object_get(j_msg, "ParkeeCallerIDName"))? : "",

      // parked channel connected line info
      "parkee_connected_line_num",  json_string_value(json_object_get(j_msg, "ParkeeConnectedLineNum"))? : "",
      "parkee_connected_line_name", json_string_value(json_object_get(j_msg, "ParkeeConnectedLineName"))? : "",

      // parked channel account info
      "parkee_account_code",  json_string_value(json_object_get(j_msg, "ParkeeAccountCode"))? : "",

      // parked channel dialplan info
      "parkee_context",   json_string_value(json_object_get(j_msg, "ParkeeContext"))? : "",
      "parkee_exten",     json_string_value(json_object_get(j_msg, "ParkeeExten"))? : "",
      "parkee_priority",  json_string_value(json_object_get(j_msg, "ParkeePriority"))? : "",

      // parked channel id info
      "parkee_unique_id",   json_string_value(json_object_get(j_msg, "ParkeeUniqueid"))? : "",
      "parkee_linked_id",   json_string_value(json_object_get(j_msg, "ParkeeLinkedid"))? : "",

      // parked channel parker info
      "parker_dial_string", json_string_value(json_object_get(j_msg, "ParkerDialString"))? : "",

      // parking lot info
      "parking_lot",      json_string_value(json_object_get(j_msg, "Parkinglot"))? : "",
      "parking_space",    json_string_value(json_object_get(j_msg, "ParkingSpace"))? : "",
      "parking_timeout",  json_string_value(json_object_get(j_msg, "ParkingTimeout"))? atoi(json_string_value(json_object_get(j_msg, "ParkingTimeout"))): 0,
      "parking_duration", json_string_value(json_object_get(j_msg, "ParkingDuration"))? atoi(json_string_value(json_object_get(j_msg, "ParkingDuration"))): 0,

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = update_park_parkedcall_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to parked_call.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ParkedCallTimeOut
 * @param j_msg
 */
static void ami_event_parkedcalltimeout(json_t* j_msg)
{
  int ret;
  const char* tmp_const;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_parkedcalltimeout.");

  tmp_const = json_string_value(json_object_get(j_msg, "ParkeeUniqueid"));
  ret = delete_park_parkedcall_info(tmp_const);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete parked_call.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: UnParkedCall
 * @param j_msg
 */
static void ami_event_unparkedcall(json_t* j_msg)
{
  int ret;
  const char* tmp_const;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_unparkedcall.");

  tmp_const = json_string_value(json_object_get(j_msg, "ParkeeUniqueid"));
  ret = delete_park_parkedcall_info(tmp_const);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete parked_call.");
    return;
  }
  return;
}

/**
 * AMI event handler.
 * Event: ParkedCallGiveUp
 * @param j_msg
 */
static void ami_event_parkedcallgiveup(json_t* j_msg)
{
  int ret;
  const char* tmp_const;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired ami_event_parkedcallgiveup.");

  tmp_const = json_string_value(json_object_get(j_msg, "ParkeeUniqueid"));
  ret = delete_park_parkedcall_info(tmp_const);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete parked_call.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: DeviceStateChange
 * @param j_msg
 */
static void ami_event_devicestatechange(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_devicestatechange.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s"
      "}",

      "device", json_string_value(json_object_get(j_msg, "Device"))? : "",
      "state",  json_string_value(json_object_get(j_msg, "State"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  ret = db_ctx_insert_or_replace(g_db_ast, "device_state", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to device_state.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: VarSet
 * @param j_msg
 */
static void ami_event_varset(json_t* j_msg)
{
  json_t* j_chan;
  int ret;
  char* timestamp;
  const char* unique_id;
  const char* key;
  const char* val;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_varset.");

  // get unique_id info
  unique_id = json_string_value(json_object_get(j_msg, "Uniqueid"));
  if(unique_id == NULL) {
    slog(LOG_ERR, "Could not get unique_id info.");
    return;
  }

  // get values
  key = json_string_value(json_array_get(json_object_get(j_msg, "Variable"), 0));
  val = json_string_value(json_object_get(j_msg, "Value"));
  slog(LOG_DEBUG, "Check value. key[%s], val[%s]", key, val);

  // get channel info
  j_chan = get_core_channel_info(unique_id);
  if(j_chan == NULL) {
    slog(LOG_ERR, "Could not get channel info.");
    return;
  }

  // update variables
  json_object_set_new(json_object_get(j_chan, "variables"), key, json_string(val));

  // update other values
  json_object_set(j_chan, "channel", json_object_get(j_msg, "Channel"));
  json_object_set_new(j_chan, "channel_state", json_integer(atoi(json_string_value(json_object_get(j_msg, "ChannelState")))));
  json_object_set(j_chan, "channel_state_desc", json_object_get(j_msg, "ChannelStateDesc"));

  json_object_set(j_chan, "caller_id_num", json_object_get(j_msg, "CallerIDNum"));
  json_object_set(j_chan, "caller_id_name", json_object_get(j_msg, "CallerIDName"));

  json_object_set(j_chan, "connected_line_num", json_object_get(j_msg, "ConnectedLineNum"));
  json_object_set(j_chan, "connected_line_name", json_object_get(j_msg, "ConnectedLineName"));

  json_object_set(j_chan, "language", json_object_get(j_msg, "Language"));
  json_object_set(j_chan, "account_code", json_object_get(j_msg, "AccountCode"));

  json_object_set(j_chan, "context", json_object_get(j_msg, "Context"));
  json_object_set(j_chan, "exten", json_object_get(j_msg, "Exten"));
  json_object_set(j_chan, "priority", json_object_get(j_msg, "Priority"));

  timestamp = get_utc_timestamp();
  json_object_set_new(j_chan, "tm_update", json_string(timestamp));
  sfree(timestamp);

  // update info
  ret = update_core_channel_info(j_chan);
  json_decref(j_chan);
  if(ret == false) {
    slog(LOG_ERR, "Could not update to channel.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: EndpointList
 * @param j_msg
 */
static void ami_event_endpointlist(json_t* j_msg)
{
  json_t* j_data;
  json_t* j_tmp;
  char* timestamp;
  const char* name;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_endpointlist.");

  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:s"
      "}",

      "object_name",  json_string_value(json_object_get(j_msg, "ObjectName"))? : "",
      "object_type",  json_string_value(json_object_get(j_msg, "ObjectType"))? : "",

      "transport",      json_string_value(json_object_get(j_msg, "Transport"))? : "",
      "aors",           json_string_value(json_object_get(j_msg, "Aor"))? : "",
      "auth",           json_string_value(json_object_get(j_msg, "Auths"))? : "",
      "outbound_auth",  json_string_value(json_object_get(j_msg, "OutboundAuths"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);

  // create info
  ret = create_pjsip_endpoint_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create endpoint info.");
  }

  // send request for detail info
  name = json_string_value(json_object_get(j_msg, "ObjectName"));
  j_tmp = json_pack("{s:s, s:s}",
      "Action", "PJSIPShowEndpoint",
      "Endpoint", name
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "PJSIPShowEndpoint");
    return;
  }

  return;
}


/**
 * AMI event handler.
 * Event: EndpointDetail
 * @param j_msg
 */
static void ami_event_endpointdetail(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_endpointdetail.");

  timestamp = get_utc_timestamp();

  // create
  j_tmp = json_pack("{"
      "s:s, s:s, s:s, s:s, "          // basic info
      "s:s, s:s, s:s, "               // context
      "s:s, s:s, s:s, "               // mailbox
      "s:s, s:s, s:s, s:s, s:s, "     // allow and disallow
      "s:s, "

      "s:s, "
      "s:s, s:s, s:s, s:i, s:i, s:i, s:s, "    // rtp
      "s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, s:s, "

      "s:s, "
      "s:s, s:s, s:i, "   // timers
      "s:s, s:s, "        // outbound
      "s:s, "
      "s:s, "

      "s:s, s:s, s:s, s:s, "    // direct media
      "s:s, "
      "s:s, s:s, s:s, "         // caller id
      "s:s, s:s, "              // trust id
      "s:s, "

      "s:s, s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:s, s:s, "    // media
      "s:s, s:s, "
      "s:s, "

      "s:s, s:s, "                        // progress
      "s:s, s:s, s:s, s:s, "              // group
      "s:s, s:i, "                        // device
      "s:s, s:i, s:s, s:s, s:i, s:s, s:s, "    // t38 fax
      "s:s, s:s, "

      "s:s, s:s, "        // record
      "s:s, s:s, "
      "s:s, s:s, "        // sdp
      "s:i, s:i, s:i, s:i, "  // tos and cos
      "s:s, s:s, s:s, "

      "s:s, s:s, "                                    // mwi
      "s:s, s:i, s:s, s:s, s:s, s:s, s:s, s:s, s:s, " // dtls
      "s:s, "
      "s:s, "
      "s:s, s:s, "

      "s:s, "
      "s:i, s:i, "                      // max streams
      "s:s, s:s, "                      // acl
      "s:s, s:s, s:s, s:s, "  // etc

      "s:s "  // timestamp
      "}",

      // basic info
      "object_type",  json_string_value(json_object_get(j_msg, "ObjectType"))? : "",
      "object_name",  json_string_value(json_object_get(j_msg, "ObjectName"))? : "",
      "auth",         json_string_value(json_object_get(j_msg, "Auth"))? : "",
      "aors",         json_string_value(json_object_get(j_msg, "Aors"))? : "",

      // context
      "context",            json_string_value(json_object_get(j_msg, "Context"))? : "",
      "message_context",    json_string_value(json_object_get(j_msg, "MessageContext"))? : "",
      "subscribe_context",  json_string_value(json_object_get(j_msg, "SubscribeContext"))? : "",

      // mail box
      "mailboxes",            json_string_value(json_object_get(j_msg, "Mailboxes"))? : "",
      "voicemail_extension",  json_string_value(json_object_get(j_msg, "VoicemailExtension"))? : "",
      "incoming_mwi_mailbox", json_string_value(json_object_get(j_msg, "IncomingMwiMailbox"))? : "",

      // allow and disallow
      "disallow",         json_string_value(json_object_get(j_msg, "Disallow"))? : "",
      "allow",            json_string_value(json_object_get(j_msg, "Allow"))? : "",
      "allow_subscribe",  json_string_value(json_object_get(j_msg, "AllowSubscribe"))? : "",
      "allow_overlap",    json_string_value(json_object_get(j_msg, "AllowOverlap"))? : "",
      "allow_transfer",   json_string_value(json_object_get(j_msg, "AllowTransfer"))? : "",

      "webrtc",           json_string_value(json_object_get(j_msg, "Webrtc"))? : "",

      "dtmf_mode",        json_string_value(json_object_get(j_msg, "DtmfMode"))? : "",

      // rtp
      "rtp_engine",           json_string_value(json_object_get(j_msg, "RtpEngine"))? : "",
      "rtp_ipv6",             json_string_value(json_object_get(j_msg, "RtpIpv6"))? : "",
      "rtp_symmetric",        json_string_value(json_object_get(j_msg, "RtpSymmetric"))? : "",
      "rtp_keepalive",        json_string_value(json_object_get(j_msg, "RtpKeepalive"))? atoi(json_string_value(json_object_get(j_msg, "RtpKeepalive"))) : 0,
      "rtp_timeout",          json_string_value(json_object_get(j_msg, "RtpTimeout"))? atoi(json_string_value(json_object_get(j_msg, "RtpTimeout"))) : 0,
      "rtp_timeout_hold",     json_string_value(json_object_get(j_msg, "RtpTimeoutHold"))? atoi(json_string_value(json_object_get(j_msg, "RtpTimeoutHold"))) : 0,
      "asymmetric_rtp_codec", json_string_value(json_object_get(j_msg, "AsymmetricRtpCodec")),

      "rtcp_mux", json_string_value(json_object_get(j_msg, "RtcpMux"))? : "",

      "ice_support",  json_string_value(json_object_get(j_msg, "IceSupport"))? : "",
      "use_ptime",    json_string_value(json_object_get(j_msg, "UsePtime"))? : "",

      "force_rport",      json_string_value(json_object_get(j_msg, "ForceRport"))? : "",
      "rewrite_contact",  json_string_value(json_object_get(j_msg, "RewriteContact"))? : "",
      "transport",        json_string_value(json_object_get(j_msg, "Transport"))? : "",
      "moh_suggest",      json_string_value(json_object_get(j_msg, "MohSuggest"))? : "",

      "rel_100",  json_string_value(json_object_get(j_msg, "100rel"))? : "",

      // timers
      "timers",               json_string_value(json_object_get(j_msg, "Timers"))? : "",
      "timers_min_se",        json_string_value(json_object_get(j_msg, "TimersMinSe"))? : "",
      "timers_sess_expires",  json_string_value(json_object_get(j_msg, "TimersSessExpires"))? atoi(json_string_value(json_object_get(j_msg, "TimersSessExpires"))) : 0,

      // outbound
      "outbound_proxy",   json_string_value(json_object_get(j_msg, "OutboundProxy"))? : "",
      "outbound_auth",    json_string_value(json_object_get(j_msg, "OutboundAuth"))? : "",

      "identify_by",  json_string_value(json_object_get(j_msg, "IdentifyBy"))? : "",

      "redirect_method",  json_string_value(json_object_get(j_msg, "RedirectMethod"))? : "",

      // direct media
      "direct_media",                   json_string_value(json_object_get(j_msg, "DirectMedia"))? : "",
      "direct_media_method",            json_string_value(json_object_get(j_msg, "DirectMediaMethod"))? : "",
      "direct_media_glare_mitigation",  json_string_value(json_object_get(j_msg, "DirectMediaGlareMitigation"))? : "",
      "disable_direct_media_on_nat",    json_string_value(json_object_get(j_msg, "DisableDirectMediaOnNat"))? : "",

      "connected_line_method",  json_string_value(json_object_get(j_msg, "ConnectedLineMethod"))? : "",

      // caller id
      "caller_id",          json_string_value(json_object_get(j_msg, "Callerid"))? : "",
      "caller_id_privacy",  json_string_value(json_object_get(j_msg, "CalleridPrivacy"))? : "",
      "caller_id_tag",      json_string_value(json_object_get(j_msg, "CalleridTag"))? : "",

      // trust id
      "trust_id_inbound",   json_string_value(json_object_get(j_msg, "TrustIdInbound"))? : "",
      "trust_id_outbound",  json_string_value(json_object_get(j_msg, "TrustIdOutbound"))? : "",

      "send_pai", json_string_value(json_object_get(j_msg, "SendPai"))? : "",

      "rpid_immediate",   json_string_value(json_object_get(j_msg, "RpidImmediate"))? : "",
      "send_rpid",        json_string_value(json_object_get(j_msg, "SendRpid"))? : "",
      "send_diversion",   json_string_value(json_object_get(j_msg, "SendDiversion"))? : "",

      "aggregate_mwi",  json_string_value(json_object_get(j_msg, "AggregateMwi"))? : "",

      // media
      "media_address",                json_string_value(json_object_get(j_msg, "MediaAddress"))? : "",
      "media_encryption",             json_string_value(json_object_get(j_msg, "MediaEncryption"))? : "",
      "media_encryption_optimistic",  json_string_value(json_object_get(j_msg, "MediaEncryptionOptimistic"))? : "",
      "media_use_received_transport", json_string_value(json_object_get(j_msg, "MediaUseReceivedTransport"))? : "",

      "use_avpf",   json_string_value(json_object_get(j_msg, "UseAvpf"))? : "",
      "force_avp",  json_string_value(json_object_get(j_msg, "ForceAvp"))? : "",

      "one_touch_recording",  json_string_value(json_object_get(j_msg, "OneTouchRecording"))? : "",

      // progress
      "inband_progress",        json_string_value(json_object_get(j_msg, "InbandProgress"))? : "",
      "refer_blind_progress",   json_string_value(json_object_get(j_msg, "ReferBlindProgress"))? : "",

      // group
      "call_group",           json_string_value(json_object_get(j_msg, "CallGroup"))? : "",
      "pickup_group",         json_string_value(json_object_get(j_msg, "PickupGroup"))? : "",
      "named_call_group",     json_string_value(json_object_get(j_msg, "NamedCallGroup"))? : "",
      "named_pickup_group",   json_string_value(json_object_get(j_msg, "NamedPickupGroup"))? : "",

      // device
      "device_state",           json_string_value(json_object_get(j_msg, "DeviceState"))? : "",
      "device_state_busy_at",   json_string_value(json_object_get(j_msg, "DeviceStateBusyAt"))? atoi(json_string_value(json_object_get(j_msg, "DeviceStateBusyAt"))) : 0,

      // t38 fax
      "fax_detect",             json_string_value(json_object_get(j_msg, "FaxDetect"))? : "",
      "fax_detect_time",        json_string_value(json_object_get(j_msg, "FaxDetectTimeout"))? atoi(json_string_value(json_object_get(j_msg, "FaxDetectTimeout"))) : 0,
      "t38_udptl",              json_string_value(json_object_get(j_msg, "T38Udptl"))? : "",
      "t38_udptl_ec",           json_string_value(json_object_get(j_msg, "T38UdptlEc"))? : "",
      "t38_udptl_maxdatagram",  json_string_value(json_object_get(j_msg, "T38UdptlMaxdatagram"))? atoi(json_string_value(json_object_get(j_msg, "T38UdptlMaxdatagram"))) : 0,
      "t38_udptl_nat",          json_string_value(json_object_get(j_msg, "T38UdptlNat"))? : "",
      "t38_udptl_ipv6",         json_string_value(json_object_get(j_msg, "T38UdptlIpv6"))? : "",

      "tone_zone",  json_string_value(json_object_get(j_msg, "ToneZone"))? : "",
      "language",   json_string_value(json_object_get(j_msg, "Language"))? : "",

      // record
      "record_on_feature",    json_string_value(json_object_get(j_msg, "RecordOnFeature"))? : "",
      "record_off_feature",   json_string_value(json_object_get(j_msg, "RecordOffFeature"))? : "",

      "user_eq_phone",    json_string_value(json_object_get(j_msg, "UserEqPhone"))? : "",
      "moh_passthrough",  json_string_value(json_object_get(j_msg, "MohPassthrough"))? : "",

      // sdp
      "sdp_owner",    json_string_value(json_object_get(j_msg, "SdpOwner"))? : "",
      "sdp_session",  json_string_value(json_object_get(j_msg, "SdpSession"))? : "",

      // tos and cos
      "tos_audio",  json_string_value(json_object_get(j_msg, "TosAudio"))? atoi(json_string_value(json_object_get(j_msg, "TosAudio"))) : 0,
      "tos_video",  json_string_value(json_object_get(j_msg, "TosVideo"))? atoi(json_string_value(json_object_get(j_msg, "TosVideo"))) : 0,
      "cos_audio",  json_string_value(json_object_get(j_msg, "CosAudio"))? atoi(json_string_value(json_object_get(j_msg, "CosAudio"))) : 0,
      "cos_video",  json_string_value(json_object_get(j_msg, "CosVideo"))? atoi(json_string_value(json_object_get(j_msg, "CosVideo"))) : 0,

      "sub_min_expiry",   json_string_value(json_object_get(j_msg, "SubMinExpiry"))? : "",
      "from_user",        json_string_value(json_object_get(j_msg, "FromUser"))? : "",
      "from_domain",      json_string_value(json_object_get(j_msg, "FromDomain"))? : "",

      // mwi
      "mwi_from_user",                      json_string_value(json_object_get(j_msg, "MwiFromUser"))? : "",
      "mwi_subscribe_replaces_unsolicited", json_string_value(json_object_get(j_msg, "MwiSubscribeReplacesUnsolicited"))? : "",

      // dtls
      "dtls_verify",        json_string_value(json_object_get(j_msg, "DtlsVerify"))? : "",
      "dtls_rekey",         json_string_value(json_object_get(j_msg, "DtlsRekey"))? atoi(json_string_value(json_object_get(j_msg, "DtlsRekey"))) : 0,
      "dtls_cert_file",     json_string_value(json_object_get(j_msg, "DtlsCertFile"))? : "",
      "dtls_private_key",   json_string_value(json_object_get(j_msg, "DtlsPrivateKey"))? : "",
      "dtls_cipher",        json_string_value(json_object_get(j_msg, "DtlsCipher"))? : "",
      "dtls_ca_file",       json_string_value(json_object_get(j_msg, "DtlsCaFile"))? : "",
      "dtls_ca_path",       json_string_value(json_object_get(j_msg, "DtlsCaPath"))? : "",
      "dtls_setup",         json_string_value(json_object_get(j_msg, "DtlsSetup"))? : "",
      "dtls_fingerprint",   json_string_value(json_object_get(j_msg, "DtlsFingerprint"))? : "",

      "srtp_tag32", json_string_value(json_object_get(j_msg, "SrtpTag32"))? : "",

      "set_var",  json_string_value(json_object_get(j_msg, "SetVar"))? : "",

      "account_code",           json_string_value(json_object_get(j_msg, "Accountcode"))? : "",
      "preferred_codec_only",   json_string_value(json_object_get(j_msg, "PreferredCodecOnly"))? : "",

      "active_channels",  json_string_value(json_object_get(j_msg, "ActiveChannels"))? : "",

      // max streams
      "max_audio_streams",            json_string_value(json_object_get(j_msg, "MaxAudioStreams"))? atoi(json_string_value(json_object_get(j_msg, "MaxAudioStreams"))) : 0,
      "max_video_streams",            json_string_value(json_object_get(j_msg, "MaxVideoStreams"))? atoi(json_string_value(json_object_get(j_msg, "MaxVideoStreams"))) : 0,

      // acl
      "acl",                          json_string_value(json_object_get(j_msg, "Acl"))? : "",
      "contact_acl",                  json_string_value(json_object_get(j_msg, "ContactAcl"))? : "",

      // etc
      "g_726_non_standard",           json_string_value(json_object_get(j_msg, "G726NonStandard"))? : "",
      "notify_early_inuse_ringing",   json_string_value(json_object_get(j_msg, "NotifyEarlyInuseRinging"))? : "",
      "bind_rtp_to_media_address",    json_string_value(json_object_get(j_msg, "BindRtpToMediaAddress"))? : "",
      "bundle",                       json_string_value(json_object_get(j_msg, "Bundle"))? : "",

      "tm_update",    timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // update info
  ret = update_pjsip_endpoint_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to pjsip_endpoint.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ContactStatus
 * @param j_msg
 */
static void ami_event_contactstatus(json_t* j_msg)
{
  json_t* j_data;
  char* timestamp;
  const char* uri;
  const char* tmp_const;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_contactstatus.");

  // get uri
  uri = json_string_value(json_object_get(j_msg, "URI"));
  if(uri == NULL) {
    slog(LOG_ERR, "Could not get uri info.");
    return;
  }

  // create update info
  j_data = json_pack("{s:s}", "uri", uri);

  // status
  tmp_const = json_string_value(json_object_get(j_msg, "ContactStatus"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "status", json_string(tmp_const));
  }

  // aor
  tmp_const = json_string_value(json_object_get(j_msg, "AOR"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "aor", json_string(tmp_const));
  }

  // endpoint_name
  tmp_const = json_string_value(json_object_get(j_msg, "EndpointName"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "endpoint_name", json_string(tmp_const));
  }

  // round_trip_usec
  tmp_const = json_string_value(json_object_get(j_msg, "RoundtripUsec"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "round_trip_usec", json_integer(atoi(tmp_const)));
  }

  // user_agent
  tmp_const = json_string_value(json_object_get(j_msg, "UserAgent"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "user_agent", json_string(tmp_const));
  }

  // reg_expire
  tmp_const = json_string_value(json_object_get(j_msg, "RegExpire"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "reg_expire", json_string(tmp_const));
  }

  // via_address
  tmp_const = json_string_value(json_object_get(j_msg, "ViaAddress"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "via_address", json_string(tmp_const));
  }

  // call_id
  tmp_const = json_string_value(json_object_get(j_msg, "CallID"));
  if(tmp_const != NULL) {
    json_object_set_new(j_data, "call_id", json_string(tmp_const));
  }

  timestamp = get_utc_timestamp();
  json_object_set_new(j_data, "tm_update", json_string(timestamp));
  sfree(timestamp);

  // update info
  ret = update_pjsip_contact_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to pjsip_contact.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: ContactStatusDetail
 * @param j_msg
 */
static void ami_event_contactstatusdetail(json_t* j_msg)
{
  json_t* j_data;
  char* timestamp;
  const char* tmp_const;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_contactstatusdetail.");

  tmp_const = json_string_value(json_object_get(j_msg, "URI"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get URI info.");
    return;
  }

  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s, s:s, s:s, "
      "s:s, "
      "s:s, s:s, s:i, s:s, s:s, "
      "s:i, s:s, s:s, "
      "s:i, s:i,"
      "s:s "
      "}",

      "uri",            json_string_value(json_object_get(j_msg, "URI"))? : "",
      "id",             json_string_value(json_object_get(j_msg, "ID"))? : "",
      "aor",            json_string_value(json_object_get(j_msg, "AOR"))? : "",
      "endpoint_name",  json_string_value(json_object_get(j_msg, "EndpointName"))? : "",

      "status",   json_string_value(json_object_get(j_msg, "Status"))? : "",

      "round_trip_usec",  json_string_value(json_object_get(j_msg, "RoundtripUsec"))? : "",
      "user_agent",       json_string_value(json_object_get(j_msg, "UserAgent"))? : "",
      "reg_expire",       json_string_value(json_object_get(j_msg, "RegExpire"))? atoi(json_string_value(json_object_get(j_msg, "RegExpire"))) : 0,
      "via_address",      json_string_value(json_object_get(j_msg, "ViaAddress"))? : "",
      "call_id",          json_string_value(json_object_get(j_msg, "CallID"))? : "",

      "authentication_qualify", json_string_value(json_object_get(j_msg, "AuthenticateQualify"))? atoi(json_string_value(json_object_get(j_msg, "AuthenticateQualify"))) : 0,
      "outbound_proxy",         json_string_value(json_object_get(j_msg, "OutboundProxy"))? : "",
      "path",                   json_string_value(json_object_get(j_msg, "Path"))? : "",

      "qualify_frequency",  json_string_value(json_object_get(j_msg, "QualifyFrequency"))? atoi(json_string_value(json_object_get(j_msg, "QualifyFrequency"))) : 0,
      "qualify_timout",     json_string_value(json_object_get(j_msg, "QualifyTimeout"))? atoi(json_string_value(json_object_get(j_msg, "QualifyTimeout"))) : 0,

      "tm_update", timestamp
      );
  sfree(timestamp);

  // create info
  ret = create_pjsip_contact_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to pjsip_contact.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AorDetail
 * @param j_msg
 */
static void ami_event_aordetail(json_t* j_msg)
{
  json_t* j_data;
  char* timestamp;
  const char* tmp_const;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_aordetail.");

  tmp_const = json_string_value(json_object_get(j_msg, "ObjectName"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get ObjectName info.");
    return;
  }

  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s, s:s, "
      "s:i, s:i, s:i, "
      "s:i, s:i, "
      "s:s, s:i, s:i, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, "
      "s:s "
      "}",

      // basic info
      "object_type",    json_string_value(json_object_get(j_msg, "ObjectType"))? : "",
      "object_name",    json_string_value(json_object_get(j_msg, "ObjectName"))? : "",
      "endpoint_name",  json_string_value(json_object_get(j_msg, "EndpointName"))? : "",

      // expiration
      "minimum_expiration", json_string_value(json_object_get(j_msg, "MinimumExpiration"))? atoi(json_string_value(json_object_get(j_msg, "MinimumExpiration"))): 0,
      "default_expiration", json_string_value(json_object_get(j_msg, "DefaultExpiration"))? atoi(json_string_value(json_object_get(j_msg, "DefaultExpiration"))): 0,
      "maximum_expiration", json_string_value(json_object_get(j_msg, "MaximumExpiration"))? atoi(json_string_value(json_object_get(j_msg, "MaximumExpiration"))): 0,

      // qualify
      "qualify_timeout",    json_string_value(json_object_get(j_msg, "QualifyTimeout"))? atoi(json_string_value(json_object_get(j_msg, "QualifyTimeout"))): 0,
      "qualify_frequency",  json_string_value(json_object_get(j_msg, "QualifyFrequency"))? atoi(json_string_value(json_object_get(j_msg, "QualifyFrequency"))): 0,

      // contact
      "contacts",             json_string_value(json_object_get(j_msg, "Contacts"))? : "",
      "max_contacts",         json_string_value(json_object_get(j_msg, "MaxContacts"))? atoi(json_string_value(json_object_get(j_msg, "MaxContacts"))): 0,
      "total_contacts",       json_string_value(json_object_get(j_msg, "TotalContacts"))? atoi(json_string_value(json_object_get(j_msg, "TotalContacts"))): 0,
      "contacts_registered",  json_string_value(json_object_get(j_msg, "ContactsRegistered"))? atoi(json_string_value(json_object_get(j_msg, "ContactsRegistered"))): 0,
      "remove_existing",      json_string_value(json_object_get(j_msg, "RemoveExisting"))? : "",

      // etc
      "mailboxes",            json_string_value(json_object_get(j_msg, "Mailboxes"))? : "",
      "support_path",         json_string_value(json_object_get(j_msg, "SupportPath"))? : "",
      "voicemail_extension",  json_string_value(json_object_get(j_msg, "VoicemailExtension"))? : "",
      "authenticate_qualify", json_string_value(json_object_get(j_msg, "AuthenticateQualify"))? : "",
      "outbound_proxy",       json_string_value(json_object_get(j_msg, "OutboundProxy"))? : "",

      // timestamp. UTC."
      "tm_update",    timestamp
      );
  sfree(timestamp);

  ret = create_pjsip_aor_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to pjsip_aor.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AuthDetail
 * @param j_msg
 */
static void ami_event_authdetail(json_t* j_msg)
{
  json_t* j_data;
  char* timestamp;
  const char* tmp_const;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_authdetail.");

  tmp_const = json_string_value(json_object_get(j_msg, "ObjectName"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get ObjectName info.");
    return;
  }

  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:i, "
      "s:s "
      "}",

      // basic info
      "object_type",    json_string_value(json_object_get(j_msg, "ObjectType"))? : "",
      "object_name",    json_string_value(json_object_get(j_msg, "ObjectName"))? : "",
      "username",       json_string_value(json_object_get(j_msg, "Username"))? : "",
      "endpoint_name",  json_string_value(json_object_get(j_msg, "EndpointName"))? : "",

      // credential
      "auth_type",  json_string_value(json_object_get(j_msg, "AuthType"))? : "",
      "password",   json_string_value(json_object_get(j_msg, "Password"))? : "",
      "md5_cred",   json_string_value(json_object_get(j_msg, "Md5Cred"))? : "",

      // etc
      "realm",          json_string_value(json_object_get(j_msg, "Realm"))? : "",
      "nonce_lifetime", json_string_value(json_object_get(j_msg, "NonceLifetime"))? atoi(json_string_value(json_object_get(j_msg, "NonceLifetime"))): 0,

      // timestamp. UTC."
      "tm_update", timestamp
      );
  sfree(timestamp);

  // create info
  ret = create_pjsip_auth_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to pjsip_aor.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: VoicemailUserEntry
 * @param j_msg
 */
static void ami_event_voicemailuserentry(json_t* j_msg)
{
  json_t* j_data;
  char* timestamp;
  char* id;
  const char* context;
  const char* mailbox;
  int ret;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_voicemailuserentry.");

  context = json_string_value(json_object_get(j_msg, "VMContext"));
  if(context == NULL) {
    slog(LOG_ERR, "Could not get VMContext info.");
    return;
  }

  mailbox = json_string_value(json_object_get(j_msg, "VoiceMailbox"));
  if(mailbox == NULL) {
    slog(LOG_ERR, "Could not get VoiceMailbox info.");
    return;
  }

  // create id
  asprintf(&id, "%s@%s", mailbox, context);

  // create info
  timestamp = get_utc_timestamp();
  j_data = json_pack("{"
      "s:s, s:s, s:s, "   // basic info
      "s:s, s:s, s:s, "   // user info
      "s:s, s:s, s:s, "   // mail setting

      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:i, s:s, s:s, "
      "s:s, s:s, s:s, s:f, s:s, s:s, "

      "s:i, s:i, s:i, s:i, "   // message info
      "s:s, s:s, s:s, s:s, "    // imap info

      "s:s "    // timestamp
      "}",

      // basic info
      "id",      id,
      "context", context,
      "mailbox", mailbox,

      // user info
      "full_name",    json_string_value(json_object_get(j_msg, "Fullname"))? : "",
      "email",        json_string_value(json_object_get(j_msg, "Email"))? : "",
      "pager",        json_string_value(json_object_get(j_msg, "Pager"))? : "",

      // mail setting
      "server_email",   json_string_value(json_object_get(j_msg, "ServerEmail"))? : "",
      "from_string",    json_string_value(json_object_get(j_msg, "FromString"))? : "",
      "mail_command",   json_string_value(json_object_get(j_msg, "MailCommand"))? : "",


      "language",       json_string_value(json_object_get(j_msg, "Language"))? : "",
      "timezone",       json_string_value(json_object_get(j_msg, "TimeZone"))? : "",
      "callback",       json_string_value(json_object_get(j_msg, "Callback"))? : "",
      "dialout",        json_string_value(json_object_get(j_msg, "Dialout"))? : "",
      "unique_id",      json_string_value(json_object_get(j_msg, "UniqueID"))? : "",
      "exit_context",   json_string_value(json_object_get(j_msg, "ExitContext"))? : "",

      "say_duration_minimum",   json_string_value(json_object_get(j_msg, "SayDurationMinimum"))? atoi(json_string_value(json_object_get(j_msg, "SayDurationMinimum"))) : 0,
      "say_envelope",           json_string_value(json_object_get(j_msg, "SayEnvelope"))? : "",
      "say_cid",                json_string_value(json_object_get(j_msg, "SayCID"))? : "",

      "attach_message",       json_string_value(json_object_get(j_msg, "AttachMessage"))? : "",
      "attachment_format",   json_string_value(json_object_get(j_msg, "AttachmentFormat"))? : "",
      "delete_message",       json_string_value(json_object_get(j_msg, "DeleteMessage"))? : "",
      "volume_gain",          json_string_value(json_object_get(j_msg, "VolumeGain"))? atof(json_string_value(json_object_get(j_msg, "VolumeGain"))) : 0,
      "can_review",           json_string_value(json_object_get(j_msg, "CanReview"))? : "",
      "call_operator",        json_string_value(json_object_get(j_msg, "CallOperator"))? : "",


      // message info
      "max_message_count",    json_string_value(json_object_get(j_msg, "MaxMessageCount"))? atoi(json_string_value(json_object_get(j_msg, "MaxMessageCount"))) : 0,
      "max_message_length",   json_string_value(json_object_get(j_msg, "MaxMessageLength"))? atoi(json_string_value(json_object_get(j_msg, "MaxMessageLength"))) : 0,
      "new_message_count",    json_string_value(json_object_get(j_msg, "NewMessageCount"))? atoi(json_string_value(json_object_get(j_msg, "NewMessageCount"))) : 0,
      "old_message_count",    json_string_value(json_object_get(j_msg, "OldMessageCount"))? atoi(json_string_value(json_object_get(j_msg, "OldMessageCount"))) : 0,

      // imap info
      "imap_user",      json_string_value(json_object_get(j_msg, "IMAPUser"))? : "",
      "imap_server",    json_string_value(json_object_get(j_msg, "IMAPServer"))? : "",
      "imap_port",      json_string_value(json_object_get(j_msg, "IMAPPort"))? : "",
      "imap_flag",      json_string_value(json_object_get(j_msg, "IMAPFlags"))? : "",


      // timestamp. UTC."
      "tm_update", timestamp
      );
  sfree(timestamp);
  sfree(id);

  // create info
  ret = create_voicemail_user_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to voicemail_user.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: CoreShowChannel
 * @param j_msg
 */
static void ami_event_coreshowchannel(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;
  const char* tmp_const;
  int sec;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_coreshowchannel.");

  // convert duration
  tmp_const = json_string_value(json_object_get(j_msg, "Duration"));
  sec = convert_time_string(tmp_const, "%H:%M:%S");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:i, s:s, "
      "s:{}"
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "application",   json_string_value(json_object_get(j_msg, "Application"))? : "",
      "application_data",   json_string_value(json_object_get(j_msg, "ApplicationData"))? : "",

      "duration",   sec,
      "bridge_id",  json_string_value(json_object_get(j_msg, "BridgeId"))? : "",

      "variables",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // create channel info
  ret = create_core_channel_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to channel.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Newstate
 * @param j_msg
 */
static void ami_event_newstate(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_newstate.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // update channel info.
  ret = update_core_channel_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update channel info.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Newexten
 * @param j_msg
 */
static void ami_event_newexten(json_t* j_msg)
{
  json_t* j_tmp;
  int ret;
  char* timestamp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_newexten.");

  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s, s:s, "
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",

      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "application",   json_string_value(json_object_get(j_msg, "Application"))? : "",
      "application_data",   json_string_value(json_object_get(j_msg, "ApplicationData"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  // update channel info.
  ret = update_core_channel_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update channel info.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AsyncAGIStart
 * @param j_msg
 */
static void ami_event_asyncagistart(json_t* j_msg)
{
  json_t* j_tmp;
  json_t* j_env;
  int ret;
  char* timestamp;
  const char* env_tmp;
  const char* agi_uuid;
  char* env;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_asyncagistart.");

  // get env
  j_env = NULL;
  env_tmp = json_string_value(json_object_get(j_msg, "Env"));
  if(env_tmp != NULL) {
    env = uri_decode(env_tmp);
    slog(LOG_DEBUG, "Check value. env[%s]", env);
    j_env = parse_ami_agi_env(env);
    sfree(env);
  }

  // creat info
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:{}, "
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "cmd",

      "tm_update",  timestamp
      );
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create message.");
    return;
  }

  if(j_env != NULL) {
    json_object_set_new(j_tmp, "env", j_env);
  }

  // insert info
  ret = create_core_agi_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert to core_agi.");
    return;
  }

  // add cmds
  agi_uuid = json_string_value(json_object_get(j_msg, "Uniqueid"));
  ret = add_dialplan_cmds(agi_uuid);

  return;
}

/**
 * AMI event handler.
 * Event: AsyncAGIEnd
 * @param j_msg
 */
static void ami_event_asyncagiend(json_t* j_msg)
{
  int ret;
  const char* unique_id;
  char* timestamp;
  json_t* j_tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_asyncagiend.");

  unique_id = json_string_value(json_object_get(j_msg, "Uniqueid"));
  if(unique_id == NULL) {
    slog(LOG_ERR, "Could not get unique id info.");
    return;
  }

  // update channel info
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);

  // update info
  ret = update_core_agi_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_agi info.");
    return;
  }

  // delete core_agi info.
  ret = delete_core_agi_info(unique_id);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete core_agi info. unique_id[%s]", unique_id);
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: AsyncAGIExec
 * @param j_msg
 */
static void ami_event_asyncagiexec(json_t* j_msg)
{
  int ret;
  char* tmp;
  char* command_id;
  char* result;
  const char* unique_id;
  const char* tmp_const;
  char* timestamp;
  json_t* j_tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_asyncagiexec.");

  unique_id = json_string_value(json_object_get(j_msg, "Uniqueid"));
  if(unique_id == NULL) {
    slog(LOG_ERR, "Could not get unique id info.");
    return;
  }

  // update channel info
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:i, s:s, "
      "s:s, s:s, s:s, s:s, s:s, s:s, "
      "s:s, s:s, s:s, "
      "s:s"
      "}",

      "unique_id",  json_string_value(json_object_get(j_msg, "Uniqueid"))? : "",
      "linked_id",  json_string_value(json_object_get(j_msg, "Linkedid"))? : "",

      "channel",   json_string_value(json_object_get(j_msg, "Channel"))? : "",
      "channel_state",      json_string_value(json_object_get(j_msg, "ChannelState"))? atoi(json_string_value(json_object_get(j_msg, "ChannelState"))): 0,
      "channel_state_desc", json_string_value(json_object_get(j_msg, "ChannelStateDesc"))? : "",

      "caller_id_num",        json_string_value(json_object_get(j_msg, "CallerIDNum"))? : "",
      "caller_id_name",       json_string_value(json_object_get(j_msg, "CallerIDName"))? : "",
      "connected_line_num",   json_string_value(json_object_get(j_msg, "ConnectedLineNum"))? : "",
      "connected_line_name",  json_string_value(json_object_get(j_msg, "ConnectedLineName"))? : "",
      "language",             json_string_value(json_object_get(j_msg, "Language"))? : "",
      "account_code",         json_string_value(json_object_get(j_msg, "AccountCode"))? : "",

      "context",    json_string_value(json_object_get(j_msg, "Context"))? : "",
      "exten",      json_string_value(json_object_get(j_msg, "Exten"))? : "",
      "priority",   json_string_value(json_object_get(j_msg, "Priority"))? : "",

      "tm_update",  timestamp
      );
  sfree(timestamp);

  update_core_agi_info(j_tmp);
  json_decref(j_tmp);


  // get result
  tmp_const = json_string_value(json_object_get(j_msg, "Result"));
  result = uri_decode(tmp_const);
  if(result == NULL) {
    return;
  }

  // get command id
  tmp_const = json_string_value(json_object_get(j_msg, "CommandId"));
  if(tmp_const == NULL) {
    tmp = gen_uuid();
    asprintf(&command_id, "unknown-%s", tmp);
    sfree(tmp);
  }
  else {
    command_id = strdup(tmp_const);
  }

  // update cmd result info.
  ret = update_core_agi_info_cmd_result_done(unique_id, command_id, result);
  sfree(command_id);
  sfree(result);
  if(ret == false) {
    slog(LOG_ERR, "Could not update core_agi cmd result info.");
    return;
  }

  return;
}

/**
 * AMI event handler.
 * Event: Reload
 * @param j_msg
 */
static void ami_event_reload(json_t* j_msg)
{
  int ret;
  char* timestamp;
  const char* module;
  const char* status;
  json_t* j_tmp;

  if(j_msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired ami_event_reload.");

  // get module info
  module = json_string_value(json_object_get(j_msg, "Module"));
  if(module == NULL) {
    slog(LOG_ERR, "Could not get module info.");
    return;
  }

  // get status info
  status = json_string_value(json_object_get(j_msg, "Status"));
  if(status == NULL) {
    slog(LOG_ERR, "Could not get status info.");
    return;
  }

  // check status info
  // the only 0(Success) value is valid
  ret = strcmp(status, "0");
  if(ret != 0) {
    slog(LOG_ERR, "Invalid status. status[%s]", status);
    return;
  }

  // parse module
  // fire reload handler
  if(strstr(module, "app_queue.so") != NULL) {
    ret = reload_queue_handler();
  }
  else if(strstr(module, "res_parking.so") != NULL) {
    ret = reload_park_handler();
  }
  else if(strstr(module, "res_pjsip.so") != NULL) {
    ret = reload_pjsip_handler();
  }
  else if(strstr(module, "chan_sip.so") != NULL) {
    ret = reload_sip_handler();
  }

  // update core_module info
  timestamp = get_utc_timestamp();
  j_tmp = json_pack("{s:s, s:s, s:s}",
      "name",       module,
      "load",       "loaded",
      "tm_update",  timestamp
      );
  sfree(timestamp);

  ret = update_core_module_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update module info.");
    return;
  }

  return;
}

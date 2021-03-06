

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>
#include <unistd.h>

#include "common.h"
#include "slog.h"
#include "config.h"
#include "db_ctx_handler.h"
#include "http_handler.h"
#include "event_handler.h"
#include "data_handler.h"
#include "resource_handler.h"
#include "misc_handler.h"
#include "ob_event_handler.h"
#include "zmq_handler.h"
#include "websocket_handler.h"
#include "conf_handler.h"
#include "user_handler.h"

#include "park_handler.h"
#include "queue_handler.h"
#include "pjsip_handler.h"
#include "sip_handler.h"

app* g_app;


static bool option_parse(int argc, char** argv);
static void print_help(void);

/**
 * Initiate jade_backend
 * @return
 */
bool init(void)
{
  int ret;
  
  g_app = calloc(sizeof(app), 1);
  g_app->j_conf = NULL;
  g_app->evt_base = NULL;

  ret = init_log();
  if(ret == false) {
    printf("Failed initiate log.");
    return false;
  }
  slog(LOG_NOTICE, "Finished init_log.");

  ret = init_config();
  if(ret == false) {
    printf("Could not initiate config.");
    return false;
  }
  slog(LOG_NOTICE, "Finished init_config.");

  ret = init_event_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate event_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_event_handler.");

  ret = init_resource_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate resource.");
    return false;
  }
  slog(LOG_NOTICE, "Finished resource_init.");

  ret = init_zmq_handler();
  if(ret == false) {
    slog(LOG_ERR, "Coudl not initiate zmq_handler.");
    return false;
  }

  ret = init_data_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ami_handle.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_ami_handler.");

  ret = init_http_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate http_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_http_handler.");
  
  ret = init_websocket_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate websocket_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_websocket_handler.");

  ret = init_outbound();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ob_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_outbound.");

  ret = init_conf_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate conf_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_conf_handler.");

  ret = init_user_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate user_handler");
    return false;
  }

  ret = init_misc_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate misc_handler.");
    return false;
  }

  // other module handlers
  ret = init_park_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate park_handler.");
    return false;
  }

  ret = init_queue_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate queue_handler.");
    return false;
  }

  ret = init_pjsip_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initate pjsip_handler.");
    return false;
  }

  ret = init_sip_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_handler.");
    return false;
  }

  return true;
}

bool terminate(void)
{
  slog(LOG_INFO, "Terminating..");

  term_http_handler();

  // terminate event handler
  term_event_handler();

  // terminate outbound module.
  term_outbound();

  // terminate zmq
  term_zmq_handler();

  term_websocket_handler();

  term_resource_handler();

  event_base_free(g_app->evt_base);
  g_app->evt_base = NULL;

  return true;
}

int main(int argc, char** argv) 
{
  int ret;
  
  // parse options
  ret = option_parse(argc, argv);
  if(ret == false) {
    return 1;
  }

  // initiate
  ret = init();
  if(ret == false) {
    printf("Could not initiate.\n");
    return 1;
  }
  slog(LOG_INFO, "Started backend service.");
    
  event_base_loop(g_app->evt_base, 0);

  terminate();

  return 0;
}

static void print_help(void)
{
  printf("Usage: jade_backend [OPTIONS]\n");
  printf("Valid options:\n");
  printf("  -h              : This help screen.\n");
  printf("  -c <configfile> : Use an alternate configuration file.\n");
  printf("\n");
  return;
}

/**
 * Parse option.
 * @param argc
 * @param argv
 * @return
 */
static bool option_parse(int argc, char** argv)
{
  char opt;

  if(argc > 1) {
    opt = getopt(argc, argv, "hc:");
    if(opt == -1) {
      print_help();
      return false;
    }

    switch(opt) {
      case 'f':
      {
        update_config_filename(optarg);
      }
      break;

      case 'h':
      default:
      {
        print_help();
        return false;
      }
    }
  }
  return true;
}

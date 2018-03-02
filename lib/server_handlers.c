#include <stdlib.h>
#include <stdio.h>
#include "request.h"
#include "response.h"
#include "networklib.h"
#include "state.h"
#include "server_handlers.h"

response * handle_connect(request * req, state * s);
response * handle_update(request * req, state * s);
response * handle_disconnect(request * req, state * s);
response * handle_error(enum error_type e);
void handle_request(state * user_states, int server_fd);
request * get_request(FILE * client_file);
response * interpret_request(request * req, state * s);
void send_response(response * res, FILE * client_file);
void cleanup(request * req, response * res);

int handle_requests(const char * HOST, const char * PORT) {
  int server_fd = socket_listen(HOST, PORT);
  if (server_fd < 0) {
    return 1;
  }

  state * user_states = init_state();
  while (1) {
    handle_request(user_states, server_fd);
  }
}

void handle_request(state * s, int server_fd) {
  FILE * client_file = accept_client(server_fd);
  if (client_file == NULL) return;

  request * req = get_request(client_file);
  response * res = interpret_request(req, s);
  send_response(res, client_file);
  cleanup(req, res);
  fclose(client_file);
}

void cleanup(request * req, response * res) {
  if (req) {
    free(req);
  }
  if (res) {
    free(res);
  }
}

request * get_request(FILE * client_file) {
  request * req = malloc(sizeof(request));
  if (!req) return NULL;

  while (fread((char *)req, sizeof(request), 1, client_file) != 1) continue;

  return req;
}

void send_response(response * res, FILE * client_file) {
  if (!res) return;
  fwrite((char *)res, sizeof(response), 1, client_file);
}

response * interpret_request(request * req, state * s) {
  if (!req) return NULL;
  switch (req->type) {
    case REQCONNECT:
      return handle_connect(req, s);
    case REQUPDATE:
      return handle_update(req, s);
    case REQDISCONNECT:
      return handle_disconnect(req, s);
    default:
      return handle_error(ERNOTYPE);
  }
}

response * handle_error(enum error_type e) {
  response * res = malloc(sizeof(response));
  if (!res) return NULL;
  res->type = RESERROR;
  res->content.error.error = e;
  return res;
}

response * handle_update(request * req, state * s) {
  struct user * u = find_user(s, req->content.update.session_id);
  if (!u) return handle_error(ERINVALIDSESSION);
  response * res = malloc(sizeof(response));
  if (!res) return NULL;
  res->type = RESUPDATE;
  res->content.update.length = 0; //TODO: implement actual content here
  return res;
}

response * handle_disconnect(request * req, state * s) {
  struct user * u = find_user(s, req->content.update.session_id);
  if (!u) return handle_error(ERINVALIDSESSION);
  response * res = malloc(sizeof(response));
  if (!res) return NULL;
  res->type = RESDISCONNECT;
  return res;
}

response * handle_connect(request * req, state * s) {
  int id;
  if ((id = new_user(s)) < 0) return handle_error(ERINTERNAL);
  response * res = malloc(sizeof(response));
  if (!res) return NULL;
  res->type = RESCONNECT;
  res->content.connect.session_id = id;
  return res;
}


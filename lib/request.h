#pragma once
#define IN_BUFSIZ 128
enum request_type {
  REQCONNECT,
  REQUPDATE,
  REQDISCONNECT
};


struct connect_request {
  int width;
  int height;
};


struct update_request {
  char buffer[IN_BUFSIZ];
  int length;
  int session_id;
};


struct disconnect_request {
  int session_id;
};


union request_content {
  struct connect_request connect;
  struct update_request update;
  struct disconnect_request disconnect;
};


typedef struct request {
  enum request_type type;
  union request_content content;
} request;

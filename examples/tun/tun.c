#include "mongoose.h"

static const char *s_local_port = ":8001";
static const char *s_dispatcher = "ws://localhost:8000";
static const char *s_user = "foo";
static const char *s_pass = "bar";

void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;
  switch (ev) {
    case MG_EV_ACCEPT:
      fprintf(stderr, "HTTP accept. nc=%p\n", nc);
      break;
    case MG_EV_RECV:
      fprintf(stderr, "recvd: %d bytes\n", *(int *) ev_data);
      break;
    case MG_EV_HTTP_REQUEST:
      fprintf(stderr, "HTTP got request. nc=%p path=%.*s\n", nc,
              (int) hm->uri.len, hm->uri.p);
      mg_http_send_error(nc, 200, "OK");
      break;
    case MG_EV_CLOSE:
      fprintf(stderr, "HTTP close\n");
    default:
      break;
  }
}

int main(int argc, char **argv) {
  struct mg_mgr mgr;
  struct mg_connection *nc;
  int i;

  mg_mgr_init(&mgr, NULL);

  /* Parse command line arguments */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-D") == 0) {
      mgr.hexdump_file = argv[++i];
    } else if (strcmp(argv[i], "-l") == 0) {
      s_local_port = argv[++i];
    } else if (strcmp(argv[i], "-d") == 0) {
      s_dispatcher = argv[++i];
    } else if (strcmp(argv[i], "-u") == 0) {
      s_user = argv[++i];
    } else if (strcmp(argv[i], "-p") == 0) {
      s_pass = argv[++i];
    }
  }

  if ((nc = mg_tuna_bind(&mgr, ev_handler, s_dispatcher, s_user, s_pass)) ==
      NULL) {
    fprintf(stderr, "Cannot create tunneled listening socket on [%s]\n",
            s_dispatcher);
    exit(EXIT_FAILURE);
  }
  mg_set_protocol_http_websocket(nc);
  fprintf(stderr, "Tun listener: %p\n", nc);

  if ((nc = mg_bind(&mgr, s_local_port, ev_handler)) == NULL) {
    fprintf(stderr, "Cannot bind to local port %s\n", s_local_port);
    exit(EXIT_FAILURE);
  }
  mg_set_protocol_http_websocket(nc);
  fprintf(stderr, "Local listening connection: %p\n", nc);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
}

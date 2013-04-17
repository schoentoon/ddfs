/*  ddfs
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "listener.h"

#include "client.h"

#include <string.h>

#include <event2/listener.h>

#ifndef NO_OPENSSL

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <event2/bufferevent_ssl.h>

char* private_key = NULL;
char* certificate = NULL;

SSL_CTX* ssl_context = NULL;

#endif

struct evconnlistener *listener = NULL;

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd
                       ,struct sockaddr *sa, int socklen, void *context);

int initListener(struct event_base* event_base, unsigned short listen_port)
{
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(listen_port);
#ifndef NO_OPENSSL
  if (private_key && certificate) {
    SSL_load_error_strings();
    SSL_library_init();
    RAND_poll();
    ssl_context = SSL_CTX_new(SSLv23_server_method());
    if (!SSL_CTX_use_certificate_chain_file(ssl_context, certificate) ||
        !SSL_CTX_use_PrivateKey_file(ssl_context, private_key, SSL_FILETYPE_PEM)) {
      fprintf(stderr, "Couldn't read '%s' or '%s' file.  To generate a key "
          "and self-signed certificate, run:\n"
          "  openssl genrsa -out %s 2048\n"
          "  openssl req -new -key %s -out %s.req\n"
          "  openssl x509 -req -days 365 -in %s.req -signkey %s -out %s\n"
          , certificate, private_key, private_key, private_key, certificate, certificate, private_key, certificate);
      exit(1);
    }
    SSL_CTX_set_options(ssl_context, SSL_OP_NO_SSLv2);
  }
#endif
  listener = evconnlistener_new_bind(event_base, listener_cb, event_base
                                    ,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1
                                    ,(struct sockaddr*) &sin, sizeof(sin));
  if (!listener) {
    fprintf(stderr, "There was an error listening on port %d.", listen_port);
    return 0;
  }
  return 1;
}

void closeListener()
{
  evconnlistener_free(listener);
  listener = NULL;
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd
                       ,struct sockaddr *sa, int socklen, void *context)
{
  struct event_base* base = (struct event_base*) context;
  struct bufferevent* bev = NULL;
#ifndef NO_OPENSSL
  if (ssl_context)
    bev = bufferevent_openssl_socket_new(base, fd, SSL_new(ssl_context),
                                         BUFFEREVENT_SSL_ACCEPTING,
                                         BEV_OPT_CLOSE_ON_FREE);
  else
#endif
  bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    fprintf(stderr, "Error creating bufferevent for fd %d.", fd);
    event_base_loopbreak(base);
    return;
  }
  add_client(bev);
}
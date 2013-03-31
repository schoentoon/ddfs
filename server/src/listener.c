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

struct evconnlistener *listener = NULL;

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd
                       ,struct sockaddr *sa, int socklen, void *context);

int initListener(struct event_base* event_base, unsigned short listen_port)
{
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(listen_port);
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
  struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    fprintf(stderr, "Error creating bufferevent for fd %d.", fd);
    event_base_loopbreak(base);
    return;
  }
  add_client(bev);
}
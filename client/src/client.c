/*  isyf
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
#include "client.h"
#include "../../server/include/defines.h"

#include <stdio.h>
#include <string.h>

#include <event2/dns.h>
#include <stdlib.h>

char* server = NULL;
unsigned short port = 9002;
char* folder = NULL;

struct evdns_base* dns = NULL;

static void read_cb(struct bufferevent* bev, void* ctx);
static void event_cb(struct bufferevent* bev, short events, void* ctx);

int startClient(struct event_base* event_base)
{
  if (!dns)
    dns = evdns_base_new(event_base, 1);
  struct bufferevent* bev = bufferevent_socket_new(event_base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
  bufferevent_enable(bev, EV_READ);
  return bufferevent_socket_connect_hostname(bev, dns, AF_INET, server, port);
}

static void read_cb(struct bufferevent* bev, void* ctx)
{
  struct evbuffer* buffer = bufferevent_get_input(bev);
  do {
    size_t len;
    char* header = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
    if (len > 0) {
      unsigned long bytes;
      char filename[strlen(header)];
      if (sscanf(header, "%ld:%s", &bytes, filename) == 2) {
#ifdef DEV
        printf("File: %s is %ld bytes.\n", filename, bytes);
#endif
        while (bytes > 0) {
          size_t read_size = (bytes > BUFFER_SIZE) ? bytes : BUFFER_SIZE;
          char buf[read_size];
          size_t read;
          if ((read = bufferevent_read(bev, &buf, read_size))) {
            read_size -= read;
#ifdef DEV
            printf("Buffer: %s\n", buf);
#endif
          }
        }
      }
    }
    free(header);
  } while (evbuffer_get_length(buffer) > 0);
}

static void event_cb(struct bufferevent* bev, short events, void* ctx)
{
  if (events & BEV_EVENT_EOF || events & BEV_ERROR || events & BEV_EVENT_TIMEOUT) {
    startClient(bufferevent_get_base(bev));
    bufferevent_free(bev);
  }
}
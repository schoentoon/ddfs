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
#include <stdlib.h>
#include <sys/stat.h>

#include <event2/dns.h>

char* server = NULL;
unsigned short port = 9002;
char* folder = NULL;

struct evdns_base* dns = NULL;

static void read_cb(struct bufferevent* bev, void* ctx);
static void event_cb(struct bufferevent* bev, short events, void* ctx);
static void createDir(char* filename);

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
        createDir(filename);
        FILE* file = fopen(filename, "wb");
        while (bytes > 0) {
          size_t read_size = (bytes < BUFFER_SIZE) ? bytes : BUFFER_SIZE;
          char buf[read_size];
#ifdef DEV
          printf("bufferevent_read() %d\n", bufferevent_read(bev, &buf, read_size));
#endif DEV
          bytes -= read_size;
          if (file)
            fwrite(&buf, 1, read_size, file);
#ifdef DEV
          printf("read_size: %zu, bytes: %ld\n", read_size, bytes);
#endif DEV
        }
        if (file) {
          fflush(file);
          fclose(file);
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

static void createDir(char* filename)
{
  size_t len = strlen(filename);
  size_t last_dir = 0;
  int i;
  for (i = 0; i < len; i++) {
    if (filename[i] == '/')
      last_dir = i;
  }
  if (last_dir != 0) {
    filename[last_dir] = '\0';
#ifdef DEV
    printf("Creating dir: %s\n", filename);
#endif
    mkdir(filename, 0700);
    filename[last_dir] = '/';
  }
}
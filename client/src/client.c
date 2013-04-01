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
#include "client.h"
#include "defines.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <event2/dns.h>

char* server = NULL;
unsigned short port = 9002;
char* folder = NULL;
unsigned short backoff = 10;

struct evdns_base* dns = NULL;

struct client {
  FILE* file;
  char* filename;
  unsigned long bytes_left;
};

static void read_cb(struct bufferevent* bev, void* ctx);
static void event_cb(struct bufferevent* bev, short events, void* ctx);
static void createDir(char* filename);

int startClient(struct event_base* event_base)
{
  if (!dns)
    dns = evdns_base_new(event_base, 1);
  struct bufferevent* bev = bufferevent_socket_new(event_base, -1, BEV_OPT_CLOSE_ON_FREE);
  struct client* client = malloc(sizeof(struct client));
  client->file = NULL;
  client->filename = NULL;
  client->bytes_left = 0;
  bufferevent_setcb(bev, read_cb, NULL, event_cb, client);
  bufferevent_enable(bev, EV_READ);
  return bufferevent_socket_connect_hostname(bev, dns, AF_INET, server, port);
}

static void read_cb(struct bufferevent* bev, void* ctx)
{
  struct client* client = (struct client*) ctx;
  struct evbuffer* buffer = bufferevent_get_input(bev);
  if (client->bytes_left == 0) {
    size_t len;
    char* header = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF);
    if (len > 0) {
      char filename[strlen(header)];
      if (sscanf(header, "%ld:%s", &client->bytes_left, filename) == 2) {
#ifdef DEV
        printf("File: %s is %ld bytes.\n", filename, client->bytes_left);
#endif
        createDir(filename);
        client->file = fopen(filename, "wb");
        client->filename = malloc(strlen(filename));
        strcpy(filename, client->filename);
      }
    }
    free(header);
  }
  while (client->bytes_left > 0) {
    size_t read_size = (client->bytes_left < BUFFER_SIZE) ? client->bytes_left : BUFFER_SIZE;
    if (evbuffer_get_length(buffer) < read_size)
      return; /*Let's just get back later.. */
    char buf[read_size];
    client->bytes_left -= bufferevent_read(bev, &buf, read_size);
    if (client->file) {
      if (fwrite(&buf, 1, read_size, client->file) != read_size) {
        fclose(client->file);
        if (remove(client->filename) != 0)
          fprintf(stderr, "Failed to remove %s.\n", client->filename);
        client->file = NULL;
        free(client->filename);
        client->filename = NULL;
      }
    }
  }
  if (client->file && client->bytes_left == 0) {
    fflush(client->file);
    fclose(client->file);
    client->file = NULL;
    free(client->filename);
    client->filename = NULL;
  }
  if (evbuffer_get_length(buffer) > 0)
    read_cb(bev, ctx);
}

static void startClientTimer(evutil_socket_t fd, short events, void* ctx)
{
  startClient((struct event_base*) ctx);
}

static void event_cb(struct bufferevent* bev, short events, void* ctx)
{
  if (events != BEV_EVENT_CONNECTED) {
    struct event_base* base = bufferevent_get_base(bev);
    struct event* timer = evtimer_new(base, startClientTimer, base);
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = backoff;
    tv.tv_usec = 0;
    event_add(timer, &tv);
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
    int output = mkdir(filename, 0700);
#ifdef DEV
    if (output == 0)
      printf("Creating dir: %s\n", filename);
#else
    (void) output; /* Shut up compiler.. */
#endif
    filename[last_dir] = '/';
  }
}
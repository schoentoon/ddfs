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

#include <string.h>
#include <stdlib.h>

#include <event.h>

struct client* clients = NULL;

unsigned int count_clients()
{
  unsigned int count = 0;
  struct client* node = clients;
  while (node) {
    count++;
    node = node->next;
  }
  return count;
}

unsigned int write_to_clients(const char* data, size_t size)
{
  unsigned int count = 0;
  struct client* node = clients;
  while (node) {
    if (bufferevent_write(node->bev, data, size) == 0)
      count++;
    node = node->next;
  }
  return count;
}

void add_client(struct bufferevent* bev)
{
  struct client* client = malloc(sizeof(struct client));
  client->bev = bev;
  client->keepalive = NULL;
  client->next = NULL;
  bufferevent_setcb(bev, client_readcb, NULL, client_eventcb, client);
  bufferevent_enable(bev, EV_READ);
  if (clients == NULL)
    clients = client;
  else {
    struct client* node = clients;
    while (node->next)
      node = node->next;
    node->next = client;
  }
#ifdef DEV
  printf("There are %d clients left.\n", count_clients());
#endif
}

void free_client(struct client* client)
{
  struct client* node = clients;
  if (node == client)
    clients = node->next;
  else {
    while (node) {
      if (node->next == client) {
        node->next = node->next->next;
        break;
      }
      node = node->next;
    }
  }
  bufferevent_free(client->bev);
  if (client->keepalive) {
    event_del(client->keepalive);
    event_free(client->keepalive);
  }
  free(client);
}

static void keep_alive_timer(evutil_socket_t fd, short event, void *context)
{
  struct client* client = (struct client*) context;
  char keepalive[] = { '\n' };
  bufferevent_write(client->bev, &keepalive, 1);
}

void client_readcb(struct bufferevent* bev, void* context)
{
  struct client* client = (struct client*) context;
  struct evbuffer* buffer = bufferevent_get_input(bev);
  char* line = NULL;
  size_t len;
  while ((line = evbuffer_readln(buffer, &len, EVBUFFER_EOL_CRLF))) {
    char* key = malloc(len);
    unsigned int value;
    if (sscanf(line, "%d:%s", &value, key) == 2) {
#ifdef DEV
      printf("Key: %s value: %d\n", key, value);
#endif
      if (strcmp(key, "keepalive") == 0) {
        if (client->keepalive == NULL && value > 0) {
          client->keepalive = event_new(bufferevent_get_base(bev), -1, EV_PERSIST, keep_alive_timer, client);
          struct timeval tv;
          evutil_timerclear(&tv);
          tv.tv_sec = value;
          tv.tv_usec = 0;
          event_add(client->keepalive, &tv);
        } else if (client->keepalive) {
          event_del(client->keepalive);
          event_free(client->keepalive);
          client->keepalive = NULL;
          if (value > 0) {
            client->keepalive = event_new(bufferevent_get_base(bev), -1, EV_PERSIST, keep_alive_timer, client);
            struct timeval tv;
            evutil_timerclear(&tv);
            tv.tv_sec = value;
            tv.tv_usec = 0;
            event_add(client->keepalive, &tv);
          }
        }
      }
    }
    free(key);
#ifdef DEV
    printf("From client: %s\n", line);
#endif
    free(line);
  }
}

void client_eventcb(struct bufferevent* bev, short events, void* context)
{
  if (events & BEV_EVENT_EOF)
    free_client((struct client*) context);
#ifdef DEV
  printf("There are %d clients left.\n", count_clients());
#endif
}
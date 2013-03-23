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

#include <stdlib.h>

#include <event.h>

struct client* clients = NULL;

void add_client(struct bufferevent* bev)
{
  struct client* client = malloc(sizeof(struct client));
  client->bev = bev;
  client->next = NULL;
  bufferevent_setcb(bev, client_readcb, client_writecb, client_eventcb, client);
  bufferevent_enable(bev, EV_WRITE|EV_READ);
  if (clients == NULL)
    clients = client;
  else {
    struct client* node = clients;
    while (node->next)
      node = node->next;
    node->next = client;
  }
}

void client_readcb(struct bufferevent* bev, void* context)
{
  char buf[4096];
  size_t numRead = bufferevent_read(bev, buf, 4096);
  while (numRead) {
    printf("Buffer: %s\n", buf);
    numRead = bufferevent_read(bev, buf, 4096);
  }
}

void client_writecb(struct bufferevent* bev, void* context)
{
}

void client_eventcb(struct bufferevent* bev, short events, void* context)
{
  if (events & BEV_EVENT_EOF) {
    bufferevent_free(bev);
    struct client* client = (struct client*) context;
    free(client);
  }
}
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
#ifndef _CLIENT_H
#define _CLIENT_H

#include <event.h>

#ifndef NO_OPENSSL
unsigned char openssl;
#endif

char* server;
unsigned short port;

unsigned short backoff;
char* folder;

unsigned int timeout;
unsigned int keepalive;

int startClient(struct event_base* event_base);

void shutdownClient();

#endif //_CLIENT_H
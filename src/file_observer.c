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

#include "file_observer.h"

static int inotifyfd = 0;
static void (*inotifycallback)(struct inotify_event*) = NULL;

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static void readcb(struct bufferevent* bev, void* args)
{
  char buf[BUF_LEN];
  size_t numRead = bufferevent_read(bev, buf, BUF_LEN);
  char *p;
  for (p = buf; p < buf + numRead; ) {
    struct inotify_event *event = (struct inotify_event*) p;
    if (inotifycallback)
      inotifycallback(event);
    p += sizeof(struct inotify_event) + event->len;
  }
}

int initFileObserver(struct event_base* event_base, void (*callback)(struct inotify_event*))
{
  if (inotifyfd <= 0)
    inotifyfd = inotify_init();
  if (inotifyfd == -1)
    return 0;
  inotifycallback = callback;
  struct bufferevent *bufferevent = bufferevent_socket_new(event_base, inotifyfd, 0);
  bufferevent_setcb(bufferevent, readcb, NULL, NULL, NULL);
  bufferevent_enable(bufferevent, EV_READ);
  return 1;
}

int watch_folder(const char* folder, uint32_t mask)
{
  int wd = inotify_add_watch(inotifyfd, folder, IN_ALL_EVENTS);
  if (wd == -1)
    return 0;
  return 1;
}
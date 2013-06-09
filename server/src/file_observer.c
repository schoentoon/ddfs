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

#include "file_observer.h"

#include <dirent.h>
#include <string.h>

char recursive = 0;

static int inotifyfd = 0;
static void (*inotifycallback)(struct inotify_event*) = NULL;

#define BUF_LEN (1024 * (sizeof(struct inotify_event) + NAME_MAX + 1))

struct folder_wd_container {
  int wd;
  char* folder;
  struct folder_wd_container* next;
};

static struct folder_wd_container* folders = NULL;

static void assign_wd_folder(int wd, const char* folder)
{
  struct folder_wd_container* wd_container = malloc(sizeof(struct folder_wd_container));
  memset(wd_container, 0, sizeof(struct folder_wd_container));
  wd_container->wd = wd;
  wd_container->folder = (char*) folder;
  if (folders) {
    struct folder_wd_container* tmp = folders;
    while (tmp->next)
      tmp = tmp->next;
    tmp->next = wd_container;
  } else
    folders = wd_container;
}

static void readcb(struct bufferevent* bev, void* args)
{
  char buf[BUF_LEN];
  size_t numRead;
  while ((numRead = bufferevent_read(bev, buf, BUF_LEN))) {
    char *p;
    for (p = buf; p < buf + numRead; ) {
      struct inotify_event *event = (struct inotify_event*) p;
      if (inotifycallback)
        inotifycallback(event);
      if (recursive && event->mask & IN_CREATE && event->mask & IN_ISDIR && event->len > 0) {
        char* folder = get_folder(event->wd);
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", folder, folder);
        watch_folder(strdup(fullpath));
      }
      p += sizeof(struct inotify_event) + event->len;
    }
  };
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

#define DEFAULT_MASK (IN_CLOSE_WRITE|IN_DELETE)

int watch_folder(const char* folder)
{
  int wd = inotify_add_watch(inotifyfd, folder, DEFAULT_MASK);
  if (wd == -1) {
    fprintf(stderr, "There was an error adding '%s' to the file observer, error code %d.\n", folder, wd);
    return 0;
  }
  assign_wd_folder(wd, folder);
  printf("Watching folder '%s'.\n", folder);
  if (recursive) {
    DIR* dir = opendir(folder);
    struct dirent *dp = NULL;
    while ((dp = readdir(dir)) != NULL) {
      if (dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0) {
        size_t len = strlen(folder) + strlen(dp->d_name) + 2; /* 1 for / and 1 for \0 */
        char fullpath[len];
        snprintf(fullpath, len, "%s/%s", folder, dp->d_name);
        if (watch_folder(strdup(fullpath)) == 0) {
          closedir(dir);
          return 0;
        }
      }
    }
    closedir(dir);
  }
  return 1;
}

char* get_folder(int wd)
{
  struct folder_wd_container* node = folders;
  while (node) {
    if (node->wd == wd)
      return node->folder;
    node = node->next;
  }
  return NULL;
}
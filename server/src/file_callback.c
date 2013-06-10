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

#include "file_callback.h"

#include "log.h"
#include "client.h"
#include "defines.h"
#include "file_observer.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

char* getFullPath(char* name, int wd) {
  char* folder = get_folder(wd);
  if (!folder)
    return NULL;
  char fullpath[PATH_MAX];
  snprintf(fullpath, sizeof(fullpath), "%s/%s", folder, name);
  return strdup(fullpath);
}

void sendAllFiles(struct inotify_event* event) {
  if (event->mask & IN_CLOSE_WRITE && event->len > 0) {
    char* fullpath = getFullPath(event->name, event->wd);
    if (!fullpath)
      return;
    int fd = open(fullpath, 0);
    if (fd) {
      struct stat st;
      if (fstat(fd, &st) == 0) {
        char header_buf[strlen(fullpath)*2];
        if (snprintf(header_buf, sizeof(header_buf), "\n%ld:%s\n", st.st_size, fullpath)) {
          write_to_clients((const char*) &header_buf, strlen(header_buf));
          char buf[BUFFER_SIZE];
          size_t numRead = 0;
          while ((numRead = read(fd, &buf, BUFFER_SIZE)))
            write_to_clients((const char*) &buf, numRead);
        }
      } else
        fprintf(stderr, "fstat failed on file %s :/\n", fullpath);
    } else
      fprintf(stderr, "There was an error while opening on file %s :/\n", fullpath);
    close(fd);
    free(fullpath);
  } else if (event->mask & IN_DELETE && event->len) {
    char* fullpath = getFullPath(event->name, event->wd);
    if (!fullpath)
      return;
    char header_buf[strlen(fullpath)*2];
    if (snprintf(header_buf, sizeof(header_buf), "\nrm:%s\n", fullpath))
      write_to_clients((const char*) &header_buf, strlen(header_buf));
    free(fullpath);
  }
}
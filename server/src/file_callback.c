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

#include "file_callback.h"

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

char* getFullPath(char* name, int wd)
{
  char* folder = get_folder(wd);
  size_t len = strlen(folder) + strlen(name) + 2; /* 1 for / and 1 for \0 */
  char fullpath[len];
  snprintf(fullpath, len, "%s/%s", folder, name);
  char* output = malloc(len);
  return strcpy(output, fullpath);
}

void sendAllFiles(struct inotify_event* event)
{
#ifdef DEV
  printf("---------------------------\n");
  printf("mask %d\n", event->mask);
  if (event->len > 0)
    printf("name = %s\n", event->name);
  printf("In folder %s\n", get_folder(event->wd));
  printf("---------------------------\n");
#endif
  if (event->mask & IN_CLOSE_WRITE && event->len > 0) {
    char* fullpath = getFullPath(event->name, event->wd);
    int fd = open(fullpath, 0);
    if (fd) {
      printf("Full path: %s\n", fullpath);
      struct stat st;
      if (fstat(fd, &st) == 0) {
        char header_buf[strlen(fullpath)*2];
        snprintf(header_buf, sizeof(header_buf), "\n%ld:%s\n", st.st_size, fullpath);
        write_to_clients((const char*) &header_buf, strlen(header_buf));
        char buf[BUFFER_SIZE];
        size_t numRead = read(fd, &buf, BUFFER_SIZE);
        while (numRead) {
#ifdef DEV //Not printing because it may contain binary data.
          printf("Read %zu bytes.\n", numRead);
#endif
          write_to_clients((const char*) &buf, numRead);
          numRead = read(fd, &buf, BUFFER_SIZE);
        }
      }
    } else
      fprintf(stderr, "There was an error while opening this file :/\n");
    close(fd);
    free(fullpath);
  }
}
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

#include "file_observer.h"

#include <stdlib.h>
#include <string.h>

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
    FILE *file = fopen(fullpath, "rb");
    if (file) {
      printf("Full path: %s\n", fullpath);
      char buf[4096];
      size_t numRead = fread(&buf, 1, 4096, file);
      while (numRead) {
    #ifdef DEV
        printf("Buffer: %s\n", buf);
        //write_to_clients((const char*) &buf, numRead); /* Writing it back to all clients just for testing.. */
    #endif
        numRead = fread(&buf, 1, 4096, file);
      }
    } else
      fprintf(stderr, "There was an error while opening this file :/\n");
    fclose(file);
    free(fullpath);
  }
}
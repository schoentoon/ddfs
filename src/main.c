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

#include <event.h>
#include <getopt.h>
#include <string.h>

#include "file_observer.h"

static const struct option g_LongOpts[] = {
  { "help",      no_argument,       0, 'h' },
  { "folder",    required_argument, 0, 'f' },
  { "recursive", no_argument,       0, 'r' },
  { 0, 0, 0, 0 }
};

void usage()
{
  printf("USAGE: isyf [options]\n");
  printf("-h, --help\tShow this help.\n");
  printf("-f, --folder\tMonitor this folder for files to sync.\n");
  printf("-r, --recursive\tMonitor subfolders as well.\n");
}

void test(struct inotify_event* event)
{
  if (event->len > 0)
    printf("name = %s\n", event->name);
  printf("In folder %s\n", get_folder(event->wd));
}

int main(int argc, char **argv)
{
  int iArg, iOptIndex = -1;
  char recursive = 0;
  struct event_base* event_base = event_base_new();
  initFileObserver(event_base, test);
  while ((iArg = getopt_long(argc, argv, "hrf:", g_LongOpts, &iOptIndex)) != -1) {
    switch (iArg) {
    case 'r':
      recursive = 1;
      break;
    case 'f':
      if (optarg[strlen(optarg)-1] == '/') {
        size_t offset = strlen(optarg)-1;
        optarg[offset] = '\0';
        watch_folder(optarg, recursive, IN_ALL_EVENTS);
        optarg[offset] = '/';
      } else
        watch_folder(optarg, recursive, IN_ALL_EVENTS);
      break;
    default:
    case 'h':
      usage();
      return 0;
    }
  }
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  event_base_free(event_base);
  return 0;
}
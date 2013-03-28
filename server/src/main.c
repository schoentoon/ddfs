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

#include "listener.h"
#include "file_callback.h"
#include "file_observer.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

static const struct option g_LongOpts[] = {
  { "help",      no_argument,       0, 'h' },
  { "folder",    required_argument, 0, 'f' },
  { "recursive", no_argument,       0, 'r' },
  { "port",      required_argument, 0, 'p' },
  { "version",   no_argument,       0, 'v' },
  { 0, 0, 0, 0 }
};

void usage()
{
  printf("USAGE: isyf [options]\n");
  printf("-h, --help\tShow this help.\n");
  printf("-f, --folder\tMonitor this folder for files to sync.\n");
  printf("-r, --recursive\tMonitor subfolders as well.\n");
  printf("-p, --port\tPort to listen on, defaults to 9002.\n");
  printf("-v, --version\tPrint the version.\n");
}

int main(int argc, char **argv)
{
  int iArg, iOptIndex = -1;
  struct event_base* event_base = event_base_new();
  initFileObserver(event_base, sendAllFiles);
  unsigned short listen_port = 9002;
  while ((iArg = getopt_long(argc, argv, "hvrf:p:", g_LongOpts, &iOptIndex)) != -1) {
    switch (iArg) {
    case 'r':
      recursive = 1;
      break;
    case 'f':
      if (optarg[strlen(optarg)-1] == '/') {
        size_t offset = strlen(optarg)-1;
        optarg[offset] = '\0';
        watch_folder(optarg);
      } else
        watch_folder(optarg);
      break;
    case 'p': {
      long tmp = strtol(optarg, NULL, 10);
      if ((errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0) || tmp < 0 || tmp > 65535) {
        fprintf(stderr, "--port requires a valid port.\n");
        return 1;
      }
      listen_port = (unsigned short) tmp;
      break;
    }
    case 'v':
      printf("Isyf Syncs Your Files ~ %s\n", VERSION);
      return 0;
    default:
    case 'h':
      usage();
      return 0;
    }
  }
  initListener(event_base, listen_port);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  event_base_free(event_base);
  return 0;
}
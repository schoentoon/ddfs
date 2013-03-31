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

#include <event.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>

static const struct option g_LongOpts[] = {
  { "folder",    required_argument, 0, 'f' },
  { "help",      no_argument,       0, 'h' },
  { "server",    required_argument, 0, 's' },
  { "version",   no_argument,       0, 'v' },
  { "port",      required_argument, 0, 'p' },
  { "backoff",   required_argument, 0, 'b' },
  { 0, 0, 0, 0 }
};

void usage()
{
  printf("USAGE: isyf [options]\n");
  printf("-h, --help\tShow this help.\n");
  printf("-f, --folder\tWrite files to this folder.\n");
  printf("-s, --server\tServer to connect to.\n");
  printf("-p, --port\tConnect to the server on this port, defaults to 9002.\n");
  printf("-b, --backoff\tWait this amount of seconds between reconnect attempts.\n");
  printf("-v, --version\tPrint the version.\n");
}

int main(int argc, char **argv)
{
  int iArg, iOptIndex = -1;
  while ((iArg = getopt_long(argc, argv, "hvs:p:f:b:", g_LongOpts, &iOptIndex)) != -1) {
    switch (iArg) {
    case 'f':
      folder = optarg;
      break;
    case 'p': {
      long tmp = strtol(optarg, NULL, 10);
      if ((errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0) || tmp < 0 || tmp > 65535) {
        fprintf(stderr, "--port requires a valid port.\n");
        return 1;
      }
      port = (unsigned short) tmp;
      break;
    }
    case 'b': {
      long tmp = strtol(optarg, NULL, 10);
      if ((errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0) || tmp < 0) {
        fprintf(stderr, "--backoff requires a valid amount of seconds.\n");
        return 1;
      }
      backoff = (unsigned short) tmp;
      break;
    }
    case 's':
      server = optarg;
      break;
    case 'v':
      printf("Isyf Syncs Your Files ~ %s\n", VERSION);
      return 0;
    default:
    case 'h':
      usage();
      return 0;
    }
  }
  if (!server) {
    fprintf(stderr, "No server specified.\n");
    return 1;
  } else if (!folder) {
    fprintf(stderr, "No folder specified.\n");
    return 1;
  } else if (chdir(folder) != 0) {
    fprintf(stderr, "Couldn't change work directory to %s, does it even exist?\n", folder);
    return 2;
  }
  struct event_base* event_base = event_base_new();
  startClient(event_base);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  event_base_free(event_base);
  return 0;
}
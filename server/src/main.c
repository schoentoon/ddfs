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

#include "log.h"
#include "listener.h"
#include "file_callback.h"
#include "file_observer.h"

#include <time.h>
#include <event.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>

static const struct option g_LongOpts[] = {
  { "help",        no_argument,       0, 'h' },
  { "folder",      required_argument, 0, 'f' },
  { "recursive",   no_argument,       0, 'r' },
  { "port",        required_argument, 0, 'p' },
  { "version",     no_argument,       0, 'v' },
#ifndef NO_OPENSSL
  { "private-key", required_argument, 0, 'P' },
  { "certificate", required_argument, 0, 'C' },
#endif
  { 0, 0, 0, 0 }
};

void usage() {
  printf("USAGE: ddfs_server [options]\n");
  printf("-h, --help\tShow this help.\n");
  printf("-v, --version\tPrint the version.\n");
  printf("-f, --folder\tMonitor this folder for files to sync.\n");
  printf("-r, --recursive\tMonitor subfolders as well.\n");
  printf("-p, --port\tPort to listen on, defaults to 9002.\n");
#ifndef NO_OPENSSL
  printf("-P, --private-key\tUse this private key file for the ssl.\n");
  printf("-C, --certificate\tUse this certificate file for the ssl.\n");
  printf("To create these files use the following commands.\n");
  printf("openssl genrsa -out pkey 2048\n");
  printf("openssl req -new -key pkey -out cert.req\n");
  printf("openssl x509 -req -days 365 -in cert.req -signkey pkey -out cert\n");
#endif
}

void onSignal(int signal) {
  DEBUG("Received the %d signal.", signal);
  fprintf(stderr, "Shutting down.\n");
  closeListener();
  exit(0);
}

int main(int argc, char **argv) {
  int iArg, iOptIndex = -1;
  struct event_base* event_base = event_base_new();
  initFileObserver(event_base, sendAllFiles);
  unsigned short listen_port = 9002;
#ifndef NO_OPENSSL
  srand(getpid()^time(NULL));
  while ((iArg = getopt_long(argc, argv, "hvrf:p:P:C:", g_LongOpts, &iOptIndex)) != -1) {
#else
  while ((iArg = getopt_long(argc, argv, "hvrf:p:", g_LongOpts, &iOptIndex)) != -1) {
#endif
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
#ifndef NO_OPENSSL
    case 'P':
      private_key = optarg;
      break;
    case 'C':
      certificate = optarg;
      break;
#endif
    case 'v':
      printf("Dumb Distributed File System ~ %s\n", VERSION);
      return 0;
    default:
    case 'h':
      usage();
      return 0;
    }
  }
#ifndef NO_OPENSSL
  if ((certificate && !private_key) || (!certificate && private_key)) {
    fprintf(stderr, "To use openssl you need to specify both the certificate and the private key.\n");
    return 1;
  }
#endif
  initListener(event_base, listen_port);
  signal(SIGINT, onSignal);
  signal(SIGTERM, onSignal);
  signal(SIGSTOP, onSignal);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  event_base_free(event_base);
  return 0;
}
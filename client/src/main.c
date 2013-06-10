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

#include "client.h"
#include "hook.h"

#include <event.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <signal.h>

#ifndef NO_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#endif

static const struct option g_LongOpts[] = {
  { "folder",    required_argument, 0, 'f' },
  { "help",      no_argument,       0, 'h' },
  { "server",    required_argument, 0, 's' },
  { "version",   no_argument,       0, 'v' },
  { "port",      required_argument, 0, 'p' },
  { "backoff",   required_argument, 0, 'b' },
  { "timeout",   required_argument, 0, 't' },
  { "keepalive", required_argument, 0, 'k' },
  { "hook",      required_argument, 0, 'H' },
#ifndef NO_OPENSSL
  { "openssl",   no_argument,       0, 'S' },
#endif
  { 0, 0, 0, 0 }
};

void usage() {
  printf("USAGE: ddfs [options]\n");
  printf("-h, --help\tShow this help.\n");
  printf("-f, --folder\tWrite files to this folder.\n");
  printf("-s, --server\tServer to connect to.\n");
  printf("-p, --port\tConnect to the server on this port, defaults to 9002.\n");
  printf("-b, --backoff\tWait this amount of seconds between reconnect attempts, defaults to 10.\n");
  printf("-H, --hook\tExecute this command when connect, rsync for example.\n");
  printf("-t, --timeout\tUse this amount of seconds as a read timeout, 0 is disabled.\n");
  printf("-k, --keepalive\tTell the server to use this as the keep alive interval, recommended when using --timeout.\n");
  printf("-S, --openssl\tUse openssl to connect to the server.\n");
  printf("-v, --version\tPrint the version.\n");
}

void onSignal(int signal) {
  shutdownClient();
  exit(0);
}

int main(int argc, char **argv) {
  int iArg, iOptIndex = -1;
#ifndef NO_OPENSSL
  srand(getpid()^time(NULL));
  while ((iArg = getopt_long(argc, argv, "hvs:p:f:b:t:k:H:S", g_LongOpts, &iOptIndex)) != -1) {
#else
  while ((iArg = getopt_long(argc, argv, "hvs:p:f:b:t:k:H:", g_LongOpts, &iOptIndex)) != -1) {
#endif
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
    case 't': {
      long tmp = strtol(optarg, NULL, 10);
      if ((errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0) || tmp < 0) {
        fprintf(stderr, "--timeout requires a valid amount of seconds.\n");
        return 1;
      }
      timeout = (unsigned int) tmp;
      break;
    }
    case 'k': {
      long tmp = strtol(optarg, NULL, 10);
      if ((errno == ERANGE || (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0) || tmp < 0) {
        fprintf(stderr, "--keepalive requires a valid amount of seconds.\n");
        return 1;
      }
      keepalive = (unsigned int) tmp;
      break;
    }
    case 's':
      server = optarg;
      break;
    case 'H': {
      struct hook* hook = new_hook();
      hook->executable = optarg;
      break;
    }
#ifndef NO_OPENSSL
    case 'S':
      openssl = 1;
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
  if (!server) {
    fprintf(stderr, "No server specified.\n");
    usage();
    return 1;
  } else if (!folder) {
    fprintf(stderr, "No folder specified.\n");
    usage();
    return 1;
  } else if (chdir(folder) != 0) {
    fprintf(stderr, "Couldn't change work directory to %s, does it even exist?\n", folder);
    usage();
    return 2;
  }
  if (keepalive > timeout)
    fprintf(stderr, "Keepalive is bigger than the timeout, this is NOT recommended.\n");
#ifdef _WIN32
  WSADATA wsa_data;
  WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif //_WIN32
#ifndef NO_OPENSSL
  if (openssl) {
    SSL_load_error_strings();
    SSL_library_init();
    RAND_poll();
  }
#endif
  struct event_base* event_base = event_base_new();
  startClient(event_base);
  signal(SIGINT, onSignal);
  signal(SIGTERM, onSignal);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  event_base_free(event_base);
  return 0;
}
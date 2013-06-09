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

#include "hook.h"

#include "log.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

struct hook *hooks = NULL;

struct hook* new_hook()
{
  struct hook* output = malloc(sizeof(struct hook));
  memset(output, 0, sizeof(struct hook));
  if (!hooks)
    hooks = output;
  else {
    struct hook* node = hooks;
    while (node->next)
      node = node->next;
    node->next = output;
  }
  return output;
}

void* execute_hooks_thread(void* hooks);

void execute_hooks()
{
  if (hooks) {
    pthread_t thread_t;
    pthread_create(&thread_t, NULL, execute_hooks_thread, hooks);
  }
}

void* execute_hooks_thread(void* hooks)
{
  struct hook* node = (struct hook*) hooks;
  while (node) {
    DEBUG("Executing %s\n", node->executable);
    int output = system(node->executable);
    if (output != 0)
      DEBUG("There was an error while executing hook '%s'", node->executable);
    node = node->next;
  }
  return NULL;
}
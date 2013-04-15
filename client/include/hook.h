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

#ifndef _HOOK_H
#define _HOOK_H

struct hook {
  char* executable;
  struct hook *next;
};

/** Used to create a new hook struct
 * This will automatically append it to the
 * list that is used by execute_hooks()
 */
struct hook* new_hook();

/** This will loop through the list of hook
 * structs and will try to execute them. We'll
 * totally ignore any error output from any program.
 */
void execute_hooks();

#endif //_HOOK_H
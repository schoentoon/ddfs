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

#ifndef _FILE_OBSERVER_H
#define _FILE_OBSERVER_H

#include <event.h>
#include <limits.h>
#include <sys/inotify.h>

char recursive;

int initFileObserver(struct event_base* event_base, void (*callback)(struct inotify_event*));

int watch_folder(const char* folder, uint32_t mask);

char* get_folder(int wd);

#endif //_FILE_OBSERVER_H
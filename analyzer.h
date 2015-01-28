/*
 * Skynet
 * Copyright (C) 2015 Andrea Nardinocchi (andrea@nardinan.it)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef skynet_analyzer_h
#define skynet_analyzer_h
#include <miranda/ground.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/dir.h>
typedef int (* t_analyzer_recall)(const char * /* complete path */);
typedef int (* t_analyzer_load)(void);
typedef void (* t_analyzer_destroy)(void);
typedef struct s_analyzer_action {
	const char *extension;
	t_analyzer_recall action;
	t_analyzer_load load;
	t_analyzer_destroy destroy;
} s_analyzer_action;
extern int f_analyze_compare_extension(const char *file, const char *extension);
extern int p_analyze_directory_file(const char *file, struct s_analyzer_action *actions);
extern int f_analyze_directory(const char *directory, struct s_analyzer_action *actions, const char *directory_ignore_list);
#endif

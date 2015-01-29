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
#ifndef skynet_mysql_local_h
#define skynet_mysql_local_h
#include <miranda/ground.h>
#include <mysql.h>
#define d_mysql_local_query_size 5120
typedef int (* t_mysql_local_recall)(MYSQL_ROW entries, size_t elements);
typedef struct s_mysql_local_parameters {
	char *server, *username, *password, *database;
} s_mysql_local_parameters;
extern MYSQL *v_mysql_link;
extern int f_mysql_local_init(struct s_mysql_local_parameters *parameters);
extern int f_mysql_local_run(char *query, t_mysql_local_recall action);
extern void f_mysql_local_destroy(void);
#endif

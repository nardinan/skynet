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
#include "mysql.local.h"
MYSQL *v_mysql_link = NULL;
int f_mysql_local_init(struct s_mysql_local_parameters *parameters) {
	int result = d_false;
	if (!v_mysql_link) {
		v_mysql_link = mysql_init(NULL);
		if (mysql_real_connection(v_mysql_link, parameters->server, parameters->username, parameters->password, parameters->database, 0, NULL, 0))
			result = d_true;
	}
	return result;
}

int f_mysql_local_run(char *query, t_mysql_local_recall action) {
	MYSQL_RES *output;
	MYSQL_ROW output_row;
	int fields, result = d_false;
	if (v_mysql_link)
		if (mysql_query(v_mysql_link, query) == 0) {
			if ((action) && (output = mysql_store_result(v_mysql_link))) {
				fields = mysql_num_fields(output);
				while ((output_row = mysql_fetch_row(output)))
					if (!action(output_row, fields))
						break;
				mysql_free_result(output);
			}
			result = d_true;
		}
	return result;
}

void f_mysql_local_destroy(void) {
	if (v_mysql_link)
		mysql_close(v_mysql_link);
}


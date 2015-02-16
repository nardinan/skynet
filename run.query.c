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
#include "analyzer.h"
#define d_divisor ';'
int f_print_result(MYSQL_ROW entries, size_t elements) {
	int index;
	for (index = 0; index < elements; ++index)
		printf("%s%c", (entries[index])?entries[index]:"[null]", (index==(elements-1))?'\n':d_divisor);
	return d_true;
}

void f_execute(char *file) {
	FILE *stream;
	size_t size, real_dimension;
	char *query;
	if ((stream = fopen(file, "r"))) {
		fseek(stream, 0, SEEK_END);
		if ((size = ftell(stream)) > 0) {
			fseek(stream, 0, SEEK_SET);
			if ((query = (char *) d_malloc(size+1))) {
				if ((real_dimension = fread(query, 1, size, stream)) == size)
					p_mysql_local_run_single(query, f_print_result);
				d_free(query);
			}
		}
		fclose(stream);
	} else
		d_err(e_log_level_ever, "warning, query file %s not found", file);
}

int main (int argc, char *argv[]) {
	struct s_mysql_local_parameters db_parameters = {
		NULL, /* socket connection */
		"root",
		"digitare",
		"dampefm"
	};
	int index;
	v_log_level = e_log_level_ever;
	if (argc >= 2) {
		f_memory_init();
		if (f_mysql_local_init(&db_parameters)) {
			for (index = 1; index < argc; ++index)
				f_execute(argv[index]);
			f_mysql_local_destroy();
		} else
			d_err(e_log_level_ever, "warning, unable to open the database %s", db_parameters.database);
		f_memory_destroy();
	} else
		d_err(e_log_level_ever, "%s <file>.sql", argv[0]);
	return 0;
}

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
		if (mysql_real_connect(v_mysql_link, parameters->server, parameters->username, parameters->password, parameters->database, 0, NULL, 0))
			result = d_true;
	}
	return result;
}

int p_mysql_local_run_sanitize(char *query, struct s_mysql_local_variable *environment) {
	char *pointer = query, *next, *final, *invalid_character, keyword[d_mysql_local_keyword_size], entry[d_mysql_local_entry_size],
	     sanitized_query[d_mysql_local_query_size] = {0};
	int index;
	while ((next = strchr(pointer, '#'))) {
		final = (next+1);
		while ((isalnum(*final)) || (*final == '_'))
			final++;
		memset(keyword, 0, d_mysql_local_keyword_size);
		strncpy(keyword, (next+1), (final-next)-1);
		if (next > pointer)
			strncat(sanitized_query, pointer, (next-pointer)-1);
		if ((f_string_strlen(keyword) > 0) && (environment)) {
			for (index = 0; environment[index].link; ++index)
				if (f_string_strcmp(environment[index].link, keyword) == 0) {
					switch (environment[index].format) {
						case e_mysql_local_format_int:
							snprintf(entry, d_mysql_local_entry_size, " %d ", *((int *)environment[index].variable));
							break;
						case e_mysql_local_format_float:
							snprintf(entry, d_mysql_local_entry_size, " %.02f ", *((float *)environment[index].variable));
							break;
						case e_mysql_local_format_char:
							if (*((char *)environment[index].variable) == d_mysql_local_string_invalid)
								*((char *)environment[index].variable) = d_mysql_local_string_invalid_replace_character;
							snprintf(entry, d_mysql_local_entry_size, " \"%c\" ", *((char *)environment[index].variable));
							break;
						case e_mysql_local_format_string:
							while ((invalid_character = strchr((char *)environment[index].variable, d_mysql_local_string_invalid)))
							       *invalid_character = d_mysql_local_string_invalid_replace_character;
							snprintf(entry, d_mysql_local_entry_size, " \"%s\" ", (char *)environment[index].variable);
							break;
					}
					strncat(sanitized_query, entry, (d_mysql_local_query_size-f_string_strlen(sanitized_query)));
					break;
				}
			if (!environment[index].link)
				d_log(e_log_level_high, "warning, keyword #%s was not defined in the current context", keyword);
		}
		pointer = final;
	}
	if (f_string_strlen(pointer))
		strcat(sanitized_query, pointer);
	return mysql_query(v_mysql_link, sanitized_query);
}

int f_mysql_local_run(char *query, struct s_mysql_local_variable *environment, t_mysql_local_recall action) {
	MYSQL_RES *output;
	MYSQL_ROW output_row;
	int fields, result = d_false;
	if (v_mysql_link)
		if (p_mysql_local_run_sanitize(query, environment) == 0) {
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

int f_mysql_local_run_file(const char *file, struct s_mysql_local_variable *environment, t_mysql_local_recall action) {
	FILE *stream;
	char *query;
	size_t size, real_dimension;
	int result = d_false;
	if ((stream = fopen(file, "r"))) {
		fseek(stream, 0, SEEK_END);
		if ((size = ftell(stream)) > 0) {
			fseek(stream, 0, SEEK_SET);
			if ((query = (char *) d_malloc(size+1))) {
				if ((real_dimension = fread(query, 1, size, stream)) == size) {
					query[size] = '\0';
					result = f_mysql_local_run(query, environment, action);
				}
				d_free(query);
			}
		}
		fclose(stream);
	} else
		d_log(e_log_level_high, "warning, query file %s not found", file);
	return result;
}

void f_mysql_local_destroy(void) {
	if (v_mysql_link)
		mysql_close(v_mysql_link);
}


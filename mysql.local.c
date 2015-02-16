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

char *f_mysql_local_sanitize(char *raw_query, char *sanitized_query, size_t *computed_size, size_t size, struct s_mysql_local_variable *environment) {
	char *pointer = raw_query, *next, *last, *invalid_character, keyword[d_mysql_local_keyword_size], entry[d_mysql_local_entry_size] = {0};
	size_t dimension, remaining = size-1, lower;
	int index;
	*computed_size = 0;
	memset(sanitized_query, 0, size);
	while ((next = strchr(pointer, '#'))) {
		last = (next+1);
		while ((isalnum(*last)) || (strchr("_.-", *last)))
			last++;
		memset(keyword, 0, d_mysql_local_keyword_size);
		if ((dimension = (last-next)-1) > 0)
			strncpy(keyword, (next+1), (dimension>d_mysql_local_keyword_size)?d_mysql_local_keyword_size:dimension);
		if ((dimension = (next-pointer)) > 0) {
			*computed_size += dimension;
			if ((lower = (dimension>remaining)?remaining:dimension)) {
				remaining -= lower;
				strncat(sanitized_query, pointer, lower);
			}
		}
		if ((f_string_strlen(keyword) > 0) && (environment)) {
			for (index = 0; environment[index].link; ++index)
				if (f_string_strcmp(environment[index].link, keyword) == 0) {
					switch (environment[index].format) {
						case e_mysql_local_format_int:
							snprintf(entry, d_mysql_local_entry_size, d_mysql_local_replace_INT,
									*((int *)environment[index].variable));
							break;
						case e_mysql_local_format_float:
							snprintf(entry, d_mysql_local_entry_size, d_mysql_local_replace_FLOAT,
									*((float *)environment[index].variable));
							break;
						case e_mysql_local_format_char:
							if (*((char *)environment[index].variable) == d_mysql_local_string_invalid)
								*((char *)environment[index].variable) = d_mysql_local_string_invalid_replace_character;
							snprintf(entry, d_mysql_local_entry_size, d_mysql_local_replace_CHAR,
									*((char *)environment[index].variable));
							break;
						case e_mysql_local_format_string:
							while ((invalid_character = strchr((char *)environment[index].variable, d_mysql_local_string_invalid)))
								*invalid_character = d_mysql_local_string_invalid_replace_character;
							snprintf(entry, d_mysql_local_entry_size, d_mysql_local_replace_STRING,
									(char *)environment[index].variable);
							break;
					}
					if ((dimension = f_string_strlen(entry)) > 0) {
						*computed_size += dimension;
						if ((lower = (dimension>remaining)?remaining:dimension)) {
							remaining -= lower;
							strncat(sanitized_query, entry, lower);
						}
					}
					break;
				}
			if (!environment[index].link)
				d_err(e_log_level_high, "warning, on query:\n\n%s\n\nkeyword #%s was not defined in the current context", raw_query, keyword);
		}
		pointer = last;
	}
	if ((dimension = f_string_strlen(pointer)) > 0) {
		*computed_size += dimension;
		if ((lower = (dimension>remaining)?remaining:dimension)) {
			remaining -= lower;
			strncat(sanitized_query, pointer, lower);
		}
	}
	return sanitized_query;
}

int f_mysql_local_append(char *query, struct s_mysql_local_variable *environment, struct s_list *queries) {
	struct s_mysql_query *node;
	char buffer[d_mysql_local_query_size];
	size_t current_length = d_mysql_local_query_size, length;
	int result = d_false;
	if ((node = (struct s_mysql_query *) d_malloc(sizeof(struct s_mysql_query)))) {
		f_mysql_local_sanitize(query, buffer, &length, current_length, environment);
		if ((length > 0) && (node->query = (char *)d_malloc(length))) {
			current_length = length;
			f_mysql_local_sanitize(query, node->query, &length, current_length, environment);
			f_list_append(queries, (struct s_list_node *)node, e_list_insert_tail);
			result = d_true;
		} else
			d_free(node);
	}
	return result;
}

int f_mysql_local_append_file(const char *file, struct s_mysql_local_variable *environment, struct s_list *queries) {
	FILE *stream;
	char *query;
	size_t size, real_dimension;
	int result = d_false;
	if ((stream = fopen(file, "r"))) {
		fseek(stream, 0, SEEK_END);
		if ((size = ftell(stream)) > 0) {
			fseek(stream, 0, SEEK_SET);
			if ((query = (char *) d_malloc(size+1))) {
				if ((real_dimension = fread(query, 1, size, stream)) == size)
					result = f_mysql_local_append(query, environment, queries);
				d_free(query);
			}
		}
		fclose(stream);
	} else
		d_err(e_log_level_high, "warning, query file %s not found", file);
	return result;
}

int p_mysql_local_run_single(char *query, t_mysql_local_recall action) {
	MYSQL_RES *output;
	MYSQL_ROW output_row;
	int fields, result = d_false;
	if (v_mysql_link) {
		if (mysql_query(v_mysql_link, query) == 0) {
			if ((action) && (output = mysql_store_result(v_mysql_link))) {
				fields = mysql_num_fields(output);
				while ((output_row = mysql_fetch_row(output)))
					if (!action(output_row, fields))
						break;
				mysql_free_result(output);
			}
			result = d_true;
		} else
			d_err(e_log_level_high, "warning, query:\n%s\n\nreturns: %s", query, mysql_error(v_mysql_link));
	}
	return result;
}

int f_mysql_local_run(struct s_list *queries, t_mysql_local_recall action, int output) {
	struct s_mysql_query *node = (struct s_mysql_query *)(queries->head);
	size_t executed = 0, failed = 0;
	char buffer[d_string_buffer_size];
	int result = 0;
	while (node) {
		if (p_mysql_local_run_single(node->query, action))
			executed++;
		else
			failed++;
		if (output != d_mysql_local_stream_null) {
			snprintf(buffer, d_string_buffer_size, "\33[2K\r[running MYSQL][query %zu (+ %zu) of %zu - %.02f%%]", executed, failed, queries->fill,
					(((executed+failed)/(float)queries->fill)*100.0f));
			write(output, buffer, f_string_strlen(buffer));
			fsync(output);
		}
		node = (struct s_mysql_query *)(node->head.next);
	}
	if (output != d_mysql_local_stream_null) {
		write(output, "\n", 1);
		fsync(output);
	}
	return result;
}

void f_mysql_local_destroy_list(struct s_list *queries) {
	struct s_mysql_query *node;
       	while ((node = (struct s_mysql_query *)queries->head)) {
		f_list_delete(queries, (struct s_list_node *)node);
		d_free(node->query);
		d_free(node);
	}
}

void f_mysql_local_destroy(void) {
	if (v_mysql_link)
		mysql_close(v_mysql_link);
}


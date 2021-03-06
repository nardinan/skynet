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
int f_analyze_exclude(const char *file) {
	char buffer[PATH_MAX];
	FILE *stream;
	int result = d_true;
	if ((stream = fopen(d_analyzer_database, "r"))) {
		while (!feof(stream))
			if ((fgets(buffer, PATH_MAX, stream))) {
				f_string_trim(buffer);
				if (f_string_strcmp(buffer, file) == 0) {
					result = d_false;
					break;
				}
			}
		fclose(stream);
	}
	if (result)
		if ((stream = fopen(d_analyzer_database, "a"))) {
			fprintf(stream, "%s\n", file);
			fclose(stream);
		}
	return result;
}

int f_analyze_compare_extension(const char *file, const char *extension) {
	int result = d_false;
	char *pointer;
	if ((pointer = strrchr(file, '.')))
		if (strcmp(pointer, extension) == 0)
			result = d_true;
	return result;
}

int p_analyze_directory_file(const char *file, struct s_analyzer_action *actions) {
	int index, result = d_true;
	if (actions)
		if (f_analyze_exclude(file)) {
			for (index = 0; actions[index].extension; ++index)
				if (f_analyze_compare_extension(file, actions[index].extension)) {
					result = actions[index].action(file);
					break;
				}
		}
	return result;
}

int p_analyze_directory(char *directory, struct s_analyzer_action *actions, const char *directory_ignore_list) {
	DIR *stream;
	struct dirent *descriptor;
	char next_directory[PATH_MAX];
	int result = d_true;
	size_t length;
	if ((stream = opendir(directory))) {
		while ((descriptor = readdir(stream)))
			if ((descriptor->d_name[0] != '.') && (!(strstr(directory_ignore_list, descriptor->d_name)))) {
				length = f_string_strlen(directory);
				if (directory[length-1] == '/')
					directory[length-1] = '\0';
				snprintf(next_directory, PATH_MAX, "%s/%s", directory, descriptor->d_name);
				if (!(result = p_analyze_directory(next_directory, actions, directory_ignore_list)))
					break;
			}
		closedir(stream);
	} else
		result = p_analyze_directory_file(directory, actions);
	return result;
}

int f_analyze_directory(char *directory, struct s_analyzer_action *actions, const char *directory_ignore_list) {
	struct s_mysql_local_parameters db_parameters = {
		NULL, /* socket connection */
		"root",
		"digitare",
		"dampefm"
	};
	int index, load_result, result = d_false;
	if ((result = p_analyze_directory(directory, actions, directory_ignore_list))) {
		if (f_mysql_local_init(&db_parameters)) {
			for (index = 0; actions[index].extension; ++index)
				if (actions[index].load)
					if (!(load_result = actions[index].load()))
						d_err(e_log_level_low, "unable to run '.%s' LOAD function which returns code %d", actions[index].extension,
								load_result);
			f_mysql_local_destroy();
		} else
			d_err(e_log_level_ever, "mysql deamon '%s' is unreachable", db_parameters.server);
	}
	for (index = 0; actions[index].extension; ++index)
		if (actions[index].destroy)
			actions[index].destroy();
	return result;
}

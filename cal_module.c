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
#include "cal_module.h"
struct s_list *v_cal_module_entries = NULL;
int p_cal_module_analyze_add(struct s_cal_module_data *entry) {
	struct s_cal_module_data *list_entry;
	int result = d_false;
	if (!v_cal_module_entries)
		f_list_init(&v_cal_module_entries);
	if ((list_entry = (struct s_cal_module_data *) d_malloc(sizeof(struct s_cal_module_data)))) {
		memcpy(list_entry, entry, sizeof(struct s_cal_module_data));
		f_list_append(v_cal_module_entries, (struct s_list_node *)list_entry, e_list_insert_head);
	} else
		d_die(d_error_malloc);
	return result;
}

int p_cal_module_analyze_row(char *buffer, struct s_cal_module_value *entries, size_t size) {
	char *pointer = buffer, *next;
	int index, filled;
	for (index = 0; index < size; ++index) {
		filled = d_false;
		if ((next = strchr(pointer, ','))) {
			*next = '\0';
			f_string_trim(pointer);
			if (f_string_strlen(pointer) > 0) {
				switch (entries[index].format) {
					case e_cal_module_format_int:
						*(entries[index].value.int_ptr) = atoi(pointer);
						break;
					case e_cal_module_format_float:
						*(entries[index].value.float_ptr) = atof(pointer);
						break;
				}
				filled = d_true;
			}
			pointer = (next+1);
		}
		if ((!filled) && (entries[index].mandatory))
			break;
	}
	return (index==size);
}

int f_cal_module_analyze(const char *file) {
	FILE *stream;
	char buffer[d_string_buffer_size], *next, *pointer;
	struct tm event_time;
	struct s_cal_module_data entry = {0};
	struct s_string_key_format dictionary[] = {
		{e_string_key_kind_string, 	"name",			{(void *)entry.name},			d_string_buffer_size},
		{e_string_key_kind_string, 	"starting_time",	{(void *)entry.date},			d_cal_module_date_size},
		{e_string_key_kind_string, 	"location",		{(void *)entry.location},		d_cal_module_location_size},
		{e_string_key_kind_string,	"temp_SN",		{(void *)(entry.serials[0])},		d_cal_module_serial_size},
		{e_string_key_kind_string,	"temp_SN",		{(void *)(entry.serials[1])},		d_cal_module_serial_size},
		{e_string_key_kind_float,	"bias_volt",		{(void *)&(entry.bias_voltage)}},
		{e_string_key_kind_float,	"leak_curr",		{(void *)&(entry.leakage_current)}},
		{e_string_key_kind_float,	"temp_left",		{(void *)&(entry.temperatures[0])}},
		{e_string_key_kind_float,	"temp_right",		{(void *)&(entry.temperatures[1])}}
	};
	float pedestal, sigma_raw, sigma, slope;
	int strip_index, va_index, strip_va_index, bad_channel, current_index = 0, result = d_true;
	struct s_cal_module_value entries[] = {
		{"channel", 	{.int_ptr = &strip_index}, 	e_cal_module_format_int, 	d_true},
		{"va",		{.int_ptr = &va_index},		e_cal_module_format_int, 	d_true},
		{"va_channel",	{.int_ptr = &strip_va_index},	e_cal_module_format_int, 	d_true},
		{"pedestal",	{.float_ptr = &pedestal},	e_cal_module_format_float,	d_true},
		{"sigma_raw",	{.float_ptr = &sigma_raw},	e_cal_module_format_float,	d_true},
		{"sigma",	{.float_ptr = &sigma},		e_cal_module_format_float,	d_true},
		{"bad_channel",	{.int_ptr = &bad_channel},	e_cal_module_format_int,	d_true},
		{"slope",	{.float_ptr = &slope},		e_cal_module_format_float,	d_false}
	};
	size_t dictionary_elements = ((sizeof(dictionary)/sizeof(s_string_key_format))),
	       entries_elements = ((sizeof(entries)/sizeof(s_cal_module_value)));
	if ((stream = fopen(file, "r"))) {
		strncpy(entry.cal_file, file, PATH_MAX);
		snprintf(entry.pdf_file, PATH_MAX, "%s.%s", file, d_cal_module_pdf_extension);
		while (!feof(stream))
			if (fgets(buffer, d_string_buffer_size, stream)) {
				f_string_trim(buffer);
				if (f_string_strlen(buffer) > 0)
					if ((f_string_key(buffer, dictionary, dictionary_elements, '=')) == 0)
						if (p_cal_module_analyze_row(buffer, entries, entries_elements)) {
							if (strip_index == current_index) {
								entry.rows[current_index].pedestal = pedestal;
								entry.rows[current_index].sigma_raw = sigma_raw;
								entry.rows[current_index].sigma = sigma;
								entry.rows[current_index].bad_channel = bad_channel;
							} else
								d_log(e_log_level_low, "warning, channel %d missing @ %s", current_index, file);
							current_index++;
						}
			}
		fclose(stream);
		if (dictionary[0].assigned) {/* at least, name has to be assigned */
			pointer = (char *)file;
			while ((next = strstr(pointer, entry.name)))
				pointer = (next+f_string_strlen(entry.name));
			if (pointer)
				if (*(pointer) == '_') {
					entry.location_code = *(++pointer);
					entry.location_room = *(++pointer);
					if (*(++pointer) == '_')
						if (!isdigit(*(++pointer)))
							entry.test_kind = *(pointer);
					if (strptime(entry.date, d_cal_module_date_format, &event_time))
						entry.timestamp = mktime(&event_time);
					p_cal_module_analyze_add(&entry);
				}
		}
	}
	return result;
}

void p_cal_module_load_check_device (char location_code, char device_kind, unsigned int device_code, char *device_type, char connector_side) {
	char query[d_mysql_local_query_size];
	snprintf(query, d_mysql_local_query_size, "INSERT INTO t_device(location_fk, kind, code, type, connector) SELECT * FROM (SELECT "
			"(SELECT location_pk FROM t_location WHERE code = '%c') AS tmp_location_fk, "
			"'%c' AS tmp_kind, "
			"%03d AS tmp_code, "
			"'%s' AS tmp_type, "
			"'%c' AS tmp_connector) AS temporary WHERE NOT EXISTS "
			"(SELECT kind, code, type, connector FROM t_device WHERE (kind = '%c') AND (code = %03d) AND (type = '%s')) LIMIT 1;",
			location_code, device_kind, device_code, device_type, connector_side, device_kind, device_code, device_type);
	if (f_mysql_local_run(query, NULL))
		d_log(e_log_level_high, "running \"%s\"", query);
}

int f_cal_module_load (void) {
	struct s_cal_module_data *current;
	char location_code, device_kind, device_type[d_cal_module_device_type_size], device_code[d_cal_module_device_code_size] = {0}, connector_side;
	int index, result = d_true;
	if ((v_cal_module_entries) && (current = (struct s_cal_module_data *)(v_cal_module_entries->head)))
		while (current) {
			index = 0;
			device_kind = current->name[index++];
			if (device_kind == 'H') {
				location_code = d_cal_module_device_default_location;
				if (current->name[index] == 'F') {
					device_type[0] = 'F';
					device_type[1] = 'M';
					index++;
				} else
					strncpy(device_type, d_cal_module_device_default_type, d_cal_module_device_type_size);
			} else {
				location_code = current->name[index++];
				device_type[0] = current->name[index++];
				device_type[1] = current->name[index++];
			}
			device_code[0] = current->name[index++];
			device_code[1] = current->name[index++];
			device_code[2] = current->name[index++];
			connector_side = current->name[index++];
			p_cal_module_load_check_device(location_code, device_kind, atoi(device_code), device_type, connector_side);
			current = (struct s_cal_module_data *)(current->head.next);
		}
	return result;
}

void f_cal_module_destroy (void) {
	struct s_cal_module_data *current;
	if (v_cal_module_entries) {
		while (v_cal_module_entries->head)
			if ((current = (struct s_cal_module_data *)f_list_delete(v_cal_module_entries, v_cal_module_entries->head)))
				d_free(current);
		f_list_destroy(&v_cal_module_entries);
	}
}

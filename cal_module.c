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
		{e_string_key_kind_string,	"temp_SN",		{(void *)(entry.serials[0])},		d_cal_module_serial_size, 	d_true},
		{e_string_key_kind_string,	"temp_SN",		{(void *)(entry.serials[1])},		d_cal_module_serial_size, 	d_true},
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

int f_cal_module_load (void) {
	struct s_cal_module_data *current;
	char location_code, device_kind, device_type[d_cal_module_device_type_size] = {0}, buffer_device_code[d_cal_module_device_code_size] = {0},
	     connector_side, test_date[d_string_buffer_size], test_kind;
	int index, device_code, channel, serial, result = d_true;
	struct s_mysql_local_variable environment[] = {
		{"device_location_code",	&location_code,		e_mysql_local_format_char},
		{"device_kind",			&device_kind, 		e_mysql_local_format_char},
		{"device_code", 		&device_code, 		e_mysql_local_format_int},
		{"device_type", 		device_type, 		e_mysql_local_format_string},
		{"device_connector",		&connector_side, 	e_mysql_local_format_char},
		{"test_location_code",		NULL,			e_mysql_local_format_char}, 	/* 5 */
		{"test_date",			test_date,		e_mysql_local_format_string},	/* 6 */
		{"test_room_code",		NULL,			e_mysql_local_format_char},	/* 7 */
		{"test_temperature_1",		NULL,			e_mysql_local_format_float},	/* 8 */
		{"test_temperature_2",		NULL,			e_mysql_local_format_float},	/* 9 */
		{"test_voltage",		NULL,			e_mysql_local_format_float},	/* 10 */
		{"test_current",		NULL,			e_mysql_local_format_float},	/* 11 */
		{"test_cal_file",		NULL,			e_mysql_local_format_string},	/* 12 */
		{"test_pdf_file",		NULL,			e_mysql_local_format_string},	/* 13 */
		{"test_kind",			&test_kind,		e_mysql_local_format_char},	/* 14 */
		{"test_channel",		&channel,		e_mysql_local_format_int},	/* 15 */
		{"test_pedestal",		NULL,			e_mysql_local_format_float},	/* 16 */
		{"test_sigma_raw",		NULL,			e_mysql_local_format_float},	/* 17 */
		{"test_sigma",			NULL,			e_mysql_local_format_float},	/* 18 */
		{"test_bad_channel",		NULL,			e_mysql_local_format_int},	/* 19 */
		{"test_serial",			NULL,			e_mysql_local_format_string}, 	/* 20 */
		{NULL}
	};
	struct s_list *queries_insertion = NULL, *queries_association = NULL;
	if ((v_cal_module_entries) && (current = (struct s_cal_module_data *)(v_cal_module_entries->head))) {
		f_list_init(&queries_insertion);
		f_list_init(&queries_association);
		while (current) {
			environment[5].variable = &(current->location_code);
			environment[7].variable = &(current->location_room);
			environment[8].variable = &(current->temperatures[0]);
			environment[9].variable = &(current->temperatures[1]);
			environment[10].variable = &(current->bias_voltage);
			environment[11].variable = &(current->leakage_current);
			environment[12].variable = current->cal_file;
			environment[13].variable = current->pdf_file;
			strftime(test_date, d_string_buffer_size, d_mysql_local_date_format, localtime(&(current->timestamp)));
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
			buffer_device_code[0] = current->name[index++];
			buffer_device_code[1] = current->name[index++];
			buffer_device_code[2] = current->name[index++];
			device_code = atoi(buffer_device_code);
			connector_side = current->name[index++];
			if ((test_kind = current->test_kind) == 0x00)
				test_kind = d_cal_module_device_test_default_kind;
			f_mysql_local_append_file("queries/TFH_insert.sql", environment, queries_insertion);
			f_mysql_local_append_file("queries/device_insert.sql", environment, queries_insertion);
			f_mysql_local_append_file("queries/device_test_insert.sql", environment, queries_insertion);
			for (channel = 0; channel < d_cal_module_ladder_channels; ++channel) {
				environment[16].variable = &(current->rows[channel].pedestal);
				environment[17].variable = &(current->rows[channel].sigma_raw);
				environment[18].variable = &(current->rows[channel].sigma);
				environment[19].variable = &(current->rows[channel].bad_channel);
				f_mysql_local_append_file("queries/device_measurement_insert.sql", environment, queries_insertion);
			}
			for (serial = 0; serial < d_cal_module_ladder_serials; ++serial)
				if (f_string_strlen(current->serials[serial]) > 0) {
					environment[20].variable = current->serials[serial];
					f_mysql_local_append_file("queries/serial_insert.sql", environment, queries_insertion);
					f_mysql_local_append_file("queries/TFH_device_insert.sql", environment, queries_association);
				}
			f_mysql_local_append_file("queries/TFH_position_insert.sql", environment, queries_insertion);
			current = (struct s_cal_module_data *)(current->head.next);
		}
		f_mysql_local_run(queries_insertion, NULL);
		f_mysql_local_run(queries_association, NULL);
		f_mysql_local_destroy_list(queries_insertion);
		f_mysql_local_destroy_list(queries_association);
		f_list_destroy(&queries_insertion);
		f_list_destroy(&queries_association);
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

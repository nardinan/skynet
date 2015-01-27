/*
 * Skynet
 * Copyright (C) 2014 Andrea Nardinocchi (andrea@nardinan.it)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <time.h>
#include <math.h>
#define d_string_buffer_size 512
#define d_string_serial_size 20
#define d_string_date_size 45
#define d_string_location_size 20
#define d_devices 512
#define d_device_entries 2048
#define d_device_sensors 2
#define d_device_channels 384
#define d_device_column 8
#define d_true 1
#define d_false 0
typedef struct s_key_target {
	const char *key;
	void *destination;
	char kind;
} s_key_target;
typedef struct s_ladder_garbage {
	char name[d_string_buffer_size];
	int confirmed;
} s_ladder_garbage;
typedef struct s_ladder_test {
	char cal_file[d_string_buffer_size], pdf_file[d_string_buffer_size], name[d_string_buffer_size], date[d_string_date_size],
	     location[d_string_location_size];
	float bias_voltage, leakage_current, temperature_left, temperature_right, pedestal, pedestal_rms, sigma_raw, sigma_raw_rms, sigma, sigma_rms;
	time_t date_timestamp;
} s_ladder_test;
typedef struct s_ladder {
	char last_name[d_string_buffer_size], serials[d_device_sensors][d_string_serial_size];
	struct s_ladder_test history[d_device_entries];
	int initialized, history_index, last_event;
} s_ladder;
struct s_ladder database[d_devices];
struct s_ladder_garbage garbage[d_devices];
int last_entry, last_discarded;
const char *ignore_list[] = {
	"H000T",
	"H000B",
	NULL
};
#define dD(i) database[i]
#define dE(i) history[i]
#define dS(i) serials[i]
void f_clear_garbage(const char *name) {
	int index;
	struct s_ladder_garbage backup;
	for (index = 0; index < last_discarded; index++)
		if (strcmp(name, garbage[index].name) == 0) {
			if (index < (last_discarded-1)) {
				memcpy(&backup, &garbage[index], sizeof(struct s_ladder_garbage));
				memcpy(&garbage[index], &garbage[last_discarded-1], sizeof(struct s_ladder_garbage));
				memcpy(&garbage[last_discarded-1], &backup, sizeof(struct s_ladder_garbage));
			}
			last_discarded--;
			break;
		}
}

void f_analyze_database(void) {
	int index, index_event, last_event;
	struct tm event_time;
	time_t last_event_time, current_event_time;
	for (index = 0; index < last_entry; index++) {
		for (index_event = 0, last_event = -1, last_event_time = 0; index_event < database[index].history_index; index_event++) {
			/* this is a slow operation, but we have lot of spare time! */
			f_clear_garbage(database[index].history[index_event].name);
			if (strptime(database[index].history[index_event].date, "%d %b %Y %H:%M:%S", &event_time)) {
				database[index].history[index_event].date_timestamp = mktime(&event_time);
				current_event_time = database[index].history[index_event].date_timestamp;
				if (current_event_time > last_event_time) {
					last_event_time = current_event_time;
					last_event = index_event;
				}
			} else
				fprintf(stderr, "warning, %s is not a compatible date format", database[index].history[index_event].date);
		}
		database[index].last_event = last_event;
		if (last_event >= 0)
			strcpy(database[index].last_name, database[index].history[last_event].name);
		/* just to check our working directory */
		printf("%s -> last calibration %s @ %s (%s) <ped> %.02f(+/- %.02f); <sig_raw> %.02f(+/- %.02f); <sig> %.02f(+/- %.02f)\n",
				dD(index).last_name, dD(index).dE(last_event).date, dD(index).dE(last_event).location, dD(index).dE(last_event).cal_file,
				dD(index).dE(last_event).pedestal, dD(index).dE(last_event).pedestal_rms, dD(index).dE(last_event).sigma_raw,
				dD(index).dE(last_event).sigma_raw_rms, dD(index).dE(last_event).sigma, dD(index).dE(last_event).sigma_rms);
	}
}

int f_check_serials(char serials_l[][d_string_serial_size], char serials_r[][d_string_serial_size]) {
	int index_l, index_r, result = d_true;
	for (index_l = 0; index_l < d_device_sensors; index_l++) {
		for (index_r = 0, result = d_false; index_r < d_device_sensors; index_r++)
			if (strcmp(serials_l[index_l], serials_r[index_r]) == 0) {
				result = d_true;
				break;
			}
		if (!result)
			break;
	}
	return result;
}

int f_analyze_event(char serials[][d_string_serial_size], struct s_ladder_test entry) {
	int index, serial, result = d_false;
	for (index = 0; index < last_entry; index++)
		if (f_check_serials(serials, database[index].serials)) {
			if (database[index].history_index < (d_device_entries-1)) {
				memcpy(&database[index].history[database[index].history_index], &entry, sizeof(struct s_ladder_test));
				database[index].history_index++;
			} else
				fprintf(stderr, "warning, d_device_entries is too small for device %s (d_device_entries=%d)\n", entry.name, d_device_entries);
			result = d_true;
		}
	if (!result) {
		for (serial = 0; serial < d_device_sensors; serial++)
			strcpy(database[last_entry].serials[serial], serials[serial]);
		memcpy(&database[last_entry].history[database[index].history_index], &entry, sizeof(struct s_ladder_test));
		database[index].history_index++;
		last_entry++;
	}
	return result;
}

int f_check_extension(const char *file, const char *extension) {
	int result = d_false;
	char *last_pointer = (char *)file, *pointer;
	while ((pointer = strchr(last_pointer, '.')))
		last_pointer = (pointer+1);
	if (last_pointer)
		if (strcmp(last_pointer, extension) == 0)
			result = d_true;
	return result;
}
#define d_space_character(a) (((a)==' ')||((a)=='\t'))
#define d_final_character(a) (((a)=='\0')||((a)=='\n'))
char *f_string_trim(char *string) {
	size_t length = strlen(string);
	char *begin = string, *final = (string+length)-1;
	while ((d_space_character(*begin) || (d_space_character(*final)) || (d_final_character(*final))) && (final >= begin)) {
		if (d_space_character(*begin))
			begin++;
		if ((d_space_character(*final)) || (d_final_character(*final))) {
			*final = '\0';
			final--;
		}
	}
	if (begin > string)
		memmove(string, begin, strlen(begin)+1);
	return string;
}

int p_analyze_row(char *string, float *pedestal, float *sigma_raw, float *sigma) {
	char *begin_pointer = string, *end_pointer;
	float content[d_device_column];
	int index = 0;
	while ((index < d_device_column) && (end_pointer = strchr(begin_pointer, ','))) {
		*end_pointer = '\0';
		content[index++] = atof(begin_pointer);
		begin_pointer = (end_pointer+1);
	}
	*pedestal = content[3];
	*sigma_raw = content[4];
	*sigma = content[5];
	return (int)content[0];
}

float f_babylonian_sqrt(float value, float precision) {
	float result = value, level = 1.0;
	while ((result-level) > precision) {
		result = (result+level)/2.0;
		level = value/result;
	}
	return result;
}

void p_analyze_row_values(float *values, size_t dimension, float *mean, float *rms) {
	float local_mean = 0, local_square = 0, fraction = 1/(float)dimension;
	int index;
	for (index = 0; index < dimension; index++) {
		local_mean += values[index];
		local_square += (values[index]*values[index]);
	}
	local_mean *= fraction;
	local_square *= fraction;
	*mean = local_mean;
	*rms = f_babylonian_sqrt(fabs(local_square-(local_mean*local_mean)), 0.01f);
}
#define d_check_key(_k,_v,_n,act)\
	do{\
		if(strcmp((_k),(_n).key)==0){\
			switch((_n).kind){\
				case 'S':strcpy((char *)((_n).destination),(_v));break;\
				case 'F':*((float *)((_n).destination))=atof(_v);break;\
				default:*((int *)((_n).destination))=atoi(_v);\
			}\
			act;\
		}\
	}while(0)
int f_analyze_file(const char *file) {
	FILE *stream;
	struct s_ladder_test entry = {0};
	struct s_key_target dictionary[] = {
		{"name", 		(void *)entry.name, 			'S'},
		{"starting_time", 	(void *)entry.date, 			'S'},
		{"location", 		(void *)entry.location, 		'S'},
		{"bias_volt", 		(void *)&(entry.bias_voltage), 		'F'},
		{"leak_curr", 		(void *)&(entry.leakage_current),	'F'},
		{"temp_left",		(void *)&(entry.temperature_left),	'F'},
		{"temp_right",		(void *)&(entry.temperature_right),	'F'},
		{NULL, 			NULL, 					'S'}
	};
	char line_buffer[d_string_buffer_size], *key_pointer, *value_pointer, serials[d_device_sensors][d_string_serial_size];
	int result = d_false, serial_index = 0, key_index, garbage_index, strip_index = 0, readed_strip_index;
	float pedestal[d_device_channels] = {0.0}, sigma_raw[d_device_channels] = {0.0}, sigma[d_device_channels] = {0.0};
	if (f_check_extension(file, "cal"))
		if ((stream = fopen(file, "r"))) {
			strcpy(entry.cal_file, file);
			snprintf(entry.pdf_file, d_string_buffer_size, "%s.pdf", file);
			while (!feof(stream))
				if (fgets(line_buffer, d_string_buffer_size, stream)) {
					f_string_trim(line_buffer);
					key_pointer = line_buffer;
					if ((value_pointer = strchr(key_pointer, '='))) {
						*value_pointer = '\0';
						value_pointer++;
						f_string_trim(key_pointer);
						f_string_trim(value_pointer);
						if (strlen(value_pointer) > 0) {
							if (strcmp(key_pointer, "temp_SN") == 0)
								strcpy(serials[serial_index++], value_pointer);
							else {
								for (key_index = 0; dictionary[key_index].key; key_index++)
									d_check_key(key_pointer, value_pointer, dictionary[key_index], break);
							}
						}
					} else if (strip_index < d_device_channels) {
						readed_strip_index = p_analyze_row(line_buffer, &(pedestal[strip_index]), &(sigma_raw[strip_index]),
								&(sigma[strip_index]));
						if (strip_index != readed_strip_index) {
							fprintf(stderr, "wrong channel index (%d != %d), aligning ...\n", strip_index, readed_strip_index);
							strip_index = readed_strip_index;
						}
						strip_index++;
					}
				}
			fclose(stream);
			if (serial_index == d_device_sensors) {
				p_analyze_row_values(pedestal, d_device_channels, &(entry.pedestal), &(entry.pedestal_rms));
				p_analyze_row_values(sigma_raw, d_device_channels, &(entry.sigma_raw), &(entry.sigma_raw_rms));
				p_analyze_row_values(sigma, d_device_channels, &(entry.sigma), &(entry.sigma_rms));
				f_analyze_event(serials, entry);
				result = d_true;
			} else {
				for (garbage_index = 0; garbage_index < last_discarded; garbage_index++)
					if (strcmp(garbage[garbage_index].name, entry.name) == 0)
						break;
				if (garbage_index >=last_discarded) {
					strcpy(garbage[last_discarded].name, entry.name);
					garbage[last_discarded].confirmed = d_true;
					last_discarded++;
				}
			}
		}
	return result;
}

int f_check_directory(const char *directory) {
	int index, result = d_true;
	for (index = 0; ignore_list[index]; index++)
		if (strcmp(ignore_list[index], directory) == 0) {
			result = d_false;
			break;
		}
	return result;
}

int f_analyze_directory(const char *directory) {
	DIR *stream;
	struct dirent *descriptor;
	int result = d_false;
	char next_directory[d_string_buffer_size];
	if ((stream = opendir(directory))) {
		while ((descriptor = readdir(stream)))
			if (descriptor->d_name[0] != '.')
				if (f_check_directory(descriptor->d_name)) {
					snprintf(next_directory, d_string_buffer_size, "%s/%s", directory, descriptor->d_name);
					f_analyze_directory(next_directory);
				}
		closedir(stream);
		result = d_true;
	} else
		result = f_analyze_file(directory);
	return result;
}

int main(int argc, char *argv[]) {
	int index;
	if (argc > 1) {
		f_analyze_directory(argv[1]);
		f_analyze_database();
		if (last_discarded > 0) {
			fprintf(stderr, "[WARNING] missing sensors serial numbers:");
			for (index = 0; index < last_discarded; index++)
				fprintf(stderr, "%s%s", ((!index)?" ":((index==last_discarded-1)?" and ":", ")), garbage[index].name);
			fputs("\n", stderr);
		}
	} else
		fprintf(stderr, "use %s <path>\n", argv[0]);
	return 0;
}


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
#ifndef skynet_cal_module_h
#define skynet_cal_module_h
#include <miranda/ground.h>
#include <limits.h>
#define d_cal_module_ladder_channels 384
#define d_cal_module_ladder_serials 2
#define d_cal_module_ladder_columns 8
#define d_cal_module_ladder_channel_null -1
#define d_cal_module_serial_size 20
#define d_cal_module_date_size 45
#define d_cal_module_location_size 20
#define d_cal_module_pdf_extension "pdf"
#define d_cal_module_date_format "%d %b %Y %H:%M:%S"
typedef enum e_cal_module_formats {
	e_cal_module_format_int,
	e_cal_module_format_float
} e_cal_module_formats;
typedef struct s_cal_module_value {
	const char *description;
	union {
		int *int_ptr;
		float *float_ptr;
	} value;
	enum e_cal_module_formats format;
	int mandatory;
} s_cal_module_value;
typedef struct s_cal_module_data_row {
	float pedestal, sigma_raw, sigma;
	int bad_channel;
} s_cal_module_data_row;
typedef struct s_cal_module_data { d_list_node_head;
	char serials[d_cal_module_ladder_serials][d_cal_module_serial_size], name[d_string_buffer_size], date[d_cal_module_date_size],
	     location[d_cal_module_location_size], cal_file[PATH_MAX], pdf_file[PATH_MAX], location_room, location_code, test_kind;
	float bias_voltage, leakage_current, temperatures[d_cal_module_ladder_serials];
	struct s_cal_module_data_row rows[d_cal_module_ladder_channels];
	time_t timestamp;
} s_cal_module_data;
extern struct s_list *v_cal_module_entries;
extern int p_cal_module_analyze_add(struct s_cal_module_data *entry);
extern int p_cal_module_analyze_row(char *buffer, struct s_cal_module_value *entries, size_t size);
extern int f_cal_module_analyze(const char *file);
extern int f_cal_module_load(void);
extern void f_cal_module_destroy(void);
#endif

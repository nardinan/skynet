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
#include "cal_module.h"
struct s_analyzer_action actions[] = {
	{".cal", &f_cal_module_analyze, &f_cal_module_load, &f_cal_module_destroy},
	{NULL}
};
int main (int argc, char *argv[]) {
	v_log_level = e_log_level_ever;
	if (argc == 2) {
		f_memory_init();
		f_analyze_directory(argv[1], actions, "H000T H000B HF000B HF000T");
		f_memory_destroy();
	}
	return 0;
}

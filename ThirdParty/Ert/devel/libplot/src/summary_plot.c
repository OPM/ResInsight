/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary_plot.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#include <plot.h>
#include <plot_dataset.h>
#include <plot_util.h>
#include <plot_summary.h>
#include <ecl_kw.h>
#include <ecl_sum.h>

int main(int argc, const char **argv)
{
    plot_type *item;
    plot_dataset_type *d;
    PLFLT *x, *y;
    double x_max, y_max;
    int N;
    const char *kw = argv[1];

    if (argc < 2) {
	fprintf(stderr, "Error: give keyword as argument!\n");
	exit(EXIT_FAILURE);
    }

    item = plot_alloc();
    plot_set_window_size(item, 1152, 768);
    plot_initialize(item, "png", "punqs3.png");

    {
	char str[PATH_MAX];
	int i, k;
	int len = 2;
	int interval[len][2];

	interval[0][0] = 1;
	interval[0][1] = 10;
	interval[1][0] = 50;
	interval[1][1] = 60;

	for (i = 0; i < len; i++) {
	    for (k = interval[i][0]; k <= interval[i][1]; k++) {
		snprintf(str, PATH_MAX,
			 "/h/masar/EnKF_PUNQS3/PUNQS3/Original/Realizations/PUNQS3_Realization_%d/PUNQS3_%d.DATA",
			 k, k);
		plot_summary_collect_data(&x, &y, &N, str, kw);
		d = plot_dataset_alloc(false);
		plot_dataset_set_data(d, x, y, N, BLUE, LINE);
		plot_dataset_add(item, d);
		util_safe_free(x);
		util_safe_free(y);

		snprintf(str, PATH_MAX,
			 "/d/proj/bg/enkf/EnKF_PUNQS3/enkf_runs/member_%03d/PUNQS3_%04d.DATA",
			 k, k);
		d = plot_dataset_alloc( false );
		plot_summary_collect_data(&x, &y, &N, str, kw);
		plot_dataset_set_data(d, x, y, N, RED, LINE);
		plot_dataset_add(item, d);
		util_safe_free(x);
		util_safe_free(y);
	    }
	}
    }

    d = plot_dataset_alloc();
    plot_summary_collect_data(&x, &y, &N,
			      "/d/proj/bg/enkf/EnKF_PUNQS3/PUNQS3/Original/PUNQS3.DATA",
			      kw);
    plot_dataset_set_data(d, x, y, N, BLACK, POINT);
    plot_dataset_add(item, d);

    util_safe_free(x);
    util_safe_free(y);

    plot_set_labels(item, "Days", kw, "PUNQS3", BLACK);
    plot_get_extrema(item, &x_max, &y_max, NULL, NULL);
    plot_set_viewport(item, 0, x_max, 0, y_max);
    plot_data(item);
    plot_free(item);
}

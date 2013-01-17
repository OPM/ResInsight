/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_summary.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

void
plot_summary_collect_data(PLFLT ** x, PLFLT ** y, int *size,
			  const char *data_file, const char *keyword)
{
    char *base;
    char *header_file;
    char **summary_file_list;
    char *path;
    int files;
    bool fmt_file;
    ecl_sum_type *ecl_sum;
    int report_step, first_report_step, last_report_step;
    PLFLT *x_tmp, *y_tmp;
    PLFLT diff_day;
    time_t t, t0;

    util_alloc_file_components(data_file, &path, &base, NULL);
    ecl_util_alloc_summary_files(path, base, &header_file,
				 &summary_file_list, &files, &fmt_file);
				 
    ecl_sum = ecl_sum_fread_alloc(header_file, files,
				  (const char **) summary_file_list, true,
				  true);
    ecl_sum_get_report_size(ecl_sum, &first_report_step,
			    &last_report_step);
    x_tmp = malloc(sizeof(PLFLT) * (files + 1));
    y_tmp = malloc(sizeof(PLFLT) * (files + 1));
    *size = files;

    for (report_step = first_report_step; report_step <= last_report_step;
	 report_step++) {
	if (ecl_sum_has_report_nr(ecl_sum, report_step)) {
	    int day, month, year;

	    util_set_date_values(ecl_sum_get_sim_time
				 (ecl_sum, report_step), &day, &month,
				 &year);
	    if (report_step == first_report_step)
		plot_util_get_time(day, month, year, &t0, NULL);
	    plot_util_get_time(day, month, year, &t, NULL);
	    plot_util_get_diff(&diff_day, t, t0);
	    x_tmp[report_step] = (PLFLT) diff_day;
	    y_tmp[report_step] = (PLFLT)
		ecl_sum_get_general_var(ecl_sum, report_step, keyword);
	}
    }
    *x = x_tmp;
    *y = y_tmp;
    util_safe_free(header_file);
    util_safe_free(base);
    util_safe_free(path);
    util_free_stringlist(summary_file_list, files);
    ecl_sum_free(ecl_sum);
}

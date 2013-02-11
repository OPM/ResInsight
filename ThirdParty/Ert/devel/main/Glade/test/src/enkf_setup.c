/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_setup.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include "enkf_setup.h"

#include "support.h"
#include "callbacks.h"

enkf_main_type *
enkf_setup_bootstrap(const char *enkf_config, GtkWidget * win)
{
    const char     *site_config =
	"/d/proj/bg/enkf/Config/statoil/site-config";
    enkf_main_type *enkf_main;
    const ecl_config_type *ecl_config;
    GtkWidget      *entry;
    model_config_type *model_config;
    ensemble_config_type *ensemble_config;


    enkf_main = enkf_main_bootstrap(site_config, enkf_config);
    ecl_config = enkf_main_get_ecl_config(enkf_main);
    model_config = enkf_main_get_model_config(enkf_main);
    ensemble_config = enkf_main_get_ensemble_config(enkf_main);

    {
	const ecl_grid_type *ecl_grid;
	ecl_grid = ecl_config_get_grid(ecl_config);
	entry = lookup_widget(GTK_WIDGET(win), "entry21");
	gtk_entry_set_text(GTK_ENTRY(entry), ecl_grid_get_name(ecl_grid));
    }

    {/*
	char           *path;
	char           *ext;
	char           *base;
	sched_file_type *sched_file =
	    ecl_config_get_sched_file(ecl_config);*/
	/*
	 * const stringlist_type * sched_list =
	 * config_get_stringlist_ref(ecl_config , "SCHEDULE_FILE"); const
	 * char * schedule_src = stringlist_iget( sched_list , 0);
	 * util_alloc_file_components(schedule_src , NULL, &base , &ext);
	 * printf("%s %s\n", base, ext); 
	 */
	entry = lookup_widget(GTK_WIDGET(win), "entry22");
	gtk_entry_set_text(GTK_ENTRY(entry), "Not implemented");
    }

    entry = lookup_widget(GTK_WIDGET(win), "entry23");
    gtk_entry_set_text(GTK_ENTRY(entry),
		       ecl_config_get_data_file(ecl_config));

    {
	const char     *init_file =
	    ecl_config_get_equil_init_file(ecl_config);
	entry = lookup_widget(GTK_WIDGET(win), "entry24");
	gtk_entry_set_text(GTK_ENTRY(entry), init_file);
    }

    {
	path_fmt_type  *runpath_fmt;

	runpath_fmt = model_config_get_runpath_fmt(model_config);
	entry = lookup_widget(GTK_WIDGET(win), "entry25");
	gtk_entry_set_text(GTK_ENTRY(entry),
			   path_fmt_get_fmt(runpath_fmt));

    }

    {
	path_fmt_type  *result_path_fmt;
        /*

	result_path_fmt = model_config_get_result_path_fmt(model_config);
	entry = lookup_widget(GTK_WIDGET(win), "entry29");
	gtk_entry_set_text(GTK_ENTRY(entry),
			   path_fmt_get_fmt(result_path_fmt));
                           */
    }

    {
	int             ens_size =
	    ensemble_config_get_size(ensemble_config);
	int             iens;
	int             k = 0;
	int            *arr;
	int             i;
	int             j = 0;
	GtkWidget      *treestore;
	GtkTreeIter     toplevel;
	char            str_to[8];
	char            str_from[8];


	arr = malloc(sizeof(int));

	for (iens = 0; iens < ens_size; iens++) {
	    enkf_state_type *enkf_state;
	    member_config_type *member_config;
	    keep_runpath_type keep_runpath;

	    enkf_state = enkf_main_iget_state(enkf_main, iens);
	    member_config = enkf_state_get_member_config(enkf_state);
	    keep_runpath = member_config_get_keep_runpath(member_config);

	    printf("ensemble nr %d: %p\n", iens, enkf_state);
	    printf("member config: %p\n", member_config);
	    printf("keep runtype: %d\n", keep_runpath);
	    if (keep_runpath == EXPLICIT_KEEP) {
		arr = util_realloc(arr, sizeof(int) * (k + 1), __func__);
		arr[k] = iens;
		k++;
	    }

	}

	treestore = lookup_widget(GTK_WIDGET(win), "treestore");

	/*
	 * Small algorithm to grab the intervals 
	 */
	for (i = 0; i < k; i++) {
	    if (j != i)
		continue;
	    for (j = (i + 1); j < k; j++) {
		if (arr[i] == (arr[j] - (j - i)))
		    continue;
		break;
	    }
	    snprintf(str_from, 8, "%d", arr[i]);
	    snprintf(str_to, 8, "%d", arr[j - 1]);

	    gtk_tree_store_append(GTK_TREE_STORE(treestore), &toplevel,
				  NULL);
	    gtk_tree_store_set(GTK_TREE_STORE(treestore), &toplevel,
			       COL_FROM, str_from, COL_TO, str_to, -1);
	    printf("interval was %d to %d\n", arr[i], arr[j - 1]);
	}

	util_safe_free(arr);
    }

    entry = lookup_widget(GTK_WIDGET(win), "entry28");
    gtk_entry_set_text(GTK_ENTRY(entry), "Not implemented");

    return enkf_main;
}

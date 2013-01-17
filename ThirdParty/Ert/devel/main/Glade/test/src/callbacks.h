/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'callbacks.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <enkf_tui_init.h>


gboolean
on_window1_delete_event (GtkWidget * widget,
			 GdkEvent * event, gpointer user_data);

gboolean
on_window1_destroy_event (GtkWidget * widget,
			  GdkEvent * event, gpointer user_data);

void on_new1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_open1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_save1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_save_as1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_quit1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_cut1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_copy1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_paste1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_delete1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_about1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_button2_filedialog_clicked (GtkButton * button, gpointer user_data);

void on_button3_clicked (GtkButton * button, gpointer user_data);

void on_button9_clicked (GtkButton * button, gpointer user_data);

void on_okbutton1_clicked (GtkButton * button, gpointer user_data);

void on_button19_clicked (GtkButton * button, gpointer user_data);

typedef enum _enkf_run_select_mode_type
{
  ONLY_PARENT,
  ONLY_CHILD,
  MULTI_CHILD,
  MULTI
} enkf_run_select_mode;

enum _enkf_filedialog_type
{
  GUI_CONFIG_FILE = 1,
  GUI_GRID_FILE = 2,
  GUI_DATA_FILE = 3,
  GUI_SCHEDULE_FILE = 4,
  GUI_EQUIL_FILE = 5
};
typedef enum _enkf_filedialog_type enkf_filedialog_type;

enum
{
  COL_FROM = 0,
  COL_TO,
  NUM_COLS
};

enum
{
  CONFIG_ITEM = 0,
  CONFIG_EVENT,
  CONFIG_OBJECT,
  CONFIG_NUM_COLS
};


void on_button23_clicked (GtkButton * button, gpointer user_data);

void on_button38_clicked (GtkButton * button, gpointer user_data);

void on_button92_clicked (GtkButton * button, gpointer user_data);

void on_button1_clicked (GtkButton * button, gpointer user_data);

void on_button59_clicked (GtkButton * button, gpointer user_data);

void open_button_clicked (GtkButton * button, gpointer user_data);


/********************************************************************
 ******************** Config quality check **************************
 ********************************************************************/

extern void enkf_gui_qc_tree_clicked (GtkTreeModel * tree_model,
				      GtkTreePath * path,
				      GtkTreeViewColumn * col,
				      gpointer userdata);

extern gboolean enkf_gui_qc_tree_buttonpress (GtkWidget * treeview,
					      GdkEventButton * event,
					      gpointer userdata);

/********************************************************************
 ******************** Enkf running window  **************************
 ********************************************************************/
extern void
enkf_gui_runtree_row_activated (GtkTreeView * tree, GtkTreePath * path,
				GtkTreeViewColumn * col, gpointer userdata);

extern gboolean enkf_gui_runtree_buttonpress (GtkWidget * treeview,
					      GdkEventButton * event,
					      gpointer userdata);
extern void enkf_gui_qc_run_clicked (GtkButton * button, gpointer user_data);

void on_new1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_open2_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_save2_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_save_as2_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_quit2_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_item1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_standard1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_all_summary_variables1_activate (GtkMenuItem * menuitem,
				    gpointer user_data);

void
on_genkw_parameter1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_all_all_genkw_parameters1_activate (GtkMenuItem * menuitem,
				       gpointer user_data);

void on_observation1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_depth1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_time1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_all1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_sensitivity1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_historgram1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_scalar_value_to_csv1_activate (GtkMenuItem * menuitem, gpointer user_data);

void on_to_rms_roff1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
  on_eclipse_restart__active_cells_1_activate
  (GtkMenuItem * menuitem, gpointer user_data);

void
  on_eclipse_restart__all_cells_1_activate
  (GtkMenuItem * menuitem, gpointer user_data);

void on_p_a____x___b_1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_python_module_o1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
  on_cell_values_to_text_file_s_1_activate
  (GtkMenuItem * menuitem, gpointer user_data);

void
  on_line_profile_of_a_field_to_text_file_s_1_activate
  (GtkMenuItem * menuitem, gpointer user_data);

void
  on_time_development_in_one_cell_to_text_file_s_1_activate
  (GtkMenuItem * menuitem, gpointer user_data);

void
on_gendata_genparam_to_file1_activate (GtkMenuItem * menuitem,
				       gpointer user_data);

void on_about2_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_timestep_as_parent1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
on_member_as_parent1_activate (GtkMenuItem * menuitem, gpointer user_data);

void
enkf_gui_timestep_as_parent_activate (GtkMenuItem * menuitem,
				      gpointer user_data);

void
enkf_gui_member_as_parent_activate (GtkMenuItem * menuitem,
				    gpointer user_data);

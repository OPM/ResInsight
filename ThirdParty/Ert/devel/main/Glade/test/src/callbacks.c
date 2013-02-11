/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'callbacks.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifdef HAVE_CONFIG_H
#  include <gui_config.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "enkf_setup.h"
#include "main.h"


#define GLADE_HOOKUP_OBJECT(component,widget,name) \
	  g_object_set_data_full (G_OBJECT (component), name, \
	  gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)



static void enkf_gui_runtree_buttonpress_menu (GtkWidget * treeview,
					       GdkEventButton * event,
					       gpointer userdata);
static void enkf_gui_select_all_ensemble_members (GtkMenuItem * menuitem,
						  gpointer userdata);

static void enkf_gui_qc_menupress (GtkWidget * menuitem, gpointer userdata);





gboolean
on_window1_delete_event (GtkWidget * widget,
			 GdkEvent * event, gpointer user_data)
{
  gtk_main_quit ();
  return FALSE;
}


gboolean
on_window1_destroy_event (GtkWidget * widget,
			  GdkEvent * event, gpointer user_data)
{
  printf ("Destroy event\n");
  gtk_main_quit ();

  return FALSE;
}


void
on_save1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_save_as1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_quit1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  gtk_main_quit ();
}



void
on_about1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


/********************************************************************
 ************************ Filedialog code ***************************
 ********************************************************************/


void
on_button2_filedialog_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *dialog;
  GtkWidget *window1;
  GtkWidget *notebook;
  const char *filename;
  enkf_filedialog_type command;
  enkf_main_type *enkf_main;

  dialog = lookup_widget (GTK_WIDGET (button), "filechooserdialog1");
  window1 = lookup_widget (GTK_WIDGET (dialog), "parent_window");
  notebook = lookup_widget (GTK_WIDGET (window1), "notebook1");
  command =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog), "fd_command"));

  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

  if (!lookup_widget (GTK_WIDGET (window1), "enkf_main")
      && command != GUI_CONFIG_FILE)
    {
      printf ("Enkf has not been bootstrapped!\n");
    }
  gtk_widget_show (notebook);


  switch (command)
    {
      GtkWidget *tmp;

    case GUI_CONFIG_FILE:
      enkf_main = enkf_setup_bootstrap (filename, window1);
      g_object_set_data (G_OBJECT (window1), "enkf_main", enkf_main);
      break;
    case GUI_GRID_FILE:
      tmp = lookup_widget (GTK_WIDGET (window1), "entry21");
      gtk_entry_set_text (GTK_ENTRY (tmp), filename);
      printf ("Insert the grid file\n");
      break;
    case GUI_DATA_FILE:
      printf ("Opened the data file\n");
      tmp = lookup_widget (GTK_WIDGET (window1), "entry23");
      gtk_entry_set_text (GTK_ENTRY (tmp), filename);
      break;
    case GUI_SCHEDULE_FILE:
      printf ("Opened the schedule file\n");
      tmp = lookup_widget (GTK_WIDGET (window1), "entry22");
      gtk_entry_set_text (GTK_ENTRY (tmp), filename);
      break;
    case GUI_EQUIL_FILE:
      printf ("Opened the schedule file\n");
      tmp = lookup_widget (GTK_WIDGET (window1), "entry24");
      gtk_entry_set_text (GTK_ENTRY (tmp), filename);
      break;
    default:
      break;
    }

  printf ("Chose file: %s\n", filename);
  gtk_widget_destroy (dialog);

}



void
on_button19_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *tv;
  GtkWidget *from;
  GtkWidget *to;
  const char *str_from = NULL;
  const char *str_to = NULL;
  GtkWidget *treestore;
  GtkTreeIter toplevel;

  tv = lookup_widget (GTK_WIDGET (button), "treeview3");
  from = lookup_widget (GTK_WIDGET (button), "entry27");
  to = lookup_widget (GTK_WIDGET (button), "entry26");
  treestore = lookup_widget (GTK_WIDGET (button), "treestore");

  str_from = gtk_entry_get_text (GTK_ENTRY (from));
  str_to = gtk_entry_get_text (GTK_ENTRY (to));

  if (strlen (str_from) <= 0 || strlen (str_to) <= 0)
    return;

  gtk_tree_store_append (GTK_TREE_STORE (treestore), &toplevel, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (treestore), &toplevel, COL_FROM,
		      str_from, COL_TO, str_to, -1);
  printf ("Adding to keep run path, from: %s, to: %s\n", str_from, str_to);
  gtk_entry_set_text (GTK_ENTRY (from), "");
  gtk_entry_set_text (GTK_ENTRY (to), "");
}


void
on_button1_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *dialog;
  dialog = lookup_widget (GTK_WIDGET (button), "filechooserdialog1");
  gtk_widget_destroy (dialog);
}


void
open_button_clicked (GtkButton * button, gpointer user_data)
{
  enkf_filedialog_type command;
  GtkWidget *w;
  GtkWidget *window1;

  command =
    GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "open_command"));
  window1 = lookup_widget (GTK_WIDGET (button), "window1");

  w = create_filechooserdialog1 ();
  g_object_set_data (G_OBJECT (w), "parent_window", window1);
  g_object_set_data (G_OBJECT (w), "fd_command", GINT_TO_POINTER (command));

  gtk_widget_show (w);
}


/********************************************************************
 *************** Normal information dialog box **********************
 ********************************************************************/

void
on_button9_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *w;
  w = create_dialog1 ();
  gtk_widget_show (w);

}


void
on_okbutton1_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *dialog;
  dialog = lookup_widget (GTK_WIDGET (button), "dialog1");
  gtk_widget_destroy (dialog);

}


/********************************************************************
 ********************* Nootbook code ********************************
 ********************************************************************/

void
on_button23_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *w;
  GtkWidget *win;

  w = lookup_widget (GTK_WIDGET (button), "notebook1");
  win = lookup_widget (GTK_WIDGET (button), "window1");
  printf ("Moving to next page\n");
  gtk_notebook_next_page (GTK_NOTEBOOK (w));
  /*
     enkf_main_type *enkf_main;
     * enkf_main = (enkf_main_type *) g_object_get_data(G_OBJECT(win),
     * "enkf_main");
     * 
     * printf("Enkf main: %p\n", enkf_main); { const ensemble_config_type
     * * ensemble_config = enkf_main_get_ensemble_config(enkf_main); const 
     * int ens_size = ensemble_config_get_size(ensemble_config); bool *
     * iactive = util_malloc(ens_size * sizeof * iactive , __func__); int
     * iens;
     * 
     * for (iens= 0; iens < ens_size; iens++) iactive[iens] = true;
     * 
     * enkf_tui_init(enkf_main, true , true); enkf_main_run(enkf_main ,
     * ENKF_ASSIMILATION , iactive , -1 , 0 , analyzed);
     * 
     * free(iactive); } 
   */
}


void
on_button92_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *w;
  w = lookup_widget (GTK_WIDGET (button), "notebook1");
  printf ("Moving to prev page\n");
  gtk_notebook_prev_page (GTK_NOTEBOOK (w));

}


/********************************************************************
 ******************** Config quality check **************************
 ********************************************************************/

void
enkf_gui_qc_tree_clicked (GtkTreeModel * tree_model, GtkTreePath * path,
			  GtkTreeViewColumn * col, gpointer userdata)
{

  GtkWidget *w;
  GtkTreeIter iter;
  gchar *kw;
  GtkTreeStore *treestore =
    GTK_TREE_STORE (lookup_widget (GTK_WIDGET (tree_model), "treestore2"));

  gtk_tree_model_get_iter (GTK_TREE_MODEL (treestore), &iter, path);
  /*
     gint *indices = gtk_tree_path_get_indices (path);
     gint row = indices[0];
   */

  gtk_tree_model_get (GTK_TREE_MODEL (treestore), &iter, CONFIG_ITEM, &kw,
		      -1);

  if (!strcmp ("SCHEDULE_FILE", kw))
    {
      GtkWidget *entry;
      w = lookup_widget (GTK_WIDGET (tree_model), "notebook1");
      entry = lookup_widget (GTK_WIDGET (tree_model), "entry22");
      gtk_notebook_set_current_page (GTK_NOTEBOOK (w), 0);
      gtk_widget_grab_focus (entry);
    }
  else
    {
      printf ("no event defined for this kw!\n");
    }


}

void
enkf_gui_qc_run_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *tv;
  GtkTreeStore *treestore;
  gint i;
  gboolean valid;
  GtkTreeIter iter;
  const char *map[3][3] = {
    {"GRID_FILE", "Everything is OK!", "gtk-ok"},
    {"SCHEDULE_FILE",
     "Error: Not a valid schedule file! Please correct this..",
     "gtk-stop"},
    {"DATA_FILE", "Everything is OK! (Valid file)", "gtk-ok"}
  };

  tv = lookup_widget (GTK_WIDGET (button), "treeview9");
  treestore =
    GTK_TREE_STORE (lookup_widget (GTK_WIDGET (button), "treestore2"));

  /*
   * Clear the treeview if it contains data 
   */
  valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (treestore), &iter);
  while (valid)
    valid = gtk_tree_store_remove (GTK_TREE_STORE (treestore), &iter);

  for (i = 0; i < 3; i++)
    {
      gtk_tree_store_append (GTK_TREE_STORE (treestore), &iter, NULL);
      gtk_tree_store_set (GTK_TREE_STORE (treestore), &iter, CONFIG_ITEM,
			  map[i][0], CONFIG_EVENT, map[i][1],
			  CONFIG_OBJECT, map[i][2], -1);
    }

  /*
   * Add some dummy data 
   */
  for (i = 0; i < 15; i++)
    {
      gchar *str;
      if (i > 4 && i < 8)
	str = strdup ("gtk-stop");
      else
	str = strdup ("gtk-ok");
      gtk_tree_store_append (GTK_TREE_STORE (treestore), &iter, NULL);
      gtk_tree_store_set (GTK_TREE_STORE (treestore), &iter, CONFIG_ITEM,
			  "DUMMY_KEYWORD", CONFIG_EVENT,
			  "Some check for the dummy keyword. ",
			  CONFIG_OBJECT, str, -1);
      g_free (str);
    }
}



static void
enkf_gui_qc_menupress (GtkWidget * menuitem, gpointer userdata)
{
  GtkTreeView *treeview = GTK_TREE_VIEW (userdata);
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *kw;
  GtkTreeSelection *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, CONFIG_ITEM, &kw, -1);

      if (!strcmp ("SCHEDULE_FILE", kw))
	{
	  GtkWidget *entry;
	  GtkWidget *w;
	  w = lookup_widget (GTK_WIDGET (treeview), "notebook1");
	  entry = lookup_widget (GTK_WIDGET (treeview), "entry22");
	  gtk_notebook_set_current_page (GTK_NOTEBOOK (w), 0);
	  gtk_widget_grab_focus (entry);
	}
      else
	{
	  printf ("no event defined for this kw!\n");
	}

    }

}


static void
enkf_gui_qc_handle_selection_events (GtkWidget * treeview,
				     GdkEventButton * event)
{
  GtkTreeSelection *selection;
  GtkWidget *tv = lookup_widget (GTK_WIDGET (treeview), "textview3");
  GtkTextBuffer *buffer;
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *kw;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

  /*
   * Handle the selections in the tree correctly.
   */
  if (gtk_tree_selection_count_selected_rows (selection) <= 1)
    {
      GtkTreePath *path;

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
					 (gint) event->x,
					 (gint) event->y,
					 &path, NULL, NULL, NULL))
	{
	  gtk_tree_selection_unselect_all (selection);
	  gtk_tree_selection_select_path (selection, path);
	  gtk_tree_path_free (path);
	}
    }

  /*
   * Fill some information into the textview.
   */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, CONFIG_ITEM, &kw, -1);
      gtk_text_buffer_set_text (buffer, kw, -1);
    }

  return;
}


gboolean
enkf_gui_qc_tree_buttonpress (GtkWidget * treeview, GdkEventButton * event,
			      gpointer userdata)
{
  GtkWidget *menu, *menuitem;

  /*
   * Single right click 
   */
  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      enkf_gui_qc_handle_selection_events (treeview, event);
      menu = gtk_menu_new ();
      menuitem = gtk_menu_item_new_with_label ("Correct error");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      g_signal_connect (menuitem, "activate",
			(GCallback) enkf_gui_qc_menupress, treeview);

      menuitem = gtk_menu_item_new_with_label ("Help information");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      g_signal_connect (menuitem, "activate",
			(GCallback) on_button9_clicked, treeview);
      gtk_widget_show_all (menu);

      gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
		      (event != NULL) ? event->button : 0,
		      gdk_event_get_time ((GdkEvent *) event));
      return TRUE;
    }

  /*
   * Single click 
   */
  if (event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
      enkf_gui_qc_handle_selection_events (treeview, event);
      return TRUE;
    }

  return FALSE;
}


/********************************************************************
 ******************** Enkf running window  **************************
 ********************************************************************/

void
enkf_gui_runtree_row_activated (GtkTreeView * tree, GtkTreePath * path,
				GtkTreeViewColumn * col, gpointer userdata)
{

  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkExpander *expander;
  gchar *str;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree));
  expander = GTK_EXPANDER (lookup_widget (GTK_WIDGET (tree), "expander1"));

  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, ENSEMBLE_MEMBER,
		      &str, -1);

  gtk_expander_set_expanded (expander, TRUE);

  if (gtk_expander_get_expanded (expander))
    {
      gchar buf[64];
      snprintf (buf, 64, "Details for Ensemble member %s", str);
      gtk_expander_set_label (expander, buf);
    }

  g_free (str);

}

static void
enkf_gui_handle_selection_events (GtkWidget * treeview,
				  GdkEventButton * event)
{
  GtkTreeSelection *selection;
  gint k;


  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  k = gtk_tree_selection_count_selected_rows (selection);
  printf ("Selected rows was %d\n", k);

  /*
   * Handle the selections in the tree correctly.
   */
  if (k <= 1)
    {
      GtkTreePath *path;

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
					 (gint) event->x,
					 (gint) event->y,
					 &path, NULL, NULL, NULL))
	{
	  gtk_tree_selection_unselect_all (selection);
	  gtk_tree_selection_select_path (selection, path);
	  gtk_tree_path_free (path);
	}
    }

  return;
}

static enkf_run_select_mode
enkf_gui_decide_menutype (GtkWidget * treeview)
{
  enkf_run_select_mode menu_mode;
  GtkTreeModel *model;
  GList *path_list;
  GList *l;
  gboolean flag = FALSE;
  gint counter = 0;
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

  /*
   * Get the stored enkf_gui data structure and look trough it.
   */
  path_list =
    gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection),
					  NULL);
  for (l = path_list; l != NULL; l = g_list_next (l))
    {
      GtkTreePath *path = l->data;
      gchar *str;
      enkf_gui *g;

      gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, ENSEMBLE_MEMBER,
			  &str, ENKF_POINTER, &g, -1);
      counter++;
      if (g->type == PARENT)
	flag = TRUE;

      gtk_tree_path_free (path);
      g_free (str);
    }

  g_list_free (path_list);

  /*
   * Decide what menu to build.
   */
  if (flag && counter > 1)
    menu_mode = MULTI;
  else if (flag && counter == 1)
    menu_mode = ONLY_PARENT;
  else if (!flag && counter == 1)
    menu_mode = ONLY_CHILD;
  else if (!flag && counter > 1)
    menu_mode = MULTI_CHILD;

  return menu_mode;
}

gboolean
enkf_gui_runtree_buttonpress (GtkWidget * treeview, GdkEventButton * event,
			      gpointer userdata)
{
  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      enkf_gui_handle_selection_events (treeview, event);
      enkf_gui_runtree_buttonpress_menu (treeview, event, userdata);
      return TRUE;
    }

  return FALSE;
}

static void
enkf_gui_select_all_ensemble_members (GtkMenuItem * menuitem,
				      gpointer userdata)
{
  GtkTreeView *treeview = userdata;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GtkTreeIter parent_iter;
  gint ens_members;
  GtkTreeIter first_iter;
  GtkTreeIter last_iter;
  GtkTreePath *first_path;
  GtkTreePath *last_path;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

  /* Grab the first root node */
  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model), &iter, NULL, 0);
  parent_iter = iter;
  ens_members =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), &parent_iter);

  /* Grab the first and last child nodes */
  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model), &first_iter,
				 &parent_iter, 0);
  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model), &last_iter,
				 &parent_iter, ens_members - 1);
  first_path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &first_iter);
  last_path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &last_iter);

  /* Do the selection (unselect the parent first) */
  gtk_tree_selection_unselect_all (selection);
  gtk_tree_selection_select_range (GTK_TREE_SELECTION (selection), first_path,
				   last_path);

  gtk_tree_path_free (first_path);
  gtk_tree_path_free (last_path);
}

static void
enkf_gui_runtree_buttonpress_menu (GtkWidget * treeview,
				   GdkEventButton * event, gpointer userdata)
{
  GtkWidget *menu, *menuitem;
  enkf_run_select_mode menu_mode;

  menu_mode = enkf_gui_decide_menutype (treeview);

  /*
   * Build the correct menu.
   */
  menu = gtk_menu_new ();
  switch (menu_mode)
    {
    case ONLY_PARENT:
      printf ("Only parent selected!\n");
      menuitem = gtk_menu_item_new_with_label ("Restart timstep");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

      menuitem = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

      menuitem = gtk_menu_item_new_with_label ("Stop all jobs");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

      menuitem = gtk_menu_item_new_with_label ("Select all ensemble members");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      g_signal_connect (menuitem, "activate",
			G_CALLBACK (enkf_gui_select_all_ensemble_members),
			treeview);
      break;
    case ONLY_CHILD:
      printf ("Only child selected!\n");
      menuitem = gtk_menu_item_new_with_label ("Kill selected job");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      break;
    case MULTI_CHILD:
      printf ("Multi child selected\n");
      menuitem = gtk_menu_item_new_with_label ("Kill selected jobs");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      break;
    case MULTI:
      printf ("Child and parent selected\n");
      menuitem =
	gtk_menu_item_new_with_label ("Child AND parent selected?!?");
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      break;
    default:
      break;
    }

  gtk_widget_show_all (menu);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
		  (event != NULL) ? event->button : 0,
		  gdk_event_get_time ((GdkEvent *) event));
}

void
on_new1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_open2_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_save2_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_save_as2_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_quit2_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_item1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_standard1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_all_summary_variables1_activate (GtkMenuItem * menuitem,
				    gpointer user_data)
{

}


void
on_genkw_parameter1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_all_all_genkw_parameters1_activate (GtkMenuItem * menuitem,
				       gpointer user_data)
{

}


void
on_observation1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_depth1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_time1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_all1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_sensitivity1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_historgram1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_scalar_value_to_csv1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_to_rms_roff1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
  on_eclipse_restart__active_cells_1_activate
  (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
  on_eclipse_restart__all_cells_1_activate
  (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_p_a____x___b_1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_python_module_o1_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
  on_cell_values_to_text_file_s_1_activate
  (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
  on_line_profile_of_a_field_to_text_file_s_1_activate
  (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
  on_time_development_in_one_cell_to_text_file_s_1_activate
  (GtkMenuItem * menuitem, gpointer user_data)
{

}


void
on_gendata_genparam_to_file1_activate (GtkMenuItem * menuitem,
				       gpointer user_data)
{

}


void
on_about2_activate (GtkMenuItem * menuitem, gpointer user_data)
{

}

void
enkf_gui_timestep_as_parent_activate (GtkMenuItem * menuitem,
				      gpointer user_data)
{
  GtkWidget *window2 = lookup_widget (GTK_WIDGET (menuitem), "window2");
  GtkTreeStore *store =
    GTK_TREE_STORE (lookup_widget (GTK_WIDGET (window2), "treestore"));
  GtkTreeViewColumn *column_timestep;
  GtkTreeViewColumn *column_ensemble_member;
  GtkWidget *tree = lookup_widget (GTK_WIDGET (window2), "treeview10");

  column_timestep =
    GTK_TREE_VIEW_COLUMN (lookup_widget
			  (GTK_WIDGET (window2), "column_timestep"));
  column_ensemble_member =
    GTK_TREE_VIEW_COLUMN (lookup_widget
			  (GTK_WIDGET (window2), "column_ensemble_member"));

  gtk_tree_store_clear (store);
  store = enkf_gui_store_timestep_parent ();
  g_object_set_data (G_OBJECT (window2), "treestore", GTK_TREE_MODEL (store));
  gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree),
				   column_ensemble_member, column_timestep);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  g_object_unref (G_OBJECT (store));

}


void
enkf_gui_member_as_parent_activate (GtkMenuItem * menuitem,
				    gpointer user_data)
{
  GtkWidget *window2 = lookup_widget (GTK_WIDGET (menuitem), "window2");
  GtkTreeStore *store =
    GTK_TREE_STORE (lookup_widget (GTK_WIDGET (window2), "treestore"));
  GtkTreeViewColumn *column_timestep;
  GtkTreeViewColumn *column_ensemble_member;
  GtkWidget *tree = lookup_widget (GTK_WIDGET (window2), "treeview10");

  column_timestep =
    GTK_TREE_VIEW_COLUMN (lookup_widget
			  (GTK_WIDGET (window2), "column_timestep"));
  column_ensemble_member =
    GTK_TREE_VIEW_COLUMN (lookup_widget
			  (GTK_WIDGET (window2), "column_ensemble_member"));

  gtk_tree_store_clear (store);
  store = enkf_gui_store_ensemble_member_parent ();
  g_object_set_data (G_OBJECT (window2), "treestore", GTK_TREE_MODEL (store));
  gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree), column_timestep,
				   column_ensemble_member);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  g_object_unref (G_OBJECT (store));

}

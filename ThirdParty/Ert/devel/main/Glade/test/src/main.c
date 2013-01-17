/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'main.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include "main.h"
#include "callbacks.h"
#include "enkf_setup.h"
#include "custom-cell-renderer-progressbar.h"

#define STEP  0.03

static gboolean enkf_gui_increase_progress_timeout (gpointer * data);



void
setup_quality_check (GtkWidget * window1)
{
  GtkWidget *tv;
  GtkTreeViewColumn *col;
  GtkCellRenderer *renderer;
  GtkTreeStore *treestore;

  tv = lookup_widget (GTK_WIDGET (window1), "treeview9");

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, "Config item");
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", CONFIG_ITEM);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, "Event");
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", CONFIG_EVENT);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (col, 20);
  gtk_tree_view_column_set_title (col, "Status");
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "stock-id",
				      CONFIG_OBJECT);

  treestore =
    gtk_tree_store_new (CONFIG_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tv), GTK_TREE_MODEL (treestore));
  g_object_set_data (G_OBJECT (window1), "treestore2",
		     GTK_TREE_MODEL (treestore));

  g_signal_connect (GTK_WIDGET (tv), "row-activated",
		    G_CALLBACK (enkf_gui_qc_tree_clicked), NULL);
  g_signal_connect (tv, "button-press-event",
		    (GCallback) enkf_gui_qc_tree_buttonpress, NULL);
  g_object_unref (treestore);
}

static void
setup_widgets_config_win (GtkWidget * window1)
{
  GtkWidget *from;
  GtkWidget *to;
  GtkTreeViewColumn *col;
  GtkTreeStore *treestore;
  GtkCellRenderer *renderer;
  GtkWidget *tv;
  enkf_main_type *enkf_main;

  tv = lookup_widget (GTK_WIDGET (window1), "treeview3");
  from = lookup_widget (GTK_WIDGET (window1), "entry27");
  to = lookup_widget (GTK_WIDGET (window1), "entry26");

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, "From");
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_FROM);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, "To");
  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COL_TO);

  treestore = gtk_tree_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tv), GTK_TREE_MODEL (treestore));
  g_object_set_data (G_OBJECT (window1), "treestore",
		     GTK_TREE_MODEL (treestore));

  g_object_unref (treestore);

  g_object_set_data (G_OBJECT (lookup_widget (GTK_WIDGET (window1), "open1")),
		     "open_command", GINT_TO_POINTER (GUI_CONFIG_FILE));
  g_object_set_data (G_OBJECT
		     (lookup_widget (GTK_WIDGET (window1), "button57")),
		     "open_command", GINT_TO_POINTER (GUI_GRID_FILE));
  g_object_set_data (G_OBJECT
		     (lookup_widget (GTK_WIDGET (window1), "button58")),
		     "open_command", GINT_TO_POINTER (GUI_SCHEDULE_FILE));
  g_object_set_data (G_OBJECT
		     (lookup_widget (GTK_WIDGET (window1), "button59")),
		     "open_command", GINT_TO_POINTER (GUI_DATA_FILE));
  g_object_set_data (G_OBJECT
		     (lookup_widget (GTK_WIDGET (window1), "button60")),
		     "open_command", GINT_TO_POINTER (GUI_EQUIL_FILE));

  enkf_main =
    enkf_setup_bootstrap ("./enkf_config",
			  window1);
  g_object_set_data (G_OBJECT (window1), "enkf_main", enkf_main);
  gtk_widget_show (GTK_WIDGET
		   (lookup_widget (GTK_WIDGET (window1), "notebook1")));

}


gint
sort_iter_compare_func (GtkTreeModel * model,
			GtkTreeIter * a, GtkTreeIter * b, gpointer userdata)
{
  gint sortcol = GPOINTER_TO_INT (userdata);
  gint ret = 0;

  switch (sortcol)
    {
      gchar *str1, *str2;
      gint k1, k2;
    case SORT_TIMESTEP:
      gtk_tree_model_get (model, a, TIMESTEP, &str1, -1);
      gtk_tree_model_get (model, b, TIMESTEP, &str2, -1);
      if (str1 == NULL || str2 == NULL)
	{
	  if (str1 == NULL && str2 == NULL)
	    break;
	}
      else
	{
	  printf ("%s, %s\n", str1, str2);
	  k1 = atoi (str1);
	  k2 = atoi (str2);
	  printf ("blah: %d, blah: %d\n", k1, k2);
	  ret = (k1 > k2) ? 1 : -1;
	}
      g_free (str1);
      g_free (str2);

      break;
    case SORT_ENSEMBLE_MEMBER:
      gtk_tree_model_get (model, a, ENSEMBLE_MEMBER, &str1, -1);
      gtk_tree_model_get (model, b, ENSEMBLE_MEMBER, &str2, -1);
      if (!strcmp (str1, "All") || !strcmp (str2, "All"))
	break;

      if (str1 == NULL || str2 == NULL)
	{
	  if (str1 == NULL && str2 == NULL)
	    break;
	}
      else
	{
	  printf ("%s, %s\n", str1, str2);
	  k1 = atoi (str1);
	  k2 = atoi (str2);
	  printf ("blah: %d, blah: %d\n", k1, k2);
	  ret = (k1 > k2) ? 1 : -1;
	}

      g_free (str1);
      g_free (str2);

      break;
    default:
      g_return_val_if_reached (0);
    }

  return ret;

}


/*
 * Builds the tree structure with an ENSEMBLE MEMBERS as parents
 * and timesteps as childs.
 */
GtkTreeStore *
enkf_gui_store_ensemble_member_parent ()
{
  GtkTreeStore *store;
  gint ens_size = 30;
  gint j;
  gint k;
  enkf_gui *g;
  GtkTreeIter iter1;
  GtkTreeIter iter2;

  store =
    gtk_tree_store_new (N_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT,
			G_TYPE_STRING, GTK_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_POINTER);
  for (j = 1; j <= ens_size; j++)
    {
      gchar buf[8];
      g = malloc (sizeof (enkf_gui));
      g->type = PARENT;
      g->timestep = -1;
      g->ensemble_member = j;
      gtk_tree_store_append (store, &iter1, NULL);
      snprintf (buf, 8, "%d", j);
      gtk_tree_store_set (store, &iter1, ENSEMBLE_MEMBER, buf, PROGRESS,
			  0.0, ENKF_POINTER, g, -1);
      for (k = 0; k < 4; k++)
	{
	  gchar str[8];
	  g = malloc (sizeof (enkf_gui));
	  g->type = CHILD;
          g->timestep = k;
          g->ensemble_member = j;
	  gtk_tree_store_append (store, &iter2, &iter1);
	  snprintf (str, 8, "%d", k);
          
	  gtk_tree_store_set (store, &iter2,
			      TIMESTEP, str, ENSEMBLE_MEMBER, NULL, PROGRESS,
			      0.0, PROGRESS_BUF, "", PROGRESS_ICON,
			      "gtk-redo", ENKF_POINTER, g, -1);
	}
    }

  return store;
}


/*
 * Builds the tree structure with an ENKF TIMESTEPS as parents
 * and ensemble members as childs.
 */
GtkTreeStore *
enkf_gui_store_timestep_parent ()
{
  GtkTreeStore *store;
  gint ens_size = 30;
  gint j;
  gint k;
  enkf_gui *g;
  GtkTreeIter iter1;
  GtkTreeIter iter2;

  store =
    gtk_tree_store_new (N_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT,
			G_TYPE_STRING, GTK_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_POINTER);
  for (k = 0; k < 4; k++)
    {
      gchar str[8];
      g = malloc (sizeof (enkf_gui));
      g->type = PARENT;
      g->timestep = k;
      g->ensemble_member = -1;
      gtk_tree_store_append (store, &iter1, NULL);
      snprintf (str, 8, "%d", k);
      gtk_tree_store_set (store, &iter1,
			  TIMESTEP, str, ENSEMBLE_MEMBER, "All", PROGRESS,
			  0.0, PROGRESS_BUF, "", ENKF_POINTER, g, -1);
      for (j = 1; j <= ens_size; j++)
	{
	  gchar buf[8];
	  g = malloc (sizeof (enkf_gui));
	  g->type = CHILD;
          g->ensemble_member = j;
          g->timestep = k;
	  gtk_tree_store_append (store, &iter2, &iter1);
	  snprintf (buf, 8, "%d", j);
	  gtk_tree_store_set (store, &iter2, ENSEMBLE_MEMBER, buf, PROGRESS,
			      0.0, PROGRESS_ICON, "gtk-redo", ENKF_POINTER,
			      g, -1);
	}
    }

  return store;
}


static void
setup_widgets_run_win (GtkWidget * window2)
{
  GtkTreeStore *store;
  GtkTreeViewColumn *column;
  GtkTreeViewColumn *column_timestep;
  GtkTreeViewColumn *column_ensemble_member;
  GtkCellRenderer *renderer;
  GtkWidget *tree;
  GtkTreeSelection *selection;
  GtkTreeSortable *sortable;

  store = enkf_gui_store_timestep_parent ();
  g_object_set_data (G_OBJECT (window2), "treestore", GTK_TREE_MODEL (store));

  tree = lookup_widget (GTK_WIDGET (window2), "treeview10");
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  sortable = GTK_TREE_SORTABLE (store);
  gtk_tree_sortable_set_sort_func (sortable, SORT_TIMESTEP,
				   sort_iter_compare_func,
				   GINT_TO_POINTER (SORT_TIMESTEP), NULL);
  gtk_tree_sortable_set_sort_func (sortable, SORT_ENSEMBLE_MEMBER,
				   sort_iter_compare_func,
				   GINT_TO_POINTER (SORT_ENSEMBLE_MEMBER),
				   NULL);
  g_object_unref (G_OBJECT (store));

  renderer = gtk_cell_renderer_text_new ();
  column_timestep =
    gtk_tree_view_column_new_with_attributes ("Timestep", renderer, "text",
					      TIMESTEP, NULL);
  gtk_tree_view_column_set_sort_column_id (column_timestep, SORT_TIMESTEP);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column_timestep);
  g_object_set_data (G_OBJECT (window2), "column_timestep", column_timestep);

  renderer = gtk_cell_renderer_text_new ();
  column_ensemble_member = gtk_tree_view_column_new_with_attributes ("Member",
								     renderer,
								     "text",
								     ENSEMBLE_MEMBER,
								     NULL);
  gtk_tree_view_column_set_sort_column_id (column_ensemble_member,
					   SORT_ENSEMBLE_MEMBER);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column_ensemble_member);
  g_object_set_data (G_OBJECT (window2), "column_ensemble_member",
		     column_ensemble_member);

  gtk_tree_view_move_column_after (GTK_TREE_VIEW (tree), column_timestep,
				   column_ensemble_member);


  renderer = custom_cell_renderer_progress_new ();
  column = gtk_tree_view_column_new_with_attributes ("Progress",
						     renderer, "percentage",
						     PROGRESS);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("",
						     renderer,
						     "text", PROGRESS_BUF,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes ("Icon", renderer,
						     "stock-id",
						     PROGRESS_ICON, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Status message",
						     renderer,
						     "text", COL_STATUS,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  g_signal_connect (GTK_TREE_VIEW (tree), "button-press-event",
		    (GCallback) enkf_gui_runtree_buttonpress, NULL);
  g_signal_connect (GTK_WIDGET (tree), "row-activated",
		    G_CALLBACK (enkf_gui_runtree_row_activated), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_MULTIPLE);

  //gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (member_as_parent), FALSE);
  /*
   * Expand the first timestep node and display all children.
   */
  /*
     path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter1);
     gtk_tree_view_expand_row (GTK_TREE_VIEW (tree), path, TRUE);
     gtk_tree_path_free (path);
   */

  g_timeout_add (1000, (GSourceFunc) enkf_gui_increase_progress_timeout, window2);

}

static gboolean
enkf_gui_increase_progress_timeout (gpointer * data)
{
  GtkTreeIter iter;
  GtkTreeIter child_iter;
  gfloat perc = 0.0;
  gchar buf[20];
  GtkTreeStore *store;
  gint ens_members;
  gfloat sum = 0.0;
  gfloat total_sum = 0.0;
  GtkProgressBar *pbar;
  gint parent_iterations;
  gint child_iterations;

  store = GTK_TREE_STORE (lookup_widget (GTK_WIDGET (data), "treestore"));
  pbar = GTK_PROGRESS_BAR (lookup_widget (GTK_WIDGET (data), "progressbar1"));

  {
    gint i;
    gdouble step;
    gint r;
    gint r2;
    
    gchar *map[3] = { "PENDING: Waiting for cluster.",
      "RESTART: ECLIPSE restart!",
      "RUNNING: job running in normal mode."
    };
    
    parent_iterations = 0;
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while(r)
      {
        enkf_gui *g;
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, PROGRESS, &perc,
            ENKF_POINTER, &g, -1);

        r2 = gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child_iter, &iter);
        child_iterations = 0;
        while (r2) 
        {
          gchar *string;
          gchar *ptr;
          string = strdup ("gtk-redo");

          r = rand () % 3;
          step = STEP * drand48 ();

          gtk_tree_model_get (GTK_TREE_MODEL (store), &child_iter, PROGRESS, &perc,
              ENKF_POINTER, &g, -1);

          if (g->timestep == 0) {
          
            perc = perc + step;
            if (perc > 1)
              {
                perc = 1;
                free (string);
                string = strdup ("gtk-ok");
                ptr = strdup ("FINISHED");
              }
            else
              ptr = strdup (map[r]);
              
            g_snprintf (buf, sizeof (buf), "%u %%", (guint) (perc * 100));
            gtk_tree_store_set (GTK_TREE_STORE (store), &child_iter, PROGRESS, perc,
                                PROGRESS_BUF, buf, PROGRESS_ICON, string,
                                COL_STATUS, ptr, -1);
            sum += perc;

            g_free (string);
            g_free (ptr);
          }
          
          child_iterations += 1;

          r2 = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &child_iter);
        }

        
        g_snprintf (buf, sizeof (buf), "%u %%", (guint) (sum * 100) / child_iterations);
        gtk_tree_store_set (GTK_TREE_STORE (store), &iter, PROGRESS,
                            sum / child_iterations, PROGRESS_BUF, buf, -1);
        total_sum += sum;
        sum = 0;
        parent_iterations += 1;

        r = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
      }
  }
    
  {
    gchar new_buf[64];
    
    snprintf (new_buf, 64, "Total progress: %u %%", (guint) (total_sum * 100) / (child_iterations * parent_iterations));
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pbar), new_buf);
    gtk_progress_bar_update (GTK_PROGRESS_BAR (pbar), total_sum / (child_iterations * parent_iterations));
  }

  return TRUE;
}



int
main (int argc, char *argv[])
{
  GtkWidget *window1;
  GtkWidget *window2;
  time_t t1;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  (void) time (&t1);
  srand48 ((long) t1);

  add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");
  add_pixmap_directory ("../pixmaps");

  window1 = create_window1 ();
  setup_widgets_config_win (window1);
  setup_quality_check (window1);
  gtk_widget_show (window1);

  window2 = create_window2 ();
  setup_widgets_run_win (window2);
  gtk_widget_show (window2);

  gtk_main ();


  return 0;
}

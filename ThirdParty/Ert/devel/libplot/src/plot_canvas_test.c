/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_canvas_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <glib.h>
#include <gtk/gtk.h>
#include <plplot/plplotcanvas.h>


gboolean plot_canvas_data_join(gpointer data)
{
    plot_type *item = data;
    list_node_type *node, *next_node;
    int len;
    double *x;
    double *y;
    int step;
    int flag = true;


    node = list_get_head(plot_get_datasets(item));
    while (node != NULL) {
	plot_dataset_type *tmp;
	next_node = list_node_get_next(node);
	tmp = list_node_value_ptr(node);
	len = plot_datset_get_length(tmp);

	if (plot_dataset_is_finished(tmp)) {
	    node = next_node;
	    continue;
	}

	flag = false;

	if (plot_dataset_get_step(tmp) == len) {
	    printf("ID[%d] Plotted last node, skipping..\n",
		   plot_get_stream(item));
	    plot_dataset_finished(tmp, true);
	    node = next_node;
	    continue;
	}

	step = plot_dataset_step_next(tmp);
	x = plot_datset_get_vector_x(tmp);
	y = plot_datset_get_vector_y(tmp);
	plot_dataset_join(item, tmp, step - 1, step);
	printf("ID[%d] Plotting step %d -> %d of total %d\n",
	       plot_get_stream(item), step - 1, step, len);
	node = next_node;
    }

    plplot_canvas_adv(plot_get_canvas(item), 0);

    if (!flag)
	return true;

    return false;
}

void destroy_local(GtkWidget * widget, gpointer data)
{
    plot_free((plot_type *) data);
    gtk_main_quit();
    widget = NULL;
}

int main(int argc, char *argv[])
{
    GtkWidget *win;
    GtkBox *vbox;
    plot_type *item;
    PlplotCanvas *canvas;

    plparseopts(&argc, (const char **) argv, PL_PARSE_FULL);
    gtk_init(&argc, &argv);

    /* 
     * CREATE A PLOT ITEM
     */
    item = plot_alloc();
    plot_initialize(item, NULL, NULL, CANVAS);
    plot_set_labels(item, "x-axis", "y-axis", "Canvas test", BLACK);
    plot_set_viewport(item, 0, 2 * PI, -1, 1);

    /* 
     * START GTK PACKING CODE 
     */
    canvas = plot_get_canvas(item);
    vbox = GTK_BOX(gtk_vbox_new(FALSE, 0));
    gtk_box_pack_start(vbox, GTK_WIDGET(canvas), TRUE, TRUE, 0);
    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(win), 5);
    g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(destroy_local),
		     item);
    gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(vbox));

    /* 
     * START CREATING PLOT DATA
     */
    {
	plot_dataset_type *d;
	const double period = 2 * PI;
	int i;
	int N = pow(2, 6);
	{
	    PLFLT x[N];
	    PLFLT y[N];

	    for (i = 0; i <= N; i++) {
		x[i] = i * period / N;
		y[i] = sin(x[i]);
	    }
	    d = plot_dataset_alloc();
	    plot_dataset_set_data(d, x, y, N, BLUE, LINE);
	    plot_dataset_add(item, d);
	}

	{
	    PLFLT x[N * 2];
	    PLFLT y[N * 2];

	    for (i = 0; i <= N * 2; i++) {
		x[i] = i * period / (N * 2);
		y[i] = cos(x[i]);
	    }
	    d = plot_dataset_alloc();
	    plot_dataset_set_data(d, x, y, (N * 2), RED, LINE);
	    plot_dataset_add(item, d);
	}
    }

    /* 
     * PLOT THE DATA WITH TIMER FUNCTIONS
     */
    g_timeout_add(100, plot_canvas_data_join, item);

    gtk_widget_show_all(win);
    gtk_main();

    return 0;
}

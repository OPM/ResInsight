/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary_plot_live.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <glib.h>
#include <gtk/gtk.h>
#include <ecl_kw.h>
#include <ecl_sum.h>
#include <list.h>
#include <list_node.h>
#include <config.h>
#include <time.h>
#include <assert.h>

#define TIMEOUT 10000           /* unit: millisecond */
#define ECL_EXT ".DATA"

/***************************************************************
 ***************************************************************/

typedef struct summary_plot_gui_struct {
    GtkWidget *win;
    GtkTextBuffer *buffer;
    GtkWidget *text;
    GtkNotebook *nb;
    config_parser_type *config;
    const char *conf_file;
    list_type *list;
} summary_plot_gui_type;

typedef struct summary_plot_struct {
    summary_plot_gui_type *spg; /* "parent" pointer */
    plot_type *item;
    list_node_type *node;
    list_type *list;
    double x_max;
    double y_max;
    int timer_id;
    char *kw;
    bool off_charts;
} summary_plot_type;


typedef struct summary_plot_member_struct {
    int files;
    char *dir;
    char *file;
    int last_report_step;
} summary_plot_member_type;

/***************************************************************
 ***************************************************************/

static summary_plot_type *summary_plot_alloc();
static void summary_plot_free(summary_plot_type * sp);
static summary_plot_member_type *summary_plot_member_alloc();
static void summary_plot_member_free(summary_plot_member_type * spm);
static summary_plot_gui_type *summary_plot_gui_alloc();
static void summary_plot_gui_free(summary_plot_gui_type * spg);
static void summary_plot_get_ecl_data(summary_plot_member_type * spm,
                                      char ***sfl, char **header_file,
                                      int *files);
static void summary_plot_add_ensamble_member(summary_plot_type * sp,
                                             char *dir, char *file);
static gboolean summary_plot_timout(gpointer data);
static char *summary_plot_get_timestamp();
static void summary_plot_append_textbox(summary_plot_gui_type * spg,
                                        const char *str, ...);
static config_parser_type *summary_plot_init_config(const char *config_file);
static void summary_plot_initialize_ensembles(summary_plot_type * sp);
static void summary_plot_setup_gui(summary_plot_gui_type * spg);
static gboolean summary_plot_canvas_menu(GtkWidget * widget,
                                         GdkEvent * event);
static summary_plot_type *
summary_plot_create_tab_with_data(summary_plot_gui_type * spg, char *sp_kw,
                                  double xmax, double ymax);
void summary_plot_add_well_tabs(summary_plot_gui_type * spg,
                                char *conf_file);
static void summary_plot_destroy_local(GtkWidget * widget, gpointer data);
static void summary_plot_exit(summary_plot_gui_type * spg);

/***************************************************************
 ***************************************************************/

summary_plot_type *summary_plot_alloc()
{
    summary_plot_type *sp;
    sp = malloc(sizeof *sp);
    sp->list = list_alloc();
    sp->off_charts = false;
    sp->timer_id = -1;
    sp->node = NULL;
    return sp;
}

void summary_plot_free(summary_plot_type * sp)
{
    list_node_type *node, *next_node;

    if (list_get_size(sp->list) != 0) {
        node = list_get_head(sp->list);
        while (node != NULL) {
            summary_plot_member_type *tmp;
            next_node = list_node_get_next(node);
            tmp = list_node_value_ptr(node);
            list_del_node(sp->list, node);
            summary_plot_member_free(tmp);

            node = next_node;
        }
    }

    list_free(sp->list);
    plot_free(sp->item);
    util_safe_free(sp->kw);
    util_safe_free(sp);
}

summary_plot_member_type *summary_plot_member_alloc()
{
    summary_plot_member_type *spm;
    spm = malloc(sizeof *spm);
    spm->last_report_step = 0;
    return spm;
}

void summary_plot_member_free(summary_plot_member_type * spm)
{
    util_safe_free(spm->dir);
    util_safe_free(spm->file);
    util_safe_free(spm);
}

summary_plot_gui_type *summary_plot_gui_alloc()
{
    summary_plot_gui_type *spg;
    spg = malloc(sizeof *spg);
    spg->list = list_alloc();
    return spg;
}

void summary_plot_gui_free(summary_plot_gui_type * spg)
{
    config_free(spg->config);
    util_safe_free(spg);
}

void summary_plot_get_ecl_data(summary_plot_member_type * spm, char ***sfl,
                               char **header_file, int *files)
{
    char data_file[PATH_MAX];
    char *path;
    char *base;
    char *header;
    bool fmt_file, unified;
    char **summary_file_list;
    int j;

    snprintf(data_file, PATH_MAX, "%s/%s", spm->dir, spm->file);
    util_alloc_file_components(data_file, &path, &base, NULL);
    ecl_util_alloc_summary_files(path, base, &header,
                                 &summary_file_list, &j);
                                 
    if (sfl)
        *sfl = summary_file_list;
    else
        util_free_stringlist(summary_file_list, j);

    if (header_file)
        *header_file = header;
    else
        util_safe_free(header);

    *files = j;
    util_safe_free(path);
    util_safe_free(base);
}

void summary_plot_add_ensamble_member(summary_plot_type * sp,
                                      char *dir, char *file)
{
    summary_plot_member_type *spm;

    spm = summary_plot_member_alloc();
    spm->dir = strdup(dir);
    spm->file = strdup(file);
    summary_plot_append_textbox(sp->spg,
                                "Adding ensemble %s/%s to plot %p with keyword '%s'",
                                dir, file, sp->item, sp->kw);
    summary_plot_get_ecl_data(spm, NULL, NULL, &spm->files);
    list_append_ref(sp->list, spm);
}


/* 
   This is the timout function, it runs in a discrete manner (and infinitly many times) 
   at the interval TIMEOUT. GTK takes care of this with it's gtk_main() loop. 
*/
gboolean summary_plot_timout(gpointer data)
{
    list_node_type *node, *next_node;
    list_node_type *node2, *next_node2;
    summary_plot_gui_type *spg = data;
    double x_max, y_max;
    bool show = false;

    summary_plot_append_textbox(spg,
                                "Entered timer at %d msec interval\nLooking for new summaryfiles ...",
                                TIMEOUT);
    /*
       This code section does the following:
       while: "Iterate trough list of plots"
       while: "Iterate trough list of added ensemble members"
       - Gather summarydata
       - Plot either a list of linesegments OR 
       plot from x1,y1 -> x2,y2.
       - Collect new maximal points information. 
       end
       If we have new maximal points, plot a new tab and remove old.
       end
     */
    node2 = list_get_head(spg->list);
    while (node2 != NULL) {
        PLFLT *x, *y;
        bool flag = false;
        summary_plot_type *sp;
        next_node2 = list_node_get_next(node2);
        sp = list_node_value_ptr(node2);

        node = list_get_head(sp->list);
        while (node != NULL) {
            PLFLT diff_day;
            time_t t, t0;
            char **summary_file_list;
            char *header_file;
            summary_plot_member_type *tmp;
            ecl_sum_type *ecl_sum;
            int report_step, first_report_step, last_report_step;
            int j;

            next_node = list_node_get_next(node);
            tmp = list_node_value_ptr(node);
            summary_plot_get_ecl_data(tmp, &summary_file_list,
                                      &header_file, &j);

            if (tmp->files != j || tmp->last_report_step == 0) {
                ecl_sum =
                    ecl_sum_fread_alloc(header_file, j, (const char **)
                                        summary_file_list, true, true);
                ecl_sum_get_report_size(ecl_sum, &first_report_step,
                                        &last_report_step);
                x = malloc(sizeof(PLFLT) * (last_report_step + 1));
                y = malloc(sizeof(PLFLT) * (last_report_step + 1));
                for (report_step = first_report_step;
                     report_step <= last_report_step; report_step++) {
                    if (ecl_sum_has_report_nr(ecl_sum, report_step)) {
                        int day, month, year;
                        util_set_date_values(ecl_sum_get_sim_time
                                             (ecl_sum, report_step), &day,
                                             &month, &year);

                        if (report_step == first_report_step)
                            plot_util_get_time(day, month, year, &t0,
                                               NULL);
                        plot_util_get_time(day, month, year, &t, NULL);
                        plot_util_get_diff(&diff_day, t, t0);
                        x[report_step] = (PLFLT) diff_day;
                        y[report_step] = (PLFLT)
                            ecl_sum_get_general_var(ecl_sum, report_step,
                                                    sp->kw);
                    }
                }

                {
                    plot_dataset_type *d;

                    d = plot_dataset_alloc();
                    plot_dataset_set_data(d, x, y, last_report_step,
                                          RED, LINE);
                    /* If this is the first time - we want to plot up to the current step */
                    if (tmp->last_report_step == 0) {
                        plot_dataset(sp->item, d);
                        summary_plot_append_textbox(spg,
                                                    "Plotting dataset: (%s @ %s), until report step %d.",
                                                    sp->kw, tmp->dir,
                                                    last_report_step - 1);
                    } else {
                        /* Join lines between all the next points (steps) */
                        plot_dataset_join(sp->item, d,
                                          tmp->last_report_step - 1,
                                          last_report_step - 1);
                        summary_plot_append_textbox(spg,
                                                    "Plotting dataset segment: (%s @ %s), step %d -> %d",
                                                    sp->kw, tmp->dir,
                                                    tmp->last_report_step -
                                                    1,
                                                    last_report_step - 1);
                    }
                    plot_dataset_get_extrema(d, &x_max, &y_max, NULL,
                                             NULL);
                    if (x_max > sp->x_max) {
                        sp->x_max = x_max;
                        flag = true;
                    }
                    if (y_max > sp->y_max) {
                        sp->y_max = y_max;
                        flag = true;
                    }
                    plot_dataset_free(d);
                }
                util_safe_free(x);
                util_safe_free(y);
                tmp->last_report_step = last_report_step;
                tmp->files = j;
                ecl_sum_free(ecl_sum);
            }
            util_free_stringlist(summary_file_list, j);
            util_safe_free(header_file);
            node = next_node;
        }

        /* If we have found a new maxima in some of the datasets,
           1. Remove the old tab
           2. Create new tab with fresh resized scales and replot!
           3. Free and remove old tab
         */
        if (flag) {
            summary_plot_type *sp_new;
            int i;

            sp_new =
                summary_plot_create_tab_with_data(sp->spg, sp->kw,
                                                  sp->x_max, sp->y_max);
            fprintf
                (stderr,
                 "ID[%d] Adding a new tab %p with kw: %s and new xmax: %f, ymax: %f\n",
                 plot_get_stream(sp_new->item), sp_new->item, sp_new->kw,
                 sp_new->x_max, sp_new->y_max);
            summary_plot_append_textbox(spg,
                                        "One or more datasets in plot %s went off axis, creating new tab with resized axis!",
                                        sp_new->kw);
            i = gtk_notebook_page_num(GTK_NOTEBOOK(sp->spg->nb),
                                      GTK_WIDGET(plot_get_canvas
                                                 (sp->item)));
            list_del_node(spg->list, sp->node);
            summary_plot_free(sp);
            gtk_notebook_remove_page(GTK_NOTEBOOK(sp_new->spg->nb), i);
            show = true;
        }
        node2 = next_node2;
    }

    if (show)
        gtk_widget_show_all(GTK_WIDGET(spg->win));

    return true;
}

char *summary_plot_get_timestamp()
{
    struct tm *ptr;
    time_t tm;
    char str[10];

    tm = time(NULL);
    ptr = localtime(&tm);
    strftime(str, sizeof(str), "%T", ptr);
    return strdup(str);
}

void summary_plot_append_textbox(summary_plot_gui_type * spg,
                                 const char *str, ...)
{
    GtkTextIter iter;
    char buf[512 + 10];
    char va_buf[512];
    char *timestamp;
    va_list ap;

    if (!spg->buffer)
        return;

    va_start(ap, str);
    vsprintf(va_buf, str, ap);
    va_end(ap);
    timestamp = summary_plot_get_timestamp();
    snprintf(buf, sizeof(buf), "[%s] %s\n", timestamp, va_buf);
    gtk_text_buffer_get_end_iter(spg->buffer, &iter);
    gtk_text_buffer_insert(spg->buffer, &iter, buf, -1);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(spg->text),
                                 gtk_text_buffer_get_mark(spg->buffer,
                                                          "insert"), 0.0,
                                 FALSE, 0.0, 0.0);
    util_safe_free(timestamp);
}


config_parser_type *summary_plot_init_config(const char *config_file)
{
    config_parser_type *config;

    config = config_alloc(false);
    config_init_item(config, "DATA_FILE", 0, NULL, true, false, 0, NULL, 1,
                     1, NULL);
    config_init_item(config, "ECL_STORE_PATH", 0, NULL, true, false, 0,
                     NULL, 1, 1, NULL);
    config_init_item(config, "ECLBASE", 0, NULL, true, false, 0, NULL, 1,
                     1, NULL);
    config_init_item(config, "ECL_STORE", 0, NULL, true, false, 0, NULL, 1,
                     -1, NULL);
    config_init_item(config, "WELL", 0, NULL, true, true, 2, NULL, 1,
                     -1, NULL);
    config_parse(config, config_file, ECL_COM_KW);

    {
        /* Change Path to your enkf config dir */
        char *path;
        util_alloc_file_components(config_file, &path, NULL, NULL);
        util_chdir(path);
        util_safe_free(path);
    }
    return config;
}

void summary_plot_initialize_ensembles(summary_plot_type * sp)
{
    /* Collecting data about the ensembles and add them to the plot */
    config_schema_item_type *config_item;
    const char **argv_list;
    const char *ecl_store_path;
    const char *ecl_base;
    char *ecl_store_path_buf;
    char *ecl_base_buf;
    char *base_with_ext;
    int n, i, j;

    ecl_store_path = config_get(sp->spg->config, "ECL_STORE_PATH");
    ecl_base = config_get(sp->spg->config, "ECLBASE");
    config_item = config_get_schema_item(sp->spg->config, "ECL_STORE");
    argv_list = config_item_get_argv(config_item, &n);
    for (i = 1; i < n; i++) {
        if (*argv_list[i] == ',')
            continue;
        if (*argv_list[i] == '-') {
            /* This hack can't handle spaces in the "int-int , int-int" format! */
            for (j = atoi(argv_list[i - 1]);
                 j <= atoi(argv_list[i + 1]); j++) {
                ecl_store_path_buf = malloc(strlen(ecl_store_path) + 1);
                snprintf(ecl_store_path_buf,
                         (int) strlen(ecl_store_path) + 1,
                         ecl_store_path, j);
                ecl_base_buf = malloc(strlen(ecl_base) + 1);
                snprintf(ecl_base_buf, (int) strlen(ecl_base) + 1,
                         ecl_base, j);
                base_with_ext =
                    malloc(strlen(ecl_base_buf) + strlen(ECL_EXT) + 1);
                snprintf(base_with_ext,
                         (int) strlen(ecl_base_buf) + strlen(ECL_EXT) +
                         2, "%s%s", ecl_base_buf, ECL_EXT);
                summary_plot_add_ensamble_member(sp,
                                                 ecl_store_path_buf,
                                                 base_with_ext);
                util_safe_free(ecl_store_path_buf);
                util_safe_free(ecl_base_buf);
                util_safe_free(base_with_ext);
            }
        }
    }
}

void summary_plot_setup_gui(summary_plot_gui_type * spg)
{
    GtkBox *vbox;
    GtkFrame *frame;
    GtkScrolledWindow *sw;

    spg->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_resize(GTK_WINDOW(spg->win), 1152, 768 + 300);
    gtk_container_set_border_width(GTK_CONTAINER(spg->win), 0);
    g_signal_connect(G_OBJECT(spg->win), "destroy",
                     G_CALLBACK(summary_plot_destroy_local), spg);
    vbox = GTK_BOX(gtk_vbox_new(FALSE, 10));
    spg->nb = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_box_pack_start(vbox, GTK_WIDGET(spg->nb), FALSE, FALSE, 0);
    frame = GTK_FRAME(gtk_frame_new(NULL));
    sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    spg->text = gtk_text_view_new();

    gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(sw));
    gtk_container_add(GTK_CONTAINER(sw), spg->text);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 2);
    gtk_container_set_border_width(GTK_CONTAINER(spg->text), 2);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(spg->text), FALSE);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(spg->text),
                                    GTK_JUSTIFY_LEFT);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(spg->text), GTK_WRAP_WORD);
    spg->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(spg->text));
    gtk_box_pack_start(vbox, GTK_WIDGET(frame), TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(spg->win), GTK_WIDGET(vbox));
}

gboolean summary_plot_canvas_menu(GtkWidget * widget, GdkEvent * event)
{
    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *bevent = (GdkEventButton *) event;
        gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
                       bevent->button, bevent->time);
        return true;
    }

    return false;
}

void summary_plot_save_cb(GtkWidget * widget, gpointer data)
{
    summary_plot_type *sp = data;

    printf("Tab page: %s\n", sp->kw);
    printf("=== NOT IMPLEMENTED ===\n");
    /* 
     * Would be nice to just use plcpstream([..]), and plreplot() 
     * in this function, but this does not seem to work properly
     * with the gcw driver! (Even the driver itself does a SEGV 
     * when trying to save the plots, try: ./x01c -dev gcw and then 
     * save an image with the save button)
     *
     * To have a save function a solution is to just keep all the 
     * data in the struct and plot everything to a new image using
     * a new initialized driver.
     *
     * Unfortunately this code was not designed for that purpose, so
     * to grab this data it needs to be stored the summary_plot structure.
     */ 
    widget = NULL;
}

summary_plot_type *summary_plot_create_tab_with_data(summary_plot_gui_type
                                                     * spg, char *sp_kw,
                                                     double xmax,
                                                     double ymax)
{
    /* Setup a plot object and plot the true case */
    summary_plot_type *sp;
    int N;
    PLFLT *x, *y;
    const char *ecl_data_file;
    plot_dataset_type *d;
    GtkWidget *menu;
    GtkWidget *menu_item;

    ecl_data_file = config_get(spg->config, "DATA_FILE");
    sp = summary_plot_alloc();
    sp->spg = spg;
    sp->kw = strdup(sp_kw);
    sp->item = plot_alloc();
    plot_set_window_size(sp->item, 1024, 768);
    plot_initialize(sp->item, NULL, NULL, CANVAS);
    plot_summary_collect_data(&x, &y, &N, ecl_data_file, sp_kw);
    d = plot_dataset_alloc();
    plot_dataset_set_data(d, x, y, N, BLACK, POINT);
    util_safe_free(x);
    util_safe_free(y);
    plot_dataset_add(sp->item, d);

    if ((xmax == 0) || (ymax == 0))
        plot_get_extrema(sp->item, &sp->x_max, &sp->y_max, NULL, NULL);
    else {
        sp->x_max = xmax;
        sp->y_max = ymax;
    }
    /* Check if maxima for either x or y axis is zero */
    assert(sp->x_max != 0 && sp->y_max != 0);
    plot_set_labels(sp->item, "Days", sp_kw, spg->conf_file, BLACK);
    plot_set_viewport(sp->item, 0, sp->x_max, 0, sp->y_max);
    plot_data(sp->item);
    gtk_notebook_append_page(spg->nb,
                             GTK_WIDGET(plot_get_canvas
                                        (sp->item)), gtk_label_new(sp_kw));
    menu = gtk_menu_new();
    menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
    g_signal_connect(G_OBJECT(menu_item), "activate",
                     G_CALLBACK(summary_plot_save_cb), sp);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    gtk_widget_show(menu_item);
    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    gtk_widget_show(menu_item);
    menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate",
                     G_CALLBACK(summary_plot_destroy_local), spg);
    gtk_widget_show(menu_item);
    g_signal_connect_swapped(G_OBJECT(plot_get_canvas(sp->item)), "event",
                             G_CALLBACK(summary_plot_canvas_menu),
                             G_OBJECT(menu));
    gtk_widget_show(menu);

    summary_plot_initialize_ensembles(sp);
    summary_plot_append_textbox(spg,
                                "Adding timer for %p with timeout %d ms",
                                sp->item, TIMEOUT);
    sp->node = list_append_ref(spg->list, sp);
    return sp;
}

void summary_plot_add_well_tabs(summary_plot_gui_type * spg,
                                char *conf_file)
{
    FILE *stream;
    bool at_eof = false;

    spg->conf_file = conf_file;
    stream = util_fopen(conf_file, "r");

    while (!at_eof) {
        int i, tokens;
        int active_tokens;
        char **token_list;
        char *line;

        line = util_fscanf_alloc_line(stream, &at_eof);
        if (line != NULL) {
            util_split_string(line, " \t", &tokens, &token_list);
            active_tokens = tokens;
            for (i = 0; i < tokens; i++) {
                char *comment_ptr = NULL;
                comment_ptr = strstr(token_list[i], ECL_COM_KW);

                if (comment_ptr != NULL) {
                    if (comment_ptr == token_list[i])
                        active_tokens = i;
                    else
                        active_tokens = i + 1;
                    break;
                }
            }

            if (active_tokens > 0) {
                if (!strcmp("WELL", token_list[0])) {
                    for (i = 2; i < tokens; i++) {
                        char buf[128];
                        snprintf(buf, sizeof(buf), "%s:%s",
                                 token_list[i], token_list[1]);
                        summary_plot_create_tab_with_data(spg, buf, 0, 0);
                    }
                }
            }
        }
        util_free_stringlist(token_list, tokens);
        util_safe_free(line);
    }
}


void summary_plot_destroy_local(GtkWidget * widget, gpointer data)
{
    summary_plot_gui_type *spg = data;
    summary_plot_exit(spg);
    gtk_main_quit();
    widget = NULL;
}

void summary_plot_exit(summary_plot_gui_type * spg)
{
    list_node_type *node, *next_node;

    node = list_get_head(spg->list);
    while (node != NULL) {
        summary_plot_type *sp;

        next_node = list_node_get_next(node);
        sp = list_node_value_ptr(node);
        list_del_node(spg->list, node);
        summary_plot_free(sp);
        node = next_node;
    }
    list_free(spg->list);
    summary_plot_gui_free(spg);
}

/***************************************************************
 ***************************************************************/

int main(int argc, char **argv)
{
    summary_plot_gui_type *spg;

    if (argc < 2) {
        fprintf(stderr, "** ERROR ** %s EnKF.conf \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    gtk_init(&argc, &argv);
    spg = summary_plot_gui_alloc();
    summary_plot_setup_gui(spg);

    spg->config = summary_plot_init_config(argv[1]);
    summary_plot_add_well_tabs(spg, argv[1]);

    summary_plot_timout(spg);
    gtk_widget_show_all(spg->win);
    g_timeout_add(TIMEOUT, summary_plot_timout, spg);
    gtk_main();

    return true;
}

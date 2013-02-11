/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'errorbar_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#define LEN 10
#define DATASETS 4

void plot1()
{
    plot_type *item;
    plot_dataset_type *d;

    srand(time(NULL));

    item = plot_alloc();
    plot_initialize(item, "png", "stat.png", NORMAL);
    {
        int i, j;
        PLFLT *y, *x;

        for (j = 0; j <= DATASETS; j++) {
            y = malloc(sizeof(PLFLT));
            x = malloc(sizeof(PLFLT));
            for (i = 0; i <= LEN; i++) {
                if (i > 0) {
                    y = realloc(y, sizeof(PLFLT) * (i + 1));
                    x = realloc(x, sizeof(PLFLT) * (i + 1));
                }
                x[i] = i;
                y[i] = rand() % 100;
            }
            d = plot_dataset_alloc();
            plot_dataset_set_data(d, x, y, LEN, BLACK, POINT);
            plot_dataset_add(item, d);
            util_safe_free(y);
            util_safe_free(x);
        }
    }

    plot_set_labels(item, "x-axis", "y-axis", "Test", BLACK);
    plot_set_viewport(item, 0 - 1, LEN, 0 - 10, 100 + 10);
    plot_data(item);
    plot_errorbar_data(item);
    plot_free(item);
}

void plot2()
{
    plot_type *item;
    plot_dataset_type *d;
    int N = pow(2, 4.5);
    const PLFLT period = 2 * M_PI;
    PLFLT x[N];
    PLFLT y[N];
    int i, j;
    double xmax, ymax, xmin, ymin;

    srand48(time(NULL));

    item = plot_alloc();
    plot_initialize(item, "png", "std.png", NORMAL);
    plot_set_labels(item, "x-axis", "y-axis",
                    "Standard deviation of drand48()", BLACK);

    for (j = 0; j <= DATASETS; j++) {
        for (i = 0; i <= N; i++) {
            x[i] = (i * period) / N;
            y[i] = drand48();
        }
        d = plot_dataset_alloc();
        plot_dataset_set_data(d, x, y, N, BLACK, POINT);
        plot_dataset_add(item, d);
    }

    plot_get_extrema(item, &xmax, &ymax, &xmin, &ymin);
    plot_set_viewport(item, xmin - 0.1, xmax, ymin, ymax);
    plot_data(item);
    plot_std_data(item, true);

    /* Now plot a blue true mean line, for reference - drand48 covers [0, 1) */
    for(i = 0; i <= N; i++) {
        x[i] = (i * period) / N;
        y[i] = 0.5;
    }
    d = plot_dataset_alloc();
    plot_dataset_set_data(d, x, y, N, BLUE, LINE);
    plot_dataset(item, d);
    plot_dataset_free(d);

    plot_free(item);
}

int main()
{
    plot1();
    plot2();

    return 0;
}

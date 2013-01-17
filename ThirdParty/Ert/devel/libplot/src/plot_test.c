/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

int main(int argc, const char **argv)
{
    plot_type *item;
    plot_type *item2;
    plot_dataset_type *d;
    int N = 100;		/* Number of samples */
    const double period = 2 * PI;
    int i;
    PLFLT x[N];
    PLFLT y[N];

    item2 = plot_alloc();
    plot_initialize(item2, "png", "martin.png");

    for (i = 0; i < N; i++) {
	x[i] = i * period / N;
	y[i] = sin(x[i]);
    }
    d = plot_dataset_alloc( false , false);
    plot_dataset_set_data(d, x, y, N, BLUE, LINE);
    plot_dataset_add(item2, d);

    for (i = 0; i < N; i++) {
	x[i] = i * period / N;
	y[i] = cos(x[i]);
    }
    d = plot_dataset_alloc( false , false);
    plot_dataset_set_data(d, x, y, N, CYAN, LINE);
    plot_dataset_add(item2, d);

    /*
     * Create yet another cos, but with another angular frequency (\omega
     * = 3) 
     */
    for (i = 0; i < N; i++) {
	x[i] = i * period / N;
	y[i] = cos(3 * x[i]);
    }
    d = plot_dataset_alloc( false , false);
    plot_dataset_set_data(d, x, y, N, RED, POINT);
    plot_dataset_add(item2, d);

    /*
     * Create a second new plot window, and fill it with only 1 graph 
     */
    item = plot_alloc();
    plot_initialize(item, "jpeg", "plot.jpg");
    for (i = 0; i < N; i++) {
	x[i] = i * period / N;
	y[i] = exp(x[i]);
    }

    d = plot_dataset_alloc( false , false);
    plot_dataset_set_data(d, x, y, N, BLUE, LINE);
    plot_dataset_add(item, d);
    plot_set_labels(item, "x-axis", "y-axis", "f(x) = exp(x)", BROWN);
    
    /* This demonstrates that order doesnt mather when we use 
     * the correct outputstreams, which is handeled by the lib.
     */
    plot_set_labels(item2, "x-axis", "y-axis", "#frHarmonic waves", BLACK);
    plot_set_viewport(item2);
    plot_set_viewport(item);
    plot_data(item);
    plot_free(item);
    plot_data(item2);
    plot_free(item2);

    return 0;
    argc = 0;
    argv = NULL;
}

/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'simple.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <ert/plot/plot.h>
#include <ert/plot/plot_dataset.h>

int main()
{
    plot_type *item;
    const double period = 2 * PI;

    item = plot_alloc();
    plot_set_window_size(item, 640, 480);
    plot_initialize(item, "png", "test.png");

    {
        plot_dataset_type *d;
        int N = pow(2, 10);
        PLFLT x[2 * N];
        PLFLT y[2 * N];
        int i;

        for (i = 0; i <= 2 * N; i++) {
            x[i] = (i - N) / period;
            if (x[i] != 0.0)
                y[i] = sin(PI * x[i]) / (PI * x[i]);
            else
                y[i] = 1.0;
        }
        d = plot_dataset_alloc();
        plot_dataset_set_data(d, x, y, 2 * N, BLUE, LINE);
        plot_dataset_add(item, d);
    }

    plot_set_labels(item, "x-axis", "y-axis", "y = sinc(x)", BLACK);
    plot_set_viewport(item, -period, period, -0.3, 1);
    plot_data(item);
    plot_free(item);

    return 0;
}

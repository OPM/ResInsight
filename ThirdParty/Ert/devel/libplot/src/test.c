/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <fftw3.h>
#include <math.h>

int main()
{
    fftw_complex *in;
    fftw_complex *out;
    fftw_plan p;
    int N = pow(2, 8);
    int i;
    double x[N];
    double y[N];
    double xhat[N];
    double yhat[N];
    const double T = 2 * M_PI;
    int n;
    int n2;
    double tmp;

    in = fftw_malloc(sizeof(fftw_complex) * (N + 1));

    for (i = 0; i <= N; i++) {
	x[i] = i * T / N;
	in[i][0] = 3*cos(8 * x[i]) + 6*cos(16 * x[i]) + 9 * cos(24 * x[i]);
	y[i] = in[i][0];
    }

    out = fftw_malloc(sizeof(fftw_complex) * (N + 1));
    p = fftw_plan_dft_1d(N + 1, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    for (i = 0; i <= N; i++) {
	xhat[i] = (i - N / 2);
	yhat[i] = fabs(out[i][1]) / (double) N;
    }

    n = N;
    n2 = n / 2;
    for (i = 0; i < n2; i++) {
	tmp = xhat[i];
	xhat[i] = xhat[i + n2];
	xhat[i + n2] = tmp;
    }

    {
	plot_type *item;
	plot_dataset_type *d;
	double xmax, ymax, xmin, ymin;

	item = plot_alloc();
	plot_initialize(item, "png", "fft.png", NORMAL);
	d = plot_dataset_alloc();
	plot_dataset_set_data(d, xhat, yhat, N + 1, BLACK, LINE);
	plot_dataset_add(item, d);
	d = plot_dataset_alloc();
	plot_dataset_set_data(d, xhat, yhat, N + 1, BLUE, POINT);
	plot_dataset_add(item, d);
	plot_set_labels(item, "Frequency", "",
			"Spectrum of: f(x) = 3cos(8x) + 6cos(16x) + 9cos(24x)",
			BLACK);
	plot_get_extrema(item, &xmax, &ymax, &xmin, &ymin);
	printf("xmax: %f, ymax %f\n", xmax, ymax);
	plot_set_viewport(item, xmin, xmax, ymin, ymax);

	{
	    PLFLT x_mark[6] = { -24, -16, -8, 8, 16, 24 };
	    PLFLT y_mark[6] = { 0, 0, 0, 0, 0, 0};

	    d = plot_dataset_alloc();
	    plot_dataset_set_data(d, x_mark, y_mark, 6, RED, POINT);
	    plot_dataset(item, d);
	    plot_dataset_free(d);

	}

	plot_data(item);
	plot_free(item);
    }
    fftw_destroy_plan(p);
    fftw_free(out);
    fftw_free(in);

    return 0;
}

/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'statistics.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <math.h>
#include <stdlib.h>

#include <ert/util/util.h>
#include <ert/util/double_vector.h>
#include <ert/util/statistics.h>


double statistics_mean( const double_vector_type * data_vector ) {
  const double * data = double_vector_get_const_ptr( data_vector );
  int size = double_vector_size( data_vector );
  double sum = 0;
  int i;
  for (i=0; i < size; i++)
    sum += data[i];

  return sum / size;
}



double statistics_std( const double_vector_type * data_vector ) {
  const double * data = double_vector_get_const_ptr( data_vector );
  double std = 0;
  double mean = statistics_mean( data_vector );
  int size = double_vector_size( data_vector );
  int i;

  for (i=0; i < size; i++) {
    double d = (data[i] - mean);
    std += d*d;
  }
  
  return sqrt(std / size);
}


 

/**
   Observe that the data vector will be sorted in place. If the vector is
   already sorted, e.g. from a previous call to statistics_empirical_quantile(),
   you can call statistics_empirical_quantile__() directly.  
*/

double statistics_empirical_quantile( double_vector_type * data , double quantile ) {
  double_vector_sort( data );
  return statistics_empirical_quantile__( data , quantile );
}


/**
   This assumes that data has already been sorted, either from a
   previous call to statistics_empirical_quantile( ) or by sorting
   data explicitly with double_vector_sort( data );
*/

double statistics_empirical_quantile__( const double_vector_type * data , double quantile ) {
  if ((quantile < 0) || (quantile > 1.0))
    util_abort("%s: quantile must be in [0,1] \n",__func__);

  {
    const int size = (double_vector_size( data ) - 1);
    if (double_vector_iget( data , 0) == double_vector_iget( data , size))
      /* 
         All elements are equal - and it is impossible to find a meaingful quantile,
         we just return "the value".
      */
      return double_vector_iget( data, 0 );    
    else {
      double value;
      double lower_value;
      double upper_value;
      double real_index;
      double upper_quantile;
      double lower_quantile;
      
      int    lower_index;
      int    upper_index;
      
      
      real_index  = quantile * size;
      lower_index = floor( real_index );
      upper_index = ceil( real_index );
      
      upper_value    = double_vector_iget( data , upper_index );
      lower_value    = double_vector_iget( data , lower_index );

      /* 
         Will iterate in this loop until we have found upper_value !=
         lower_value. As long as we know that now all elements are
         equal (the first test), this is guaranteed to succeed, but of
         course the estimate will not be very meaningful if the sample
         consist of a significant number of equal values.
      */
      while (true) {

        /*1: Try to shift the upper index up. */
        if (upper_value == lower_value) {
          upper_index = util_int_min( size , upper_index + 1);
          upper_value = double_vector_iget( data , upper_index );
        } else 
          break;

        /*2: Try to shift the lower index down. */
        if (upper_value == lower_value) {
          lower_index = util_int_max( 0 , lower_index - 1);
          lower_value = double_vector_iget( data , lower_index );
        } else 
          break;
        
      }
      
      upper_quantile = upper_index * 1.0 / size;
      lower_quantile = lower_index * 1.0 / size;
      /* Linear interpolation: */
      {
        double a = (upper_value - lower_value) / (upper_quantile - lower_quantile);
        
        value = lower_value + a*(quantile - lower_quantile);
        return value;
      }
    }
  }
}

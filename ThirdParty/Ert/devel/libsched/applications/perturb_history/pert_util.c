/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'pert_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <util.h>
#include <time_t_vector.h>
#include <double_vector.h>
#include <stdbool.h>
#include <time.h>
#include <pert_util.h>

static inline int randint() {
  return rand();
}


void rand_dbl(int N , double max , double *R) {
  int i;
  for (i=0; i < N; i++) 
    R[i] = randint() * max / RAND_MAX;
}


double rand_normal(double mean , double std) {
  const double pi = 3.141592653589;
  double R[2];
  rand_dbl(2 , 1.0 , R);
  return mean + std * sqrt(-2.0 * log(R[0])) * cos(2.0 * pi * R[1]);
}


void rand_stdnormal_vector(int size , double *R) {
  int i;
  for (i = 0; i < size; i++)
    R[i] = rand_normal(0.0 , 1.0);
}




/*****************************************************************/

static void set_ts(const time_t_vector_type * time_vector , stringlist_type * stringlist , time_t start_date , time_t end_date , const char * string_value) {
  int i;
  for (i=0; i < time_t_vector_size( time_vector ); i++) {
    time_t t = time_t_vector_iget( time_vector , i );
    if ((t >= start_date) && (t < end_date)) 
      stringlist_iset_copy( stringlist , i , string_value );
  }
}

/**
   File format:

   *           -  12/07/2009   500  577 
   12/07/2009  -  16/09/2009   672  666
   17/09/2009  -      *        100   10%   
   

   1. Both dates can be replaced with '*' - which is implied to mean
      either the start date, or the end date.

   2. The formatting of the data strings is 100% NAZI - no spaces
      allowed.
     
   3. The date intervals are half-open, [date1,date2).

   4. The date lines can overlap - they are applied in line-order.

   5. The last float value (i.e. 577 and 666 on the liness above) can
      be a percent value; that should indicated with '%' immediately
      following the number. The percent number should be in the
      interval [0,100]. The bool vector tsp is updated to indicate
      whether the data should be interpreted as percent or not.
      
*/


static void load_exit( FILE * stream , const char * filename) {

  fprintf(stderr," Something wrong around line:%d of file:%s\n",util_get_current_linenr( stream ) , filename );
  fprintf(stderr," Each line should be:\n  date1 - date2  value\nwhere date should be formatted as 12/06/2003. \n");
  exit(1);

}

   
void fscanf_2ts(const time_t_vector_type * time_vector , const char * filename , stringlist_type * s1 , stringlist_type * s2) {
  time_t start_time       = time_t_vector_get_first( time_vector );
  time_t end_time         = time_t_vector_get_last( time_vector ) + 1;
  
  {
    stringlist_clear( s1 );
    stringlist_clear( s2 );
    for (int i=0; i < time_t_vector_size( time_vector ); i++) { 
      stringlist_append_ref( s1 , NULL );
      stringlist_append_ref( s2 , NULL );
    }
  }
  
  {
    FILE * stream = util_fopen( filename , "r");
    char datestring1[32];
    char datestring2[32];
    char dash;
    char value1string[32];
    char value2string[32];
      
    while (true) {
      int read_count = fscanf(stream , "%s %c %s %s %s" , datestring1 , &dash , datestring2, value1string , value2string);
      if (read_count == 5) {
        bool   OK = true;
        time_t t1      = -1;
        time_t t2      = -1;
        if (util_string_equal( datestring1 , "*"))
          t1 = start_time;
        else
          OK = util_sscanf_date( datestring1 , &t1 );
        
        if (util_string_equal( datestring2 , "*"))
          t2 = end_time;
        else
          OK = (OK && util_sscanf_date( datestring2 , &t2 ));

        if (OK) {
          set_ts( time_vector , s1 , t1 , t2 , value1string );
          set_ts( time_vector , s2 , t1 , t2 , value2string );
        } else 
          load_exit( stream  , filename ); 

      } else {
        if (read_count == EOF)
          break;
        else
          load_exit( stream , filename );
      }
    } 
    fclose( stream );
  }
}


double sscanfp( double base_value , const char * value_string ) {
  double value , parse_value;
  char * error_ptr;
  
  if (value_string == NULL)
    return 0;

  parse_value = strtod( value_string , &error_ptr);
  if (error_ptr[0] == '%') 
    value = base_value * parse_value * 0.01;
  else {
    if (error_ptr[0] == '\0')
      value = parse_value;
    else {
      value = 0;
      util_exit("Failed to parse \'%s\' as valid number. \n",value_string );
    }
  }
  
  return value; 
}


/*****************************************************************/




/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'pert_util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_PERT_UTIL_H
#define ERT_PERT_UTIL_H
#include <double_vector.h>
#include <bool_vector.h>
#include <time_t_vector.h>
#include <stringlist.h>

void                 rand_dbl(int N , double max , double *R);
double               rand_normal(double mean , double std);
void                 rand_stdnormal_vector(int size , double *R);
void                 fscanf_2ts(const time_t_vector_type * time_vector , const char * filename , stringlist_type * s1 , stringlist_type * s2);
double               sscanfp( double base_value , const char * value_string );


#endif

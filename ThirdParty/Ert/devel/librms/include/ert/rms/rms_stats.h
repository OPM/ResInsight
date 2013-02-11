/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_stats.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __RMS_STATS_H__
#define __RMS_STATS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/rms/rms_tagkey.h>

void rms_stats_mean_std(rms_tagkey_type * , rms_tagkey_type * , const char * , int , const char ** , bool);
void rms_stats_update_ens(const char * , const char *, const char **, const char *, int , const double **);

#ifdef __cplusplus
}
#endif
#endif

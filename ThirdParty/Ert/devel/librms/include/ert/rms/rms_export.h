/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_export.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __RMS_EXPORT_H__
#define __RMS_EXPORT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw.h>

  
void rms_export_roff_from_keyword(const char *filename, ecl_grid_type *ecl_grid, 
    ecl_kw_type **ecl_kw, int size);



#ifdef __cplusplus
}
#endif
#endif


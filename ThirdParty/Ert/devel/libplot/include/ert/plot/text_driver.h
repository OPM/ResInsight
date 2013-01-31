/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'text_driver.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __TEXT_DRIVER_H__
#define __TEXT_DRIVER_H__



plot_driver_type *  text_driver_alloc(void * init_arg);
char             *  text_alloc_filename( const char * plot_path , const char * label);


#endif

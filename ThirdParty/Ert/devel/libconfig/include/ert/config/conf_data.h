/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'conf_data.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONF_DATA_H__
#define __CONF_DATA_H__
#include <stdbool.h>
#include <time.h>

typedef enum {
              DT_STR,
              DT_INT,
              DT_POSINT,
              DT_FLOAT,
              DT_POSFLOAT,
              DT_FILE,
              DT_EXEC,
              DT_FOLDER,
              DT_DATE
              } dt_enum;

dt_enum conf_data_get_dt_from_string(
  const char * str);

bool conf_data_string_is_dt(
  const char * str);

const char * conf_data_get_dt_name_ref(
  dt_enum dt);

bool conf_data_validate_string_as_dt_value(
  dt_enum      dt,
  const char * str);

int conf_data_get_int_from_string(
  dt_enum      dt,
  const char * str);

double conf_data_get_double_from_string(
  dt_enum      dt,
  const char * str);

time_t conf_data_get_time_t_from_string(
  dt_enum      dt,
  const char * str);

#endif

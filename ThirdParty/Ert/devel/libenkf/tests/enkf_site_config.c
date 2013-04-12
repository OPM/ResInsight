/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_site_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>

#include <ert/config/config.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/site_config.h>


#include <ert/enkf/site_config.h>




#define INCLUDE_KEY "INCLUDE"
#define DEFINE_KEY  "DEFINE"


void test_empty() {
  site_config_type * site_config = site_config_alloc_empty();
  site_config_free( site_config );
}


void test_init(const char * config_file) {
  site_config_type * site_config = site_config_alloc_empty();
  config_type * config = config_alloc();

  site_config_add_config_items( config , true );
  if (!config_parse(config , config_file , "--" , INCLUDE_KEY , DEFINE_KEY , CONFIG_UNRECOGNIZED_WARN , true))
    test_error_exit("Parsing site config file:%s failed \n",config_file );

  if (!site_config_init( site_config , config ))
    test_error_exit("Loading site_config from config failed\n");
  
  config_free( config );
  site_config_free( site_config );
}


int main(int argc , char ** argv) {
  const char * site_config_file = argv[1];
  test_empty();
  test_init( site_config_file );
    
  exit(0);
}


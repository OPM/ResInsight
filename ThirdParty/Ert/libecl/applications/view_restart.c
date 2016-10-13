/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'view_restart.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ecl_kw.h>
#include <ecl_file.h>
#include <ecl_grid.h>
#include <util.h>



int main(int argc , char ** argv) {
  if (argc < 3)
    util_exit("Usage: <BASE|GRID>  PRESSURE:10,5,17  SWAT:10,5,7   ...\n");
  {
// Broken ...    ecl_grid_type * ecl_grid = NULL;
// Broken ...    int num_restart_files;
// Broken ...    char ** restart_files;
// Broken ...    {
// Broken ...      const char * first_arg = argv[1];
// Broken ...      char * base;
// Broken ...      char * path;
// Broken ...      ecl_file_enum file_type;
// Broken ...      
// Broken ...      ecl_util_get_file_type(first_arg , &file_type , NULL , NULL);
// Broken ...      if (file_type == ecl_grid_file || file_type == ecl_egrid_file)
// Broken ...   ecl_grid = ecl_grid_alloc( first_arg , true );
// Broken ...      
// Broken ...      util_alloc_file_components(first_arg , &path , &base , NULL);
// Broken ...      
// Broken ...      ecl_util_alloc_restart_files(path , base , &restart_files , &num_restart_files , NULL , NULL);
// Broken ...      ecl_util_get_file_type(restart_files[0] , &file_type , NULL , NULL);
// Broken ...      {
// Broken ...   const char ** arg_list = (const char **) &argv[2];
// Broken ...   char       ** kw_list;
// Broken ...   int         * index_list;
// Broken ...   int num_kw = argc - 2;
// Broken ...   ecl_fstate_type * fstate = ecl_fstate_fread_alloc(num_restart_files , (const char **) restart_files , file_type , true , true);
// Broken ...   int num_blocks = ecl_fstate_get_size(fstate);
// Broken ...   int iblock;
// Broken ...
// Broken ...   /* 
// Broken ...      Finding the indices ... 
// Broken ...   */
// Broken ...   kw_list    = util_malloc(num_kw * sizeof * kw_list    , __func__);
// Broken ...   index_list = util_malloc(num_kw * sizeof * index_list , __func__); 
// Broken ...   {
// Broken ...     char ** tokens;
// Broken ...     int     num_tokens;
// Broken ...     for (int ikw = 0; ikw < num_kw; ikw++) {
// Broken ...       util_split_string(arg_list[ikw] , ":" , &num_tokens , &tokens);
// Broken ...       if (num_tokens != 2)
// Broken ...         util_exit("Failed to parse \"%s\" as KEWYORD:INDEX \n", arg_list[ikw]);
// Broken ...       kw_list[ikw] = util_alloc_string_copy( tokens[0] );
// Broken ...       {
// Broken ...         int   *ijk , num_coord;
// Broken ...         ijk = util_sscanf_alloc_active_list( tokens[1] , &num_coord );
// Broken ...         if (ecl_grid == NULL) {
// Broken ...           if (num_coord != 1)
// Broken ...             util_exit("Failed to extract one integer from: %s \n",tokens[1]);
// Broken ...           else
// Broken ...             index_list[ikw] = ijk[0];
// Broken ...         } else {
// Broken ...           if (num_coord == 1) 
// Broken ...             index_list[ikw] = ecl_grid_get_active_index_from_global(ecl_grid , ijk[0]);
// Broken ...           else if (num_coord == 3) {
// Broken ...             int i = ijk[0] - 1;
// Broken ...             int j = ijk[1] - 1;
// Broken ...             int k = ijk[2] - 1;
// Broken ...             
// Broken ...             index_list[ikw] = ecl_grid_get_active_index(ecl_grid , i,j,k);
// Broken ...           } else 
// Broken ...             util_exit("Failed to parse \"%s\" as one or three integers.\n",tokens[1]);
// Broken ...         }
// Broken ...         free(ijk);
// Broken ...       }
// Broken ...         
// Broken ...
// Broken ...       index_list[ikw] = 100; /* ... */
// Broken ...       util_free_stringlist(tokens , num_tokens);
// Broken ...     }
// Broken ...   }
// Broken ...
// Broken ...   for (iblock = 0; iblock  < num_blocks; iblock++) {
// Broken ...     ecl_block_type * ecl_block = ecl_fstate_iget_block( fstate , iblock);
// Broken ...     time_t   sim_time          = ecl_block_get_sim_time(ecl_block);    
// Broken ...     int      day,month,year , report_step;
// Broken ...     double   sim_days;
// Broken ...     
// Broken ...     sim_days = ecl_block_get_sim_days(ecl_block);
// Broken ...     report_step = ecl_block_get_report_nr( ecl_block );
// Broken ...     util_set_date_values( sim_time , &day , &month , &year);
// Broken ...     printf("%04d   %02d/%02d/%04d   %9.3f  ",report_step , day , month , year , sim_days );         
// Broken ...     
// Broken ...     for (int ikw = 0; ikw < num_kw; ikw++) {
// Broken ...       ecl_kw_type * ecl_kw = ecl_block_iget_kw(ecl_block , kw_list[ikw] , 0);
// Broken ...       printf(" %14.6f ",   ecl_kw_iget_float(ecl_kw , index_list[ikw]));
// Broken ...     }
// Broken ...     printf("\n");
// Broken ...   }
// Broken ...   ecl_fstate_free(fstate);
// Broken ...      }
// Broken ...    }
}
}

/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'summary2csv2.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_sum.h>





static void fprintf_line( const ecl_sum_type * ecl_sum , const ecl_sum_fmt_type * fmt , const char * well , int time_index , const stringlist_type * var_list , FILE * stream) {
  /* WELL */
  fprintf(stream , fmt->header_fmt , well);
  fprintf(stream , fmt->sep );

  /* DAYS */
  fprintf(stream , fmt->days_fmt , ecl_sum_iget_sim_days(ecl_sum , time_index));
  fprintf(stream , fmt->sep );

  /* DATE */
  {  
    struct tm ts;
    const int DATE_STRING_LENGTH = 128;
    char * date_string            = util_malloc( DATE_STRING_LENGTH * sizeof * date_string);
    time_t sim_time = ecl_sum_iget_sim_time(ecl_sum , time_index );
    util_localtime( &sim_time , &ts);
    strftime( date_string , DATE_STRING_LENGTH - 1 , fmt->date_fmt , &ts);
    fprintf(stream , date_string );
    free( date_string );
  }

  {
    int ivar;
    for (ivar = 0; ivar < stringlist_get_size( var_list ); ivar++) {
      const char * var = stringlist_iget( var_list , ivar );
      double value = 0;
      if (ecl_sum_has_well_var( ecl_sum , well , var )) 
        value = ecl_sum_get_well_var( ecl_sum , time_index , well , var );
      else
        fprintf(stderr,"Missing variable:%s for well:%s - substituting 0.0 \n",var , well);
      
      fprintf(stream , fmt->sep );
      fprintf(stream , fmt->value_fmt , value );
    }
    fprintf( stream , fmt->newline );
  }
}



int main(int argc , char ** argv) {
  {
    ecl_sum_fmt_type fmt;
    bool           include_restart = true;
    int            arg_offset      = 1;  
    
    if (argc != 2) {
      printf("You must supply the name of a case as:\n\n   summary2csv.exe  ECLIPSE_CASE\n\nThe case can optionally contain a leading path component.\n");
      exit(1);
    }

    {
      char         * data_file = argv[arg_offset];
      ecl_sum_type * ecl_sum;
      stringlist_type * var_list = stringlist_alloc_new();

      stringlist_append_ref( var_list , "WOPR" );
      stringlist_append_ref( var_list , "WOPT" );
      stringlist_append_ref( var_list , "WGPR" );
      stringlist_append_ref( var_list , "WGPT" );
      stringlist_append_ref( var_list , "WWPR" );
      stringlist_append_ref( var_list , "WWPT" );

    
      ecl_sum_fmt_init_csv( &fmt );
      ecl_sum = ecl_sum_fread_alloc_case__( data_file , ":" , include_restart);
      if (ecl_sum != NULL) {
        char * csv_file = util_alloc_filename( NULL , ecl_sum_get_base(ecl_sum) , "txt");  // Will save to current path; can use ecl_sum_get_path() to save to target path instead.
        FILE * stream = util_fopen( csv_file , "w");
        
        stringlist_type * well_list = ecl_sum_alloc_well_list( ecl_sum , NULL );
        stringlist_type * key_list = stringlist_alloc_new( );

        fprintf(stream , fmt.header_fmt , "WELLNAME");

        fprintf(stream , fmt.sep );
        fprintf(stream , fmt.header_fmt , "DAYS");

        fprintf(stream , fmt.sep );
        fprintf(stream , fmt.header_fmt , "DATES");
        
        {
          int ivar;
          for (ivar = 0; ivar < stringlist_get_size( var_list ); ivar++) {
            const char * var = stringlist_iget( var_list , ivar );
            fprintf(stream , fmt.sep );
            fprintf(stream , fmt.header_fmt , var );
          }
          fprintf(stream , "\n");
        }
        
        {
          int iw;
          for (iw = 0; iw < stringlist_get_size( well_list ); iw++) {
            const char * well = stringlist_iget( well_list , iw );
            if (ecl_sum_is_oil_producer( ecl_sum , well )) {
              int time_index;
              for (time_index = 0; time_index < ecl_sum_get_data_length( ecl_sum ); time_index++) 
                fprintf_line( ecl_sum , &fmt , well , time_index , var_list , stream);
            }           
          }
        }

        stringlist_free( well_list );
        stringlist_free( key_list );
        ecl_sum_free(ecl_sum);
        fclose( stream );
        free( csv_file );
      } else 
        fprintf(stderr,"summary2csv2: No summary data found for case:%s\n", data_file );
      
      stringlist_free( var_list );
    }
  }
}

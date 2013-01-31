/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_dates.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <time.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_util.h>

#include <ert/sched/sched_util.h>
#include <ert/sched/sched_kw_dates.h>
#include <ert/sched/sched_types.h>
#include <ert/sched/sched_time.h>


#define DATES_FMT          "  %d \'%s\' %4d  /  \n"    // The format string used when writing dates the arguments are (day , month_string , year).


struct sched_kw_dates_struct {
  vector_type         * time_list;
};


/*****************************************************************/



static sched_kw_dates_type * sched_kw_dates_alloc_empty()
{
  sched_kw_dates_type *dates = util_malloc(sizeof *dates);
  dates->time_list           = vector_alloc_new();
  return dates;
}



static const char * get_month_string_from_int(int month_nr)
{
  switch(month_nr)
  {
    case(1):  return "JAN";
    case(2):  return "FEB";
    case(3):  return "MAR";
    case(4):  return "APR";
    case(5):  return "MAY";
    case(6):  return "JUN";
    case(7):  return "JUL";
    case(8):  return "AUG";
    case(9):  return "SEP";
    case(10): return "OCT";
    case(11): return "NOV";
    case(12): return "DEC";
    default:
      util_abort("%s: Internal error - %i is not a month nr.\n",__func__,month_nr);
      return "ERR\0";
  }

}


static time_t parse_time_t(const char * day_string , const char * month_string , const char * year_string) {
  int mday , month , year;
  time_t time = -1;

  month = ecl_util_get_month_nr(month_string);
  if (month < 0)
    util_abort("%s: failed to interpret:%s a month name \n",__func__ , month_string );

  if (util_sscanf_int(day_string , &mday) && util_sscanf_int(year_string , &year))
    time = util_make_date(mday , month , year);
  else 
    util_abort("%s: fatal error when extracting date from:%s %s %s \n", __func__, day_string , month_string , year_string);

  return time;
}









/*****************************************************************/


sched_kw_dates_type * sched_kw_dates_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_dates_type * kw = sched_kw_dates_alloc_empty();
  int eokw                 = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false, 0 , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      if (stringlist_get_size( line_tokens ) == 3) {
        const char * day_string   = stringlist_iget( line_tokens , 0 );
        const char * month_string = stringlist_iget( line_tokens , 1 );
        const char * year_string  = stringlist_iget( line_tokens , 2 );

        time_t date = parse_time_t( day_string , month_string , year_string );
        sched_time_type * time_node = sched_time_alloc( date , 0 , DATES_TIME );
        vector_append_owned_ref( kw->time_list , time_node , sched_time_free__ );
      } else {
        stringlist_fprintf( line_tokens , "  " , stdout );
        util_abort("%s: malformed DATES keyword\n",__func__);
      }
      stringlist_free( line_tokens );
    } 
    
  } while (!eokw);
  return kw;
}




void sched_kw_dates_fprintf(const sched_kw_dates_type *kw , FILE *stream) {
  fprintf(stream,"DATES\n");
  {
    int i;
    for (i=0; i < vector_get_size( kw->time_list ); i++) {
      const sched_time_type * time_node = vector_iget_const( kw->time_list , i );
      if (sched_time_get_type( time_node ) == DATES_TIME) {
        time_t date          = sched_time_get_date( time_node );
        int day, month, year;
        util_set_date_values(date, &day, &month, &year);
        fprintf(stream , DATES_FMT , day, get_month_string_from_int(month), year );
      } else 
        util_abort("%s: internal type fuckup \n",__func__);
    }
    fprintf(stream , "/\n\n");
  }
}



void sched_kw_dates_free(sched_kw_dates_type * kw) {
  vector_free(kw->time_list);
  free(kw);
}



int sched_kw_dates_get_size(const sched_kw_dates_type * kw)
{
  return vector_get_size(kw->time_list);
}



sched_kw_dates_type * sched_kw_dates_alloc_from_time_t(time_t date)
{
  sched_kw_dates_type * kw        = sched_kw_dates_alloc_empty();
  sched_time_type     * time_node = sched_time_alloc( date , 0 , DATES_TIME );
  vector_append_owned_ref(kw->time_list , time_node , sched_time_free__);
  return kw;
}



time_t sched_kw_dates_iget_date(const sched_kw_dates_type * kw, int i)
{
  const sched_time_type * time_node = vector_iget_const( kw->time_list , i );
  return sched_time_get_date( time_node );
}




sched_kw_dates_type * sched_kw_dates_copyc(const sched_kw_dates_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}




/***********************************************************************/



KW_IMPL(dates)
     

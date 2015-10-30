/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_util.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __ECL_UTIL_H__
#define __ECL_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <time.h>

#include <ert/util/stringlist.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/parser.h>


typedef enum { ECL_OTHER_FILE           = 0   ,
               ECL_RESTART_FILE         = 1   ,
               ECL_UNIFIED_RESTART_FILE = 2   ,
               ECL_SUMMARY_FILE         = 4   ,
               ECL_UNIFIED_SUMMARY_FILE = 8   ,
               ECL_SUMMARY_HEADER_FILE  = 16  ,
               ECL_GRID_FILE            = 32  ,
               ECL_EGRID_FILE           = 64  ,
               ECL_INIT_FILE            = 128 ,
               ECL_RFT_FILE             = 256 ,
               ECL_DATA_FILE            = 512 } ecl_file_enum;



  /*
    This enum enumerates the four different ways summary and restart information
    can be stored.
  */


  typedef enum { ECL_INVALID_STORAGE       = 0,
                 ECL_BINARY_UNIFIED        = 1,
                 ECL_FORMATTED_UNIFIED     = 2,
                 ECL_BINARY_NON_UNIFIED    = 4,
                 ECL_FORMATTED_NON_UNIFIED = 8} ecl_storage_enum;

/*
  Character data in ECLIPSE files comes as an array of fixed-length
  string. Each of these strings is 8 characters long. The type name,
  i.e. 'REAL', 'INTE', ... , come as 4 character strings.
*/

#define ECL_STRING_LENGTH 8
#define ECL_TYPE_LENGTH   4
#define ECL_KW_HEADER_DATA_SIZE   ECL_STRING_LENGTH + ECL_TYPE_LENGTH + 4
#define ECL_KW_HEADER_FORTIO_SIZE ECL_KW_HEADER_DATA_SIZE + 8

/*****************************************************************/
/*
   Observe that these type identidiers are (ab)used in both the rms and
   ert/enkf libraries in situations where ECLIPSE is not at all involved.
*/

typedef enum {
  ECL_CHAR_TYPE   = 0,
  ECL_FLOAT_TYPE  = 1,
  ECL_DOUBLE_TYPE = 2,
  ECL_INT_TYPE    = 3,
  ECL_BOOL_TYPE   = 4,
  ECL_MESS_TYPE   = 5
} ecl_type_enum;

#define ECL_TYPE_ENUM_DEFS {.value = 0 , .name = "ECL_CHAR_TYPE"}, \
{.value = 1 , .name = "ECL_FLOAT_TYPE"} ,                          \
{.value = 2 , .name = "ECL_DOUBLE_TYPE"},                          \
{.value = 3 , .name = "ECL_INT_TYPE"},                             \
{.value = 4 , .name = "ECL_BOOL_TYPE"},                            \
{.value = 5 , .name = "ECL_MESS_TYPE"}

#define ECL_TYPE_ENUM_SIZE 6



/*
  The libecl library has been built and tested 99.5% with ECLIPSE100
  as context, but in thye gravity code there is some very limited
  functionality related to ECLIPSE100 versus ECLIPSE300 functionality.
*/

typedef enum {
  ECLIPSE_UNDEFINED = 0,
  ECLIPSE100        = 1,
  ECLIPSE300        = 2
} ecl_version_enum;

/*
  Observe that the numerical enum VALUES matches those found in item
  14 in the INTEHEAD keyword in the ECLIPSE INIT files; i.e. the
  distribution of numerical values 1,2,4 can NOT BE CHANGED.

  The function ecl_util_get_phase_name() can be used to lookup a
  string name from an enum value.

  The phases in a simulation will typically be a sum of these
  fundamental phases, and represented as an integer.
*/

typedef enum {
  ECL_OIL_PHASE   = 1,
  ECL_GAS_PHASE   = 2,
  ECL_WATER_PHASE = 4
} ecl_phase_enum;

#define ECL_PHASE_ENUM_DEFS {.value = 1 , .name = "ECL_OIL_PHASE"}, {.value = 2 , .name = "ECL_GAS_PHASE"} , {.value = 4 , .name = "ECL_WATER_PHASE"}
#define ECL_PHASE_ENUM_SIZE 3


typedef enum {
  ERT_ECL_METRIC_UNITS = 0,
  ERT_ECL_FIELD_UNITS  = 1,
  ERT_ECL_LAB_UNITS    = 2
} ert_ecl_unit_enum;

#define ECL_UNIT_ENUM_DEFS {.value = 0 , .name = "ECL_METRIC_UNITS"}, {.value = 1 , .name = "ECL_FIELD_UNITS"} , {.value = 2 , .name = "ECL_LAB_UNITS"}
#define ECL_UNIT_ENUM_SIZE 3


// For unformatted files:
#define ECL_BOOL_TRUE_INT         -1   // Binary representation: 11111111  11111111  11111111  1111111
#define ECL_BOOL_FALSE_INT         0   // Binary representation: 00000000  00000000  00000000  0000000
#define ECL_COMMENT_STRING       "--"
#define ECL_COMMENT_CHAR         '-'   // Need to consecutive to make an ECLIPSE comment
#define ECL_DATA_TERMINATION      "/"

int              ecl_util_get_sizeof_ctype_fortio(ecl_type_enum ecl_type);
int              ecl_util_get_sizeof_ctype(ecl_type_enum );
ecl_type_enum    ecl_util_get_type_from_name( const char * type_name );
const char     * ecl_util_get_type_name( ecl_type_enum ecl_type );

/*****************************************************************/
bool            ecl_util_unified_file(const char *filename);
const char    * ecl_util_file_type_name( ecl_file_enum file_type );
char          * ecl_util_alloc_base_guess(const char *);
int             ecl_util_filename_report_nr(const char *);
ecl_file_enum   ecl_util_get_file_type(const char * , bool * , int * );
ecl_file_enum   ecl_util_inspect_extension(const char * ext , bool *_fmt_file, int * _report_nr);
char          * ecl_util_alloc_filename(const char * /* path */, const char * /* base */, ecl_file_enum , bool /* fmt_file */ , int /*report_nr*/);
char          * ecl_util_alloc_exfilename(const char * /* path */, const char * /* base */, ecl_file_enum , bool /* fmt_file */ , int /*report_nr*/);
void            ecl_util_memcpy_typed_data(void *, const void * , ecl_type_enum , ecl_type_enum , int );
void            ecl_util_escape_kw(char * kw);
bool            ecl_util_alloc_summary_files(const char * , const char * , const char * , char ** , stringlist_type * );
void            ecl_util_alloc_summary_data_files(const char * path , const char * base , bool fmt_file , stringlist_type * filelist);
void            ecl_util_alloc_restart_files(const char *  , const char *  , char *** , int *  , bool * , bool *);
time_t          ecl_util_get_start_date(const char * );
int             ecl_util_get_num_cpu(const char * data_file);
bool            ecl_util_fmt_file(const char * filename , bool * __fmt_file);
char          * ecl_util_alloc_exfilename_anyfmt(const char * path, const char * base , ecl_file_enum file_type , bool start_fmt , int report_nr);
int             ecl_util_get_month_nr(const char * month_name);
int             ecl_util_fname_report_cmp(const void *f1, const void *f2);
time_t          ecl_util_make_date(int mday , int month , int year);
time_t          ecl_util_make_date__(int mday , int month , int year, int * year_offset);
ert_ecl_unit_enum   ecl_util_get_unit_set(const char * data_file);

bool            ecl_util_valid_basename_fmt( const char * basename_fmt );
bool            ecl_util_valid_basename( const char * basename );
const char *    ecl_util_get_phase_name( ecl_phase_enum phase );
const char *    ecl_util_file_enum_iget( int index, int * value);

int             ecl_util_select_filelist( const char * path , const char * base , ecl_file_enum file_type , bool fmt_file , stringlist_type * filelist);
void            ecl_util_append_month_range( time_t_vector_type * date_list , time_t start_date , time_t end_date , bool force_append_end);
void            ecl_util_init_month_range( time_t_vector_type * date_list , time_t start_date , time_t end_date);
void            ecl_util_set_date_values(time_t t , int * mday , int * month , int * year);

#ifdef __cplusplus
}
#endif
#endif

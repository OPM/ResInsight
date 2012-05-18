/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#define UTIL_PATH_SEP_STRING           "/"   /* A \0 terminated separator used when we want a (char *) instance.                   */
#define UTIL_PATH_SEP_CHAR             '/'   /* A simple character used when we want an actual char instance (i.e. not a pointer). */
#define UTIL_NEWLINE_STRING "          \n"       
#define UTIL_DEFAULT_MKDIR_MODE 0777         /* Directories are by default created with mode a+rwx - and then comes the umask ... */

#ifdef __cplusplus
extern"C" {
#endif


/*****************************************************************/
/**
   
   The four macros UTIL_IS_INSTANCE_FUNCTION, UTIL_SAFE_CAST_FUNTION,
   UTIL_TYPE_ID_DECLARATION and UTIL_TYPE_ID_INIT can be used to
   implement a simple system for type checking (void *) at
   runtime. The system is based on a unique integer for each class,
   this must be provided by the user.

   The motivation for these functions is to be able to to type-check
   the arguments to callback functions like pthread_create.

    UTIL_TYPE_ID_DECLARATION: Adds a field "int __type_id;" to the
      struct defintion.

    UTIL_TYPE_ID_INIT: Should be added to the allocation routine,
      inserts a "->__type_id = magic_int;" code line in the alloc
      routine.

    UTIL_IS_INSTANCE_FUNCTION: This macro will generate a function
      <type>_is_instance(void *) which will cast the (void *) input to
      (type *), and check the value of __type_id. If this is the
      correct value true is returned, otherwise the function will
      return false. Observe that the function will accept NULL as
      input; in which case it will return false.

    UTIL_SAFE_CAST_FUNCTION: This is similar to
      UTIL_IS_INSTANCE_FUNCTION, but it will return (type *) if the
      cast succeeds, and fail hard if it fails. There is also a _CONST
      variety of this function.
      
*/


   

#define UTIL_IS_INSTANCE_FUNCTION(type , TYPE_ID)          \
bool type ## _is_instance( const void * __arg ) {          \
   if (__arg == NULL)                                      \
      return false;                                        \
   else {                                                  \
      const type ## _type * arg = (type ## _type *) __arg; \
      if ( arg->__type_id == TYPE_ID)                      \
         return true;                                      \
      else                                                 \
         return false;                                     \
   }                                                       \
}


#define UTIL_IS_INSTANCE_HEADER(type) bool type ## _is_instance( const void * __arg );


#define UTIL_SAFE_CAST_FUNCTION(type , TYPE_ID)                                          \
type ## _type * type ## _safe_cast( void * __arg ) {                                     \
   if (__arg == NULL) {                                                                  \
      util_abort("%s: runtime cast failed - tried to dereference NULL\n",__func__);      \
      return NULL;                                                                       \
   }                                                                                     \
   type ## _type * arg = (type ## _type *) __arg;                                        \
   if ( arg->__type_id == TYPE_ID)                                                       \
      return arg;                                                                        \
   else {                                                                                \
      util_abort("%s: runtime cast failed: File:%s Line:%d. Got:%d  Expected:%d \n", __func__ , __FILE__ , __LINE__ , arg->__type_id , TYPE_ID); \
      return NULL;                                                                       \
   }                                                                                     \
}
#define UTIL_SAFE_CAST_HEADER( type ) type ## _type * type ## _safe_cast( void * __arg );


#define UTIL_SAFE_CAST_FUNCTION_CONST(type , TYPE_ID)                                    \
const type ## _type * type ## _safe_cast_const( const void * __arg ) {                   \
   const type ## _type * arg = (const type ## _type *) __arg;                            \
   if ( arg->__type_id == TYPE_ID)                                                       \
      return arg;                                                                        \
   else {                                                                                \
      util_abort("%s: runtime cast failed: File:%s Line:%d. Got:%d  Expected:%d \n", __func__ , __FILE__ , __LINE__ , arg->__type_id , TYPE_ID); \
      return NULL;                                                                       \
   }                                                                                     \
}
#define UTIL_SAFE_CAST_HEADER_CONST( type ) const type ## _type * type ## _safe_cast_const( const void * __arg );

#define UTIL_TYPE_ID_DECLARATION           int   __type_id; 
#define UTIL_TYPE_ID_INIT(var , TYPE_ID)   var->__type_id = TYPE_ID;

/*****************************************************************/
/*
  
*/

#define LIBRARY_VERSION(libname) \
const char * libname ## _svn_version() { return SVN_VERSION; } \
const char * libname ## _build_time()  { return NULL; }


#define LIBRARY_VERSION_HEADER(libname) \
const char * libname ## _svn_version(); \
const char * libname ## _build_time();

/*****************************************************************/
typedef void (walk_file_callback_ftype)   (const char * , /* The current directory */
                                           const char * , /* The current file / directory */
                                           void *);       /* Arbitrary argument */

typedef bool (walk_dir_callback_ftype)   (const char * , /* The current directory */
                                          const char * , /* The current file / directory */
                                          int          , /* The current depth in the file hiearcrcy. */
                                          void *);       /* Arbitrary argument */


typedef enum {left_pad  = 0,
              right_pad = 1,
              center    = 2} string_alignement_type;

LIBRARY_VERSION_HEADER(libutil);

void         util_bitmask_on(int *  , int );
time_t       util_make_datetime(int , int , int , int , int , int );
void         util_fprintf_datetime(time_t  , FILE * );
void         util_fprintf_date(time_t  , FILE * );
time_t       util_make_date(int , int , int);
void         util_inplace_forward_days(time_t *  , double);
double       util_difftime(time_t  , time_t  , int *  , int *  , int *  , int *);
double       util_difftime_days(time_t  , time_t );
char       * util_alloc_date_string( time_t t );
char       * util_alloc_date_stamp( );
double       util_pow10(double x);
bool         util_char_in(char c, int , const char *);
char       * util_alloc_sprintf_va(const char * fmt , va_list ap);
char       * util_alloc_sprintf(const char *  , ...);
char       * util_realloc_sprintf(char * , const char *  , ...);
void         util_fprintf_int(int , int , FILE * );
void         util_fprintf_string(const char *  , int , string_alignement_type ,  FILE * );
void         util_fprintf_double(double , int , int , char , FILE *);
bool         util_fscanf_date(FILE * , time_t *);
bool         util_sscanf_date(const char * , time_t *);
char       * util_blocking_alloc_stdin_line(unsigned long );
char       * util_alloc_stdin_line();
char       * util_realloc_stdin_line(char * );
bool         util_is_executable(const char * );
bool         util_entry_exists( const char * entry );
bool         util_file_exists(const char *);
bool         util_is_abs_path(const char * );
char       * util_alloc_abs_path( const char * path );
bool         util_fmt_bit8   (const char *);
bool         util_fmt_bit8_stream(FILE * );
void         util_make_path  (const char *);
char       * util_newest_file(const char *, const char *);
double       util_file_difftime(const char * , const char *);
bool         util_file_update_required(const char *, const char *);
size_t       util_file_size(const char *);
void         util_clear_directory(const char *path, bool strict_uid , bool unlink_root);
void         util_unlink_existing(const char *filename);
void         util_strupr(char *);
bool         util_string_equal(const char * s1 , const char * s2 );
char       * util_alloc_strupr_copy(const char * );
void         util_string_tr(char * , char , char);
bool         util_copy_stream(FILE *, FILE *, int , void * , bool abort_on_error);
  void         util_move_file(const char * src_file , const char * target_file);
  void         util_move_file4( const char * src_name , const char * target_name , const char *src_path , const char * target_path);
bool         util_copy_file(const char * , const char * );
void         util_copy_directory(const char *  , const char * , const char *);
void         util_walk_directory(const char * root_path , walk_file_callback_ftype * file_callback , void * file_callback_arg , walk_dir_callback_ftype * dir_callback , void * dir_callback_arg);
char       * util_alloc_cwd(void);
char       * util_alloc_realpath(const char * );
bool         util_string_match(const char * string , const char * pattern);
bool         util_string_has_wildcard( const char * s);

#ifdef HAVE_UID_T
uid_t        util_get_entry_uid( const char * file );
#endif
bool         util_entry_readable( const char * entry );
bool         util_addmode_if_owner( const char * filename , mode_t add_mode );
bool         util_delmode_if_owner( const char * filename , mode_t del_mode);
bool         util_chmod_if_owner( const char * filename , mode_t new_mode);

int          util_forward_line(FILE * , bool * );
void         util_rewind_line(FILE *);
int          util_count_content_file_lines(FILE * );
int          util_count_file_lines(FILE * );
FILE       * util_mkdir_fopen( const char * filename , const char * mode );
FILE       * util_fopen(const char *  , const char *);
void         util_fclose( FILE * stream );
bool         util_fopen_test(const char *, const char *);
void         util_alloc_file_components(const char * , char ** , char **, char **);
  //char           * util_realloc_full_path(char * , const char *, const char *);
char       * util_alloc_tmp_file(const char * , const char * , bool );
char       * util_fscanf_alloc_line(FILE *, bool *);
char       * util_fscanf_realloc_line(FILE *, bool * , char *);
char       * util_fscanf_alloc_token(FILE * );
void         util_fskip_token(FILE * );
void         util_fskip_space(FILE *  ,  bool *);
void         util_fskip_chars(FILE * , const char * , bool *);
void         util_fskip_cchars(FILE * , const char * , bool *);
bool         util_fscanf_int(FILE * , int * );
bool         util_fscanf_bool(FILE * stream , bool * value);
bool         util_sscanf_bool(const char * , bool *);
bool         util_sscanf_octal_int(const char * buffer , unsigned int * value);
int          util_strcmp_int( const char * s1 , const char * s2);
int          util_strcmp_float( const char * s1 , const char * s2);
bool         util_sscanf_int(const char * , int * );
const char * util_parse_int(const char * , int * , bool *);
const char * util_skip_sep(const char * , const char * , bool *);
int      util_scanf_int_with_limits(const char * , int  , int  , int );
char   * util_scanf_int_with_limits_return_char(const char * , int  , int  , int );
void     util_printf_prompt(const char * , int , char , const char *);
int      util_scanf_int(const char * , int);
char   * util_scanf_int_return_char(const char * , int);
double   util_scanf_double(const char * prompt , int prompt_len);
char   * util_scanf_alloc_string(const char * );
bool     util_sscanf_double(const char * , double * );
  //char   * util_alloc_full_path(const char *, const char *);
char   * util_alloc_filename(const char * , const char *  , const char * );
char   * util_realloc_filename(char *  , const char *  , const char *  , const char * );
char   * util_alloc_strip_copy(const char *);
char   * util_realloc_strip_copy(char *);
void     util_set_strip_copy(char * , const char *);
char   * util_alloc_string_sum(const char **  , int);
char   * util_strcat_realloc(char *, const char * );
char   * util_alloc_string_copy(const char *);
char  ** util_stringlist_append_copy(char **  , int , const char * );
char  ** util_stringlist_append_ref(char **  , int , const char * );
char  ** util_alloc_stringlist_copy(const char **, int );
void     util_split_string(const char *, const char *, int *, char ***);
void     util_path_split(const char * , int *, char ***);
void     util_binary_split_string(const char * , const char * , bool  , char ** , char ** );
char   * util_alloc_joined_string(const char **  , int , const char * );
char   * util_alloc_multiline_string(const char ** , int );
char   * util_string_replace_alloc(const char *, const char *, const char *);
char   * util_string_replacen_alloc(const char *, int , const char ** , const char **);
int      util_string_replace_inplace(char ** , const char *  , const char *);
char   * util_string_strip_chars_alloc(const char *, const char * );
char   * util_realloc_string_copy(char * , const char *);
char   * util_realloc_substring_copy(char * , const char *, int );
char   * util_realloc_dequoted_string(char *);
char   * util_alloc_dequoted_copy(const char *s);
void     util_safe_free(void *);
void     util_free_stringlist(char **, int );
char   * util_alloc_substring_copy(const char *, int );
bool     util_is_directory(const char * );
bool     util_is_file(const char * );
void     util_set_datetime_values(time_t , int * , int * , int * , int * , int *  , int *);
void     util_set_date_values(time_t , int * , int * , int * );


void     util_fread_from_buffer(void *  , size_t  , size_t , char ** );

unsigned int util_clock_seed( );
void         util_fread_dev_random(int , char * );
void         util_fread_dev_urandom(int , char * );
char *       util_alloc_string_copy(const char *);
void         util_enkf_unlink_ensfiles(const char *, const char *, int , bool );
bool         util_string_isspace(const char * s);

void    util_exit(const char * fmt , ...);
void    util_abort(const char * fmt , ...);
void    util_abort_signal(int );
void    util_abort_append_version_info(const char * );
void    util_abort_free_version_info();
void    util_abort_set_executable( const char * executable );
void *  util_realloc(void *  , size_t  , const char * );
void *  util_malloc(size_t , const char * );
void * util_realloc_copy(void * org_ptr , const void * src , size_t byte_size , const char * caller);
void *  util_alloc_copy(const void * , size_t , const char * );
void    util_double_to_float(float  * , const double * , int );
void    util_float_to_double(double * , const float  * , int );

int     util_get_month_nr(const char * );

char *  util_fread_alloc_file_content(const char * , int *);
void    util_fwrite_string(const char * , FILE *);
char *  util_fread_realloc_string(char *  , FILE *);
char *  util_fread_alloc_string(FILE *);
void    util_fskip_string(FILE *stream);
void    util_endian_flip_vector(void *, int , int );
bool    util_proc_alive(pid_t pid);
int     util_proc_mem_free(void);


void     util_apply_int_limits(int * , int , int );
void     util_apply_float_limits(float * , float , float );
void     util_apply_double_limits(double * , double , double );
double   util_double_vector_mean(int , const double * );
double   util_double_vector_stddev(int , const double * );
void     util_double_vector_max_min(int  , const double *, double * , double *);
void     util_update_double_max_min(double , double * , double * );
void     util_update_float_max_min(float , float * , float * );
void     util_update_int_max_min(int , int * , int * );
float    util_float_max (float   , float );
long int util_long_max(long int a , long int b);
int      util_int_max   (int     , int);
double   util_double_max(double  , double );
float    util_float_min (float   , float );
int      util_int_min   (int     , int);
size_t   util_size_t_min(size_t a , size_t b);
size_t   util_size_t_max(size_t a , size_t b);
time_t   util_time_t_min(time_t a , time_t b);
time_t   util_time_t_max(time_t a , time_t b);
double   util_double_min(double  , double );
void     util_fskip_lines(FILE * , int);
bool     util_same_file(const char *  , const char * );
void     util_read_path(const char * , int , bool , char *  );
char *   util_fscanf_alloc_filename(const char * , int , int);
void     util_read_string(const char *  , int  , char * );
void     util_fread (void *, size_t , size_t , FILE * , const char * );
void     util_fwrite(const void *, size_t , size_t , FILE * , const char * );
time_t   util_fread_time_t(FILE * stream);
int      util_fread_int(FILE * );
long     util_fread_long(FILE * );
bool     util_fread_bool(FILE * );
double   util_fread_double(FILE * stream);
void     util_fwrite_int   (int    , FILE * );
void     util_fwrite_long  (long    , FILE * );
void     util_fwrite_bool  (bool    , FILE * );
void     util_fwrite_time_t  (time_t  , FILE * );
void     util_fwrite_double(double , FILE * );
void     util_fwrite_int_vector   (const int     * , int , FILE * , const char * );
void     util_fwrite_double_vector(const double  * , int , FILE * , const char * );
void     util_fread_char_vector(char * , int , FILE * , const char * );

#define CONTAINS_HEADER(TYPE) int util_sorted_contains_ ## TYPE(const TYPE * data , int size , TYPE value);
  CONTAINS_HEADER(int);
  CONTAINS_HEADER(time_t);
  CONTAINS_HEADER(size_t);
#undef CONTAINS_HEADER

#ifdef HAVE_ZLIB
void     util_compress_buffer(const void * , int , void * , unsigned long * );
int      util_fread_sizeof_compressed(FILE * stream);
void     util_fread_compressed(void * , FILE * );
void   * util_fread_alloc_compressed(FILE * );
void     util_fwrite_compressed(const void * , int , FILE * );
#endif

void     util_block_growing_file(const char * );
void     util_block_growing_directory(const char * );
char   * util_alloc_realpath(const char * );
bool     util_sscanf_bytesize(const char * , size_t *);
void     util_sscanf_active_range(const char *  , int , bool * );
int    * util_sscanf_alloc_active_list(const char *  , int * );
int      util_get_current_linenr(FILE * stream);
const char * util_update_path_var(const char * , const char * , bool );


int      util_get_type( void * data );
void     util_fskip_int(FILE * stream);
void     util_fskip_long(FILE * stream);
void     util_fskip_bool(FILE * stream);
bool     util_fseek_string(FILE * stream , const char * string , bool skip_string , bool case_sensitive);
char   * util_fscanf_alloc_upto(FILE * stream , const char * stop_string, bool include_stop_string);
bool     util_files_equal( const char * file1 , const char * file2 );
double   util_kahan_sum(const double *data, size_t N);
int      util_fnmatch( const char * pattern , const char * string );
void     util_localtime( time_t * t , struct tm * ts );

char       * util_alloc_PATH_executable(const char * executable );
char       * util_isscanf_alloc_envvar( const char * string , int env_index );
void         util_setenv( const char * variable , const char * value);
const char * util_interp_setenv( const char * variable , const char * value);
void         util_unsetenv( const char * variable);
char       * util_alloc_envvar( const char * value );

#ifdef HAVE_SYMLINK
  bool         util_is_link(const char * );
  void         util_make_slink(const char *, const char * );
  char       * util_alloc_link_target(const char * link);
#ifdef HAVE_READLINKAT
  char     *   util_alloc_atlink_target(const char * path , const char * link);
#endif
#endif

#ifdef HAVE_FORK
#include "util_fork.h"
#endif


#ifdef HAVE_LOCKF
FILE       * util_fopen_lockf(const char * , const char * );
bool     util_try_lockf(const char *  , mode_t  , int * );
#endif


#define UTIL_FWRITE_SCALAR(s,stream) { if (fwrite(&s , sizeof s , 1 , stream) != 1) util_abort("%s: write failed: %s\n",__func__ , strerror(errno)); }
#define UTIL_FREAD_SCALAR(s,stream)  { if (fread(&s , sizeof s , 1 , stream) != 1) util_abort("%s: read failed: %s\n",__func__ , strerror(errno)); }

#define UTIL_FWRITE_VECTOR(s,n,stream) { if (fwrite(s , sizeof s , (n) , stream) != (n)) util_abort("%s: write failed: %s \n",__func__ , strerror(errno)); }
#define UTIL_FREAD_VECTOR(s,n,stream)  { if (fread(s , sizeof s , (n) , stream) != (n)) util_abort("%s: read failed: %s \n",__func__ , strerror(errno)); }

/*****************************************************************/
/*
  The code below here is a simple functionality to support 'enum
  introspection'; the point is that when calling the library functions
  from Python/ctypes it is very valuable to have access to the enum
  values from the Python side. The enum defintions is just used during
  the compile phase, and then subsequently dropped. It is therefor
  impossible to determine enum values by inspecting the resulting
  object files.

  The approach which has been chosen is that each of the enums which
  should support 'introspection' from Python should have a function:

     const char * <enum>_iget(int index, int * value) {
        ...
     }
     
  which should take an enum element number as input argument and
  return a string representation of the corresponding enum element and
  also update the value reference to contain the corresponding enum
  value. If index is out of range the function should return NULL and
  also set value to -1. The python layer can then create an integer
  variable with the correct name and value in the calling module.

  The util_enum_element_type and the util_enum_iget() function are
  convenience functions which can be used to avoid indirectly
  repeating the enum definition in the <enum>_iget() function. 

  In the example below we create the enum definition in normal way in
  the header file, and then in addition we repeat the defintion in a
  #define a symbol which is used as argument in the <enum>_iget()
  function:


  header_file:
  ------------
  enum my_enum {
    INVALID = 0,
    VALUE1  = 2,
    VALUE2  = 17
  }
    
  // The enum definition is repeated; but at at least at the very same spot of the code.

  #define MY_ENUM_DEF  { .value = INVALID, .name="INVALID"} , {.value = VALUE1 , .name="VALUE1"} , {.value = VALUE2 , .name="VALUE2"}
  #define MY_ENUM_SIZE 3

  
  source file:
  ------------
  
  const char * my_enum_iget(int index, int * value) {
     return util_enum_iget( index , MY_ENUM_SIZE , (const util_enum_element_type []) MY_ENUM_DEF , value);
  }

*/  
   

typedef struct {
  int value;
  const char * name;
} util_enum_element_type;

const char * util_enum_iget( int index , int size , const util_enum_element_type * enum_defs , int * value);





#ifdef __cplusplus
}
#endif
#endif

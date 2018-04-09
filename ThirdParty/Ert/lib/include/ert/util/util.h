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

#ifndef ERT_UTIL_H
#define ERT_UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>


#include <ert/util/ert_api_config.h>



#ifdef ERT_HAVE_GETUID
#include <sys/stat.h>
#endif

#ifdef ERT_WINDOWS
#define UTIL_PATH_SEP_STRING           "\\"   /* A \0 terminated separator used when we want a (char *) instance.                   */
#define UTIL_PATH_SEP_CHAR             '\\'   /* A simple character used when we want an actual char instance (i.e. not a pointer). */
#else
#define UTIL_PATH_SEP_STRING           "/"   /* A \0 terminated separator used when we want a (char *) instance.                   */
#define UTIL_PATH_SEP_CHAR             '/'   /* A simple character used when we want an actual char instance (i.e. not a pointer). */
#endif

#define UTIL_NEWLINE_STRING "          \n"
#define UTIL_DEFAULT_MKDIR_MODE 0777         /* Directories are by default created with mode a+rwx - and then comes the umask ... */


#ifdef __cplusplus
extern"C" {
#endif


/*
  These ifdefs are an attempt to support large files (> 2GB)
  transparently on both Windows and Linux. See source file
  libert_util/src/util_lfs.c for more details.

  During the configure step CMAKE should check the size of (void *)
  and set the ERT_WINDOWS_LFS variable to true if a 64 bit platform is
  detected.
*/

#ifdef ERT_WINDOWS_LFS
typedef struct _stat64 stat_type;
typedef __int64 offset_type;
#else
typedef struct stat stat_type;
#ifdef HAVE_FSEEKO
  typedef off_t offset_type;
#else
  typedef long offset_type;
#endif
#endif



/*****************************************************************/
/*

*/


/*****************************************************************/
typedef void (walk_file_callback_ftype)   (const char * , /* The current directory */
                                           const char * , /* The current file / directory */
                                           void *);       /* Arbitrary argument */

typedef bool (walk_dir_callback_ftype)   (const char * , /* The current directory */
                                          const char * , /* The current file / directory */
                                          int          , /* The current depth in the file hiearcrcy. */
                                          void *);       /* Arbitrary argument */



typedef enum {left_pad   = 0,
              right_pad  = 1,
              center_pad = 2} string_alignement_type;

  //#define UTIL_CXX_MALLOC(var , num_elm) (typeof (var)) util_malloc( (num_elm) * sizeof var)
  void         util_bitmask_on(int *  , int );
  char       * util_get_timezone(void);
  time_t       util_make_datetime_utc(int , int , int , int , int , int );
  bool         util_make_datetime_utc_validated(int sec, int min, int hour , int mday , int month , int year, time_t * t);
  void         util_fprintf_date_utc(time_t  , FILE * );
  time_t       util_make_date_utc(int , int , int);
  time_t       util_make_pure_date_utc(time_t t);
  void         util_inplace_forward_seconds_utc(time_t * t , double seconds);
  void         util_inplace_forward_days_utc(time_t * t , double days);

  time_t       util_file_mtime(const char * file);
  double       util_difftime(time_t  , time_t  , int *  , int *  , int *  , int *);
  double       util_difftime_days(time_t  , time_t );
  double       util_difftime_seconds( time_t start_time , time_t end_time);
  bool         util_after( time_t t , time_t limit);
  bool         util_before( time_t t , time_t limit);
  bool         util_file_newer( const char * file , time_t t0);
  bool         util_file_older( const char * file , time_t t0);

  char       * util_alloc_date_string_utc( time_t t );
  char       * util_alloc_date_stamp_utc( void );

  double       util_pow10(double x);
  bool         util_char_in(char c, int , const char *);
  char       * util_alloc_sprintf_va(const char * fmt , va_list ap);
  char       * util_alloc_sprintf(const char *  , ...);
  char       * util_alloc_sprintf_escape(const char * src , int max_escape);
  char       * util_realloc_sprintf(char * , const char *  , ...);
  void         util_fprintf_int(int , int , FILE * );
  void         util_fprintf_string(const char *  , int , string_alignement_type ,  FILE * );
  void         util_fprintf_double(double , int , int , char , FILE *);
  bool         util_fscanf_date_utc(FILE * , time_t *);
  bool         util_sscanf_date_utc(const char * , time_t *);
  bool         util_sscanf_isodate(const char * , time_t *);
  bool         util_sscanf_percent(const char * string, double * value);
  bool         util_is_executable(const char * );
  bool         util_entry_exists( const char * entry );
  bool         util_file_exists(const char *);
  bool         util_is_abs_path(const char * );
  char       * util_alloc_abs_path( const char * path );
  char       * util_alloc_rel_path( const char * __root_path , const char * path);
  bool         util_fmt_bit8   (const char *);
  bool         util_fmt_bit8_stream(FILE * );
  char       * util_strstr_int_format(const char * string );
  int          util_int_format_count(const char * string );
  void         util_make_path  (const char *);
  char       * util_newest_file(const char *, const char *);
  double       util_file_difftime(const char * , const char *);
  bool         util_file_update_required(const char *, const char *);
  size_t       util_file_size(const char *);
  size_t       util_fd_size(int fd);
  void         util_clear_directory(const char *path, bool strict_uid , bool unlink_root);
  void         util_unlink_existing(const char *filename);
  void         util_strupr(char *);
  bool         util_string_equal(const char * s1 , const char * s2 );
  char       * util_alloc_strupr_copy(const char * );
  void         util_string_tr(char * , char , char);
  bool         util_copy_stream(FILE *, FILE *, size_t , void * , bool abort_on_error);
  void         util_move_file(const char * src_file , const char * target_file);
  void         util_move_file4( const char * src_name , const char * target_name , const char *src_path , const char * target_path);
  bool         util_copy_file(const char * , const char * );
  char       * util_alloc_cwd(void);
  bool         util_is_cwd( const char * path );
  char       * util_alloc_normal_path( const char * input_path );
  char       * util_alloc_realpath(const char * );
  char       * util_alloc_realpath__(const char * input_path);
  bool         util_string_match(const char * string , const char * pattern);
  bool         util_string_has_wildcard( const char * s);
  bool         util_file_readable( const char * file );
  bool         util_entry_readable( const char * entry );
  bool         util_entry_writable( const char * entry );
  bool         util_ftruncate(FILE * stream , long size);

  void         util_usleep( unsigned long micro_seconds );
  void         util_yield(void);

  int          util_roundf( float x );
  int          util_round( double x );

  offset_type  util_ftell(FILE * stream);
  int          util_fseek(FILE * stream, offset_type offset, int whence);
  void         util_rewind(FILE * stream);
  int          util_stat(const char * filename , stat_type * stat_info);
  int          util_fstat(int fileno, stat_type * stat_info);



#ifdef ERT_HAVE_OPENDIR
  void         util_copy_directory_content(const char * src_path , const char * target_path);
  void         util_copy_directory(const char *  , const char *);
  void         util_walk_directory(const char * root_path , walk_file_callback_ftype * file_callback , void * file_callback_arg , walk_dir_callback_ftype * dir_callback , void * dir_callback_arg);
#endif


#ifdef ERT_HAVE_GETUID
  uid_t        util_get_entry_uid( const char * file );
  mode_t       util_getmode( const char * filename);
  bool         util_addmode_if_owner( const char * filename , mode_t add_mode );
  bool         util_delmode_if_owner( const char * filename , mode_t del_mode);
  bool         util_chmod_if_owner( const char * filename , mode_t new_mode);
#endif

#ifdef HAVE_PROC
  bool    util_proc_alive(pid_t pid);
#endif

  int          util_forward_line(FILE * , bool * );
  void         util_rewind_line(FILE *);
  int          util_count_content_file_lines(FILE * );
  int          util_count_file_lines(FILE * );
  FILE       * util_mkdir_fopen( const char * filename , const char * mode );
  int          util_fmove( FILE * stream , long offset , long shift);
  FILE       * util_fopen(const char *  , const char *); 
  FILE       * util_fopen__(const char * filename , const char * mode);
  void         util_fclose( FILE * stream );
  bool         util_fopen_test(const char *, const char *);
  char       * util_split_alloc_dirname( const char * input_path );
  char       * util_split_alloc_filename( const char * input_path );
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
  bool         util_sscanf_octal_int(const char * buffer , int * value);
  int          util_strcmp_int( const char * s1 , const char * s2);
  int          util_strcmp_float( const char * s1 , const char * s2);
  bool         util_sscanf_int(const char * , int * );
  const char * util_parse_int(const char * , int * , bool *);
  const char * util_skip_sep(const char * , const char * , bool *);
 bool         util_sscanf_double(const char * , double * );
  //char   * util_alloc_full_path(const char *, const char *);
  char       * util_alloc_filename(const char * , const char *  , const char * );
  char       * util_realloc_filename(char *  , const char *  , const char *  , const char * );
  char       * util_alloc_strip_copy(const char *);
  char       * util_realloc_strip_copy(char *);
  void         util_set_strip_copy(char * , const char *);
  char       * util_alloc_string_sum(const char **  , int);
  char       * util_strcat_realloc(char *, const char * );
  char       * util_alloc_string_copy(const char *);
  char      ** util_stringlist_append_copy(char **  , int , const char * );
  char      ** util_stringlist_append_ref(char **  , int , const char * );
  char      ** util_alloc_stringlist_copy(const char **, int );
  void         util_split_string(const char *, const char *, int *, char ***);
  void         util_path_split(const char * , int *, char ***);
  char       * util_alloc_parent_path( const char * path);
  void         util_binary_split_string(const char * , const char * , bool  , char ** , char ** );
  void         util_binary_split_string_from_max_length(const char * , const char * , int  , char ** , char ** );
  char       * util_alloc_joined_string(const char **  , int , const char * );
  char       * util_alloc_multiline_string(const char ** , int );
  char       * util_string_replace_alloc(const char *, const char *, const char *);
  char       * util_string_replacen_alloc(const char *, int , const char ** , const char **);
  int          util_string_replace_inplace(char ** , const char *  , const char *);
  char       * util_string_strip_chars_alloc(const char *, const char * );
  char       * util_realloc_string_copy(char * , const char *);
  char       * util_realloc_substring_copy(char * , const char *, int N);
  char       * util_realloc_dequoted_string(char *);
  char       * util_alloc_dequoted_copy(const char *s);
  void         util_safe_free(void *);
  void         util_free_stringlist(char **, int );
  void         util_free_NULL_terminated_stringlist(char ** string_list);
  char       * util_alloc_substring_copy(const char *, int offset , int N);
  bool         util_is_directory(const char * );
  bool         util_is_file(const char * );


  void         util_set_datetime_values_utc(time_t , int * , int * , int * , int * , int *  , int *);
  void         util_set_date_values_utc(time_t , int * , int * , int * );

  bool         util_is_first_day_in_month_utc( time_t t);

  unsigned int util_dev_urandom_seed( );
  unsigned int util_clock_seed( void );
  void         util_fread_dev_random(int , char * );
  void         util_fread_dev_urandom(int , char * );
  bool         util_string_isspace(const char * s);

  char *  util_alloc_dump_filename(void);
  void    util_abort_test_set_intercept_function(const char *);
  bool    util_addr2line_lookup(const void *, char **, char **, int *);
  void    util_exit(const char * fmt , ...);
  void    util_install_signals(void);
  void    util_update_signals(void);


  void *  util_realloc(void *  , size_t  );
  void    util_free(void * ptr);
  void *  util_malloc(size_t );
  void *  util_calloc( size_t elements , size_t element_size );
  void *  util_realloc_copy(void * org_ptr , const void * src , size_t byte_size );
  void *  util_alloc_copy(const void * , size_t );
  void    util_double_to_float(float  * , const double * , int );
  void    util_float_to_double(double * , const float  * , int );

  int     util_get_month_nr(const char * );

  char *  util_fread_alloc_file_content(const char * , int *);
  void    util_fwrite_string(const char * , FILE *);
  char *  util_fread_realloc_string(char *  , FILE *);
  char *  util_fread_alloc_string(FILE *);
  void    util_fskip_string(FILE *stream);
  void     util_endian_flip_vector(void * data , int element_size , int elements);
  int      util_proc_mem_free(void);


  void     util_clamp_double(double * value , double limit1, double limit2);
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
  void     util_fread (void *, size_t , size_t , FILE * , const char * );
  void     util_fwrite(const void *, size_t , size_t , FILE * , const char * );
  time_t   util_fread_time_t(FILE * stream);
  int      util_fread_int(FILE * );
  long     util_fread_long(FILE * );
  bool     util_fread_bool(FILE * );
  double   util_fread_double(FILE * stream);
  void     util_fwrite_offset(offset_type    , FILE * );
  void     util_fwrite_size_t (size_t    , FILE * );
  void     util_fwrite_int   (int    , FILE * );
  void     util_fwrite_long  (long    , FILE * );
  void     util_fwrite_bool  (bool    , FILE * );
  void     util_fwrite_time_t  (time_t  , FILE * );
  void     util_fwrite_double(double , FILE * );
  void     util_fwrite_int_vector   (const int     * , int , FILE * , const char * );
  void     util_fwrite_double_vector(const double  * , int , FILE * , const char * );
  void     util_fread_char_vector(char * , int , FILE * , const char * );
  int      util_type_get_id( const void * data );


  bool     util_sscanf_bytesize(const char * , size_t *);
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
  bool     util_double_approx_equal( double d1 , double d2);
  bool     util_double_approx_equal__( double d1 , double d2, double rel_eps, double abs_eps);
  bool     util_float_approx_equal__( float d1 , float d2, float rel_eps, float abs_eps);
  int      util_fnmatch( const char * pattern , const char * string );
  void     util_time_utc( time_t * t , struct tm * ts );

  bool         util_is_link(const char * );  // Will always return false on windows
  int          util_chdir(const char * path);
  bool         util_chdir_file( const char * filename );

#ifdef ERT_HAVE_UNISTD
#include <unistd.h>
  bool         util_access(const char * entry, mode_t mode);
#else
  bool         util_access(const char * entry, int mode);
#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1
#endif

#define UTIL_FWRITE_SCALAR(s,stream) { if (fwrite(&s , sizeof s , 1 , stream) != 1) util_abort("%s: write failed: %s\n",__func__ , strerror(errno)); }

#define UTIL_FREAD_SCALAR(s,stream)  {                               \
  int fread_return = fread(&s , sizeof s , 1 , stream);              \
  if (fread_return == 0) {                                           \
     if (errno == 0)                                                 \
        util_abort("%s: read failed - premature EOF\n",__func__);    \
                                                                     \
     util_abort("%s: read failed: %s\n",__func__ , strerror(errno)); \
  }                                                                  \
}


#define UTIL_FWRITE_VECTOR(s,n,stream) { if (fwrite(s , sizeof s , (n) , stream) != (n)) util_abort("%s: write failed: %s \n",__func__ , strerror(errno)); }
#define UTIL_FREAD_VECTOR(s,n,stream)  { if (fread(s , sizeof s , (n) , stream) != (n)) util_abort("%s: read failed: %s \n",__func__ , strerror(errno)); }

#define CONTAINS_HEADER(TYPE) int util_sorted_contains_ ## TYPE(const TYPE * data , int size , TYPE value)
  CONTAINS_HEADER(int);
  CONTAINS_HEADER(time_t);
  CONTAINS_HEADER(size_t);
#undef CONTAINS_HEADER

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


#ifdef _MSC_VER
#define util_abort(fmt , ...) util_abort__(__FILE__ , __func__ , __LINE__ , fmt , __VA_ARGS__)
#elif __GNUC__
/* Also clang defines the __GNUC__ symbol */
#define util_abort(fmt , ...) util_abort__(__FILE__ , __func__ , __LINE__ , fmt , ##__VA_ARGS__)
#endif



/*****************************************************************/
/* Conditional section below here */

void    util_abort__(const char * file , const char * function , int line , const char * fmt , ...);
void    util_abort_signal(int );



#ifdef ERT_HAVE_ZLIB
  void     util_compress_buffer(const void * , int , void * , unsigned long * );
  int      util_fread_sizeof_compressed(FILE * stream);
  void     util_fread_compressed(void * , FILE * );
  void   * util_fread_alloc_compressed(FILE * );
  void     util_fwrite_compressed(const void * , int , FILE * );
#endif

#ifdef ERT_HAVE_SYMLINK
  void         util_make_slink(const char *, const char * );
  char       * util_alloc_link_target(const char * link);
#ifdef ERT_HAVE_READLINKAT
  char     *   util_alloc_atlink_target(const char * path , const char * link);
#endif
#endif


#ifdef ERT_HAVE_SPAWN
  pid_t      util_spawn(const char *executable, int argc, const char **argv, const char *stdout_file, const char *stderr_file);
  int        util_spawn_blocking(const char *executable, int argc, const char **argv, const char *stdout_file, const char *stderr_file);
#endif


#ifdef ERT_HAVE_LOCKF
  FILE       * util_fopen_lockf(const char * , const char * );
  bool         util_try_lockf(const char *  , mode_t  , int * );
#endif

#include "util_unlink.h"

#ifdef __cplusplus
}
#endif
#endif

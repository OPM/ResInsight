/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'fortio.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FORTIO_H__
#define __FORTIO_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
  FORTIO_NOENTRY         = 0,    /* File does not exists at all - application error. */
  FORTIO_EOF             = 1,    /* The file / record is empty */
  FORTIO_OK              = 2,    /* The file / record is OK with: [32 bit header | data | 32 bit footer] */
  FORTIO_MISSING_DATA    = 3,    
  FORTIO_MISSING_TAIL    = 4,
  FORTIO_HEADER_MISMATCH = 5
} fortio_status_type;

#define FORTIO_READ  1
#define FORTIO_WRITE 2

typedef struct fortio_struct fortio_type;

  fortio_status_type fortio_check_buffer( FILE * stream , bool endian_flip , size_t buffer_size );
  fortio_status_type fortio_check_file( const char * filename , bool endian_flip);
  bool               fortio_guess_endian_flip(const char * , bool *);
  bool               fortio_is_fortran_file(const char *  , bool * );
  void               fortio_copy_record(fortio_type * , fortio_type * , int , void * , bool *);
  fortio_type *      fortio_alloc_FILE_wrapper(const char * , bool , bool , FILE * );
  fortio_type *      fortio_open_reader(const char *, bool , bool);
  fortio_type *      fortio_open_writer(const char *, bool , bool);
  fortio_type *      fortio_open_readwrite(const char *, bool , bool);
  void               fortio_free_FILE_wrapper(fortio_type *);
  void               fortio_fclose(fortio_type *);
  int                fortio_init_read(fortio_type *);
  void               fortio_complete_read(fortio_type *);
  void               fortio_init_write(fortio_type * , int);
  void               fortio_complete_write(fortio_type *);
  void               fortio_fskip_buffer(fortio_type *, int );
  int                fortio_fskip_record(fortio_type *);
  int                fortio_fread_record(fortio_type * , char *buffer);
  void               fortio_fread_buffer(fortio_type * , char * , int );
  void               fortio_fwrite_record(fortio_type * , const char *, int);
  FILE        *      fortio_get_FILE(const fortio_type *);
  void               fortio_fflush(fortio_type * ) ;
  int                fortio_get_record_size(const fortio_type *);
  bool               fortio_is_fortio_file(fortio_type * );
  void               fortio_rewind(const fortio_type *fortio);
  const char  *      fortio_filename_ref(const fortio_type * );
  bool               fortio_fmt_file(const fortio_type *);
  long               fortio_ftell( const fortio_type * fortio );
  int                fortio_fseek( fortio_type * fortio , long offset , int whence);
  int                fortio_get_mode( const fortio_type * fortio );
  
#ifdef __cplusplus
}
#endif
#endif

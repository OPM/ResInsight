/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/parser.h>

#include <ert/ecl/ecl_util.h>


/*****************************************************************/
/* The string names for the different ECLIPSE low-level
   types. 
*/


#define ECL_TYPE_NAME_CHAR     "CHAR"
#define ECL_TYPE_NAME_FLOAT    "REAL"
#define ECL_TYPE_NAME_INT      "INTE" 
#define ECL_TYPE_NAME_DOUBLE   "DOUB"
#define ECL_TYPE_NAME_BOOL     "LOGI"
#define ECL_TYPE_NAME_MESSAGE  "MESS" 


#define ECL_PHASE_NAME_OIL   "SOIL"   // SHould match the keywords found in restart file
#define ECL_PHASE_NAME_WATER "SWAT"
#define ECL_PHASE_NAME_GAS   "SGAS"


#define ECL_OTHER_FILE_FMT_PATTERN        "*"
#define ECL_RESTART_FMT_PATTERN           "F[0-9][0-9][0-9][0-9]"
#define ECL_UNIFIED_RESTART_FMT_PATTERN   "FUNRST"
#define ECL_SUMMARY_FMT_PATTERN           "A[0-9][0-9][0-9][0-9]"
#define ECL_UNIFIED_SUMMARY_FMT_PATTERN   "FUNSMRY"
#define ECL_GRID_FMT_PATTERN              "FGRID"
#define ECL_EGRID_FMT_PATTERN             "FEGRID"
#define ECL_INIT_FMT_PATTERN              "FINIT"
#define ECL_RFT_FMT_PATTERN               "FRFT"
#define ECL_DATA_PATTERN                  "DATA"   


#define ECL_OTHER_FILE_UFMT_PATTERN       "*"
#define ECL_RESTART_UFMT_PATTERN          "X[0-9][0-9][0-9][0-9]"
#define ECL_UNIFIED_RESTART_UFMT_PATTERN  "UNRST"
#define ECL_SUMMARY_UFMT_PATTERN          "S[0-9][0-9][0-9][0-9]"
#define ECL_UNIFIED_SUMMARY_UFMT_PATTERN  "UNSMRY" 
#define ECL_GRID_UFMT_PATTERN             "GRID"
#define ECL_EGRID_UFMT_PATTERN            "EGRID"
#define ECL_INIT_UFMT_PATTERN             "INIT" 
#define ECL_RFT_UFMT_PATTERN              "RFT"   



const char * ecl_util_get_phase_name( ecl_phase_enum phase ) {
  switch( phase ) {
  case( ECL_OIL_PHASE ):
    return ECL_PHASE_NAME_OIL;
    break;
  case( ECL_WATER_PHASE ):
    return ECL_PHASE_NAME_WATER;
    break;
  case( ECL_GAS_PHASE ):
    return ECL_PHASE_NAME_GAS;
    break;
  default:
    util_abort("%s: phase enum value:%d not recognized \n",__func__ , phase);
    return NULL;
  }
}





const char * ecl_util_get_type_name( ecl_type_enum ecl_type ) {
  switch (ecl_type) {
  case(ECL_CHAR_TYPE):
    return ECL_TYPE_NAME_CHAR ;
    break;
  case(ECL_FLOAT_TYPE):
    return ECL_TYPE_NAME_FLOAT;
    break;
  case(ECL_DOUBLE_TYPE):
    return ECL_TYPE_NAME_DOUBLE;
    break;
  case(ECL_INT_TYPE):
    return ECL_TYPE_NAME_INT;
    break;
  case(ECL_BOOL_TYPE):
    return ECL_TYPE_NAME_BOOL;
    break;
  case(ECL_MESS_TYPE):
    return ECL_TYPE_NAME_MESSAGE;
    break;
  default:
    util_abort("Internal error in %s - internal eclipse_type: %d not recognized - aborting \n",__func__ , ecl_type);
  }
  return NULL; /* Dummy */
}


ecl_type_enum ecl_util_get_type_from_name( const char * type_name ) {
  ecl_type_enum ecl_type;

  if (strncmp( type_name , ECL_TYPE_NAME_FLOAT , ECL_TYPE_LENGTH) == 0)
    ecl_type = ECL_FLOAT_TYPE;
  else if (strncmp( type_name , ECL_TYPE_NAME_INT , ECL_TYPE_LENGTH) == 0)
    ecl_type = ECL_INT_TYPE;
  else if (strncmp( type_name , ECL_TYPE_NAME_DOUBLE , ECL_TYPE_LENGTH) == 0)
    ecl_type = ECL_DOUBLE_TYPE;
  else if (strncmp( type_name , ECL_TYPE_NAME_CHAR , ECL_TYPE_LENGTH) == 0)
    ecl_type = ECL_CHAR_TYPE;
  else if (strncmp( type_name , ECL_TYPE_NAME_MESSAGE , ECL_TYPE_LENGTH) == 0)
    ecl_type = ECL_MESS_TYPE;
  else if (strncmp( type_name , ECL_TYPE_NAME_BOOL , ECL_TYPE_LENGTH) == 0)
    ecl_type = ECL_BOOL_TYPE;
  else {
    util_abort("%s: unrecognized type name:%s \n",__func__ , type_name);
    ecl_type = -1; /* Dummy */
  }
  return ecl_type;
}


int ecl_util_get_sizeof_ctype_fortio(ecl_type_enum ecl_type) {
  int size = ecl_util_get_sizeof_ctype ( ecl_type );
  if (ecl_type == ECL_CHAR_TYPE)
    size = ECL_STRING_LENGTH  * sizeof(char); 
  
  return size;
}

int ecl_util_get_sizeof_ctype(ecl_type_enum ecl_type) {
  int sizeof_ctype = -1;
  switch (ecl_type) {
  case(ECL_CHAR_TYPE):
    /* 
       One element of character data is a string section of 8
       characters + \0.  Observe that the return value here
       corresponds to the size requirements of ECL_CHAR_TYPE instance
       in memory; on disk the trailing \0 is not stored.
    */
    sizeof_ctype = (ECL_STRING_LENGTH + 1) * sizeof(char); 
    break;
  case(ECL_FLOAT_TYPE):
    sizeof_ctype = sizeof(float);
    break;
  case(ECL_DOUBLE_TYPE):
    sizeof_ctype = sizeof(double);
    break;
  case(ECL_INT_TYPE):
    sizeof_ctype = sizeof(int);
    break;
  case(ECL_BOOL_TYPE):
    sizeof_ctype = sizeof(int); // The ECL_BOOL_TYPE type is internally implemented as an integer - and not a bool.
    break;
  case(ECL_MESS_TYPE):
    sizeof_ctype = sizeof(char);
    break;
  default:
    util_abort("Internal error in %s - internal eclipse_type: %d not recognized - aborting \n",__func__ , ecl_type);
  }
  return sizeof_ctype;
}





/*****************************************************************/



char * ecl_util_alloc_base_guess(const char * path) {
  char * base = NULL;
  stringlist_type * data_files = stringlist_alloc_new( );
  stringlist_type * DATA_files = stringlist_alloc_new( );
  stringlist_select_matching_files( data_files , path , "*.data");
  stringlist_select_matching_files( DATA_files , path , "*.DATA");
  
  if ((stringlist_get_size( data_files ) + stringlist_get_size( DATA_files)) == 1) {
    const char * path_name;

    if (stringlist_get_size( data_files ) == 1)
      path_name = stringlist_iget( data_files , 0 );
    else
      path_name = stringlist_iget( DATA_files , 0 );

    util_alloc_file_components( path_name , NULL , &base , NULL );
  } // Else - found either 0 or more than 1 file with extension DATA - impossible to guess.

  stringlist_free( data_files );
  stringlist_free( DATA_files );

  return base;
}



int ecl_util_filename_report_nr(const char *filename) {
  int report_nr = -1;
  char *ext = strrchr(filename , '.');
  if (ext == NULL) 
    util_abort("%s: can not determine timestep from filename:%s - aborting \n",__func__ , filename);
  
  if (ext[1] == 'X' || ext[1] == 'F' || ext[1] == 'S' || ext[1] == 'A') 
    report_nr = atoi(&ext[2]);
  else 
    util_abort("%s: Filename:%s not recognized - valid extensions: Annnn / Xnnnn / Fnnnn / Snnnn - aborting \n",__func__ , filename);
  
  return report_nr;
}


/*
bool ecl_util_numeric_extension(const char * extension) {
  
  const char digit_ascii_min = 48;
  const char digit_ascii_max = 57;
  bool valid = true;
  int pos = 1;
  while (valid && pos <= 5) {
    char c = extension[pos];
    valid = (valid & (c >= digit_ascii_min && c <= digit_ascii_max));
    if (!valid)
      break;
  }
  
  return valid;
}
*/



ecl_file_enum ecl_util_inspect_extension(const char * ext , bool *_fmt_file, int * _report_nr) {
  ecl_file_enum file_type = ECL_OTHER_FILE;
  bool fmt_file = true;
  int report_nr = -1;
  
  if (strcmp(ext , "UNRST") == 0) {
    file_type = ECL_UNIFIED_RESTART_FILE;
    fmt_file = false;
  } else if (strcmp(ext , "FUNRST") == 0) {
    file_type = ECL_UNIFIED_RESTART_FILE;
    fmt_file = true;
  } else if (strcmp(ext , "UNSMRY") == 0) {
    file_type = ECL_UNIFIED_SUMMARY_FILE;
    fmt_file  = false;
  } else if (strcmp(ext , "FUNSMRY") == 0) {
    file_type = ECL_UNIFIED_SUMMARY_FILE;
    fmt_file  = true;
  } else if (strcmp(ext , "SMSPEC") == 0) {
    file_type = ECL_SUMMARY_HEADER_FILE;
    fmt_file  = false;
  } else if (strcmp(ext , "FSMSPEC") == 0) {
    file_type = ECL_SUMMARY_HEADER_FILE;
    fmt_file  = true;
  } else if (strcmp(ext , "GRID") == 0) {
    file_type = ECL_GRID_FILE;
    fmt_file  = false;
  } else if (strcmp(ext , "FGRID") == 0) {
    file_type = ECL_GRID_FILE;
    fmt_file  = true;
  } else if (strcmp(ext , "EGRID") == 0) {
    file_type = ECL_EGRID_FILE;
    fmt_file  = false;
  } else if (strcmp(ext , "FEGRID") == 0) {
    file_type = ECL_EGRID_FILE;
    fmt_file  = true;
  } else if (strcmp(ext , "INIT") == 0) {
    file_type = ECL_INIT_FILE;
    fmt_file  = false;
  } else if (strcmp(ext , "FINIT") == 0) {
    file_type = ECL_INIT_FILE;
    fmt_file  = true;
  } else if (strcmp(ext , "FRFT") == 0) {
    file_type = ECL_RFT_FILE;
    fmt_file  = true;
  } else if (strcmp(ext , "RFT") == 0) {
    file_type = ECL_RFT_FILE;
    fmt_file  = false;
  } else if (strcmp(ext , "DATA") == 0) {
    file_type = ECL_DATA_FILE;
    fmt_file  = true;  /* Not really relevant ... */
  } else {
    switch (ext[0]) {
    case('X'):
      file_type = ECL_RESTART_FILE;
      fmt_file  = false;
      break;
    case('F'):
      file_type = ECL_RESTART_FILE;
      fmt_file  = true;
      break;
    case('S'):
      file_type = ECL_SUMMARY_FILE;
      fmt_file  = false;
      break;
    case('A'):
      file_type = ECL_SUMMARY_FILE;
      fmt_file  = true;
      break;
    default:
      file_type = ECL_OTHER_FILE;
    }
    if (file_type != ECL_OTHER_FILE) 
      if (!util_sscanf_int(&ext[1] , &report_nr))
        file_type = ECL_OTHER_FILE;
  }
  
  if (_fmt_file != NULL)
    *_fmt_file  = fmt_file;
  
  if (_report_nr != NULL)  
    *_report_nr = report_nr;
  
  return file_type;
}





/** 
  This function takes an eclipse filename as input - looks at the
  extension, and uses that to determine the type of file. In addition
  to the fundamental type, it is also determined whether the file is
  formatted or not, and in the case of summary/restart files, which
  report number this corresponds to.


*/


ecl_file_enum ecl_util_get_file_type(const char * filename, bool *_fmt_file, int * _report_nr) {
  
  char *ext = strrchr(filename , '.');
  if (ext != NULL) {
    ext++;
    return ecl_util_inspect_extension( ext , _fmt_file , _report_nr);
  } else
    return ECL_OTHER_FILE;

}



static const char * ecl_util_get_file_pattern( ecl_file_enum file_type , bool fmt_file ) {
  if (fmt_file) {
    switch( file_type ) {
    case( ECL_OTHER_FILE ):
      return ECL_OTHER_FILE_FMT_PATTERN;  /* '*' */
      break;
    case( ECL_RESTART_FILE ):
      return ECL_RESTART_FMT_PATTERN;
      break;
    case( ECL_UNIFIED_RESTART_FILE ):
      return ECL_UNIFIED_RESTART_FMT_PATTERN;
      break;
    case( ECL_SUMMARY_FILE ):
      return ECL_SUMMARY_FMT_PATTERN;
      break;
    case( ECL_UNIFIED_SUMMARY_FILE ):
      return ECL_UNIFIED_SUMMARY_FMT_PATTERN;
      break;
    case( ECL_GRID_FILE):
      return ECL_GRID_FMT_PATTERN;
      break;
    case( ECL_EGRID_FILE ):
      return ECL_EGRID_FMT_PATTERN;
      break;
    case( ECL_INIT_FILE ):
      return ECL_INIT_FMT_PATTERN;
      break;
    case( ECL_RFT_FILE ):
      return ECL_RFT_FMT_PATTERN;
      break;
    case( ECL_DATA_FILE ):
      return ECL_DATA_PATTERN;
      break;
    default:
      util_abort("%s: No pattern defined for til_type:%d \n",__func__ , file_type);
      return NULL;
    }
  } else {
    switch( file_type ) {
    case( ECL_OTHER_FILE ):
      return ECL_OTHER_FILE_UFMT_PATTERN;  /* '*' */
      break;
    case( ECL_RESTART_FILE ):
      return ECL_RESTART_UFMT_PATTERN;
      break;
    case( ECL_UNIFIED_RESTART_FILE ):
      return ECL_UNIFIED_RESTART_UFMT_PATTERN;
      break;
    case( ECL_SUMMARY_FILE ):
      return ECL_SUMMARY_UFMT_PATTERN;
      break;
    case( ECL_UNIFIED_SUMMARY_FILE ):
      return ECL_UNIFIED_SUMMARY_UFMT_PATTERN;
      break;
    case( ECL_GRID_FILE):
      return ECL_GRID_UFMT_PATTERN;
      break;
    case( ECL_EGRID_FILE ):
      return ECL_EGRID_UFMT_PATTERN;
      break;
    case( ECL_INIT_FILE ):
      return ECL_INIT_UFMT_PATTERN;
      break;
    case( ECL_RFT_FILE ):
      return ECL_RFT_UFMT_PATTERN;
      break;
    default:
      util_abort("%s: No pattern defined for til_type:%d \n",__func__ , file_type);
      return NULL;
    }
  }
}





/**
   Takes an ecl_file_enum variable and returns string with a
   descriptive name of this file type.
*/
const char * ecl_util_file_type_name( ecl_file_enum file_type ) {
  switch (file_type) {
  case(ECL_OTHER_FILE):
    return "ECL_OTHER_FILE";
    break;
  case(ECL_RESTART_FILE):
    return "ECL_RESTART_FILE";
    break;
  case(ECL_UNIFIED_RESTART_FILE):
    return "ECL_UNIFIED_RESTART_FILE";
    break;
  case(ECL_SUMMARY_FILE):
    return "ECL_SUMMARY_FILE";
    break;
  case(ECL_UNIFIED_SUMMARY_FILE):
    return "ECL_UNIFIED_SUMMARY_FILE";
    break;
  case(ECL_SUMMARY_HEADER_FILE):
    return "ECL_SUMMARY_HEADER_FILE";
    break;
  case(ECL_GRID_FILE):
    return "ECL_GRID_FILE";
    break;
  case(ECL_EGRID_FILE):
    return "ECL_EGRID_FILE";
    break;
  case(ECL_INIT_FILE):
    return "ECL_INIT_FILE";
    break;
  case(ECL_RFT_FILE):
    return "ECL_RFT_FILE";
    break;
  case(ECL_DATA_FILE):
    return "ECL_DATA_FILE";
    break;
  default:
    util_abort("%s: internal error type.%d not recognizxed \n",__func__ , file_type);
  }
  return NULL;
}






/**
   This function takes a path, along with a filetype as input and
   allocates a new string with the filename. If path == NULL, the
   filename is allocated without a leading path component.

   If the flag 'must_exist' is set to true the function will check
   with the filesystem if the file actually exists; if the file does
   not exist NULL is returned.
*/

static char * ecl_util_alloc_filename_static(const char * path, const char * base , ecl_file_enum file_type , bool fmt_file, int report_nr, bool must_exist) {
  char * filename;
  char * ext;
  switch (file_type) {
  case(ECL_RESTART_FILE):
    if (fmt_file)
      ext = util_alloc_sprintf("F%04d" , report_nr);
    else
      ext = util_alloc_sprintf("X%04d" , report_nr);
    break;

  case(ECL_UNIFIED_RESTART_FILE):
    if (fmt_file)
      ext = util_alloc_string_copy("FUNRST");
    else
      ext = util_alloc_string_copy("UNRST");
    break;

  case(ECL_SUMMARY_FILE):
    if (fmt_file)
      ext = util_alloc_sprintf("A%04d" , report_nr);
    else
      ext = util_alloc_sprintf("S%04d" , report_nr);
    break;
    
  case(ECL_UNIFIED_SUMMARY_FILE):
    if (fmt_file)
      ext = util_alloc_string_copy("FUNSMRY");
    else
      ext = util_alloc_string_copy("UNSMRY");
    break;

  case(ECL_SUMMARY_HEADER_FILE):
    if (fmt_file) 
      ext = util_alloc_string_copy("FSMSPEC");
    else
      ext = util_alloc_string_copy("SMSPEC");
    break;

  case(ECL_GRID_FILE):
    if (fmt_file) 
      ext = util_alloc_string_copy("FGRID");
    else
      ext = util_alloc_string_copy("GRID");
    break;
    
  case(ECL_EGRID_FILE):
    if (fmt_file) 
      ext = util_alloc_string_copy("FEGRID");
    else
      ext = util_alloc_string_copy("EGRID");
    break;

  case(ECL_INIT_FILE):
    if (fmt_file) 
      ext = util_alloc_string_copy("FINIT");
    else
      ext = util_alloc_string_copy("INIT");
    break;

  case(ECL_RFT_FILE):
    if (fmt_file) 
      ext = util_alloc_string_copy("FRFT");
    else
      ext = util_alloc_string_copy("RFT");
    break;

  case(ECL_DATA_FILE):
    ext = util_alloc_string_copy("DATA");
    break;
    
  default:
    util_abort("%s: Invalid input file_type to ecl_util_alloc_filename - aborting \n",__func__);
    /* Dummy to shut up compiler */
    ext        = NULL;
  }

  filename = util_alloc_filename(path , base , ext);
  free(ext);
  
  if (must_exist) {
    if (!util_file_exists( filename )) {
      free(filename);
      filename = NULL;
    }
  }
  
  return filename;
}


char * ecl_util_alloc_filename(const char * path, const char * base , ecl_file_enum file_type , bool fmt_file, int report_nr) {
  return ecl_util_alloc_filename_static(path , base , file_type ,fmt_file , report_nr , false);
}



char * ecl_util_alloc_exfilename(const char * path, const char * base , ecl_file_enum file_type , bool fmt_file, int report_nr) {
  return ecl_util_alloc_filename_static(path , base , file_type ,fmt_file , report_nr , true);
}


/**
   This function will first try if the 'fmt_file' file exists, and
   then subsequently the !fmt_file version. If neither can be found it
   will return NULL.
*/

char * ecl_util_alloc_exfilename_anyfmt(const char * path, const char * base , ecl_file_enum file_type , bool fmt_file_first , int report_nr) {
  
  char * filename = ecl_util_alloc_filename( path , base , file_type , fmt_file_first , report_nr);
  if (!util_file_exists( filename )) {
    free( filename );
    filename = ecl_util_alloc_filename( path , base , file_type , !fmt_file_first , report_nr);
  }

  if (! util_file_exists(filename)) {
    util_safe_free( filename );  
    filename = NULL;
  }
  
  return filename;
}


/**
   This function assumes that:

    o Both files are of the same type (i.e. both summary files) (this
      is not checked for).

    o Both files are of type WITH a nnnn number at the end, the
      function will fail hard in ecl_util_filename_report_nr() if
      this is not the case.

*/


int ecl_util_fname_report_cmp(const void *f1, const void *f2) {
  
  int t1 = ecl_util_filename_report_nr( (const char *) f1 );
  int t2 = ecl_util_filename_report_nr( (const char *) f2 );

  if (t1 < t2)
    return -1;
  else if (t1 > t2)
    return 1;
  else
    return 0;

}

/**
   This function will scan the directory @path (or cwd if @path == NULL) 
   for all ECLIPSE files of type @file_type. If base == NULL it will use 
   '*' as pattern for basename. If file_type == ECL_OTHER_FILE it will 
   use '*' as pattern for the extension (as a consequence files which do 
   not originate from ECLIPSE will also be included).

   The stringlist will be cleared before the actual matching process
   starts.
*/



int ecl_util_select_filelist( const char * path , const char * base , ecl_file_enum file_type , bool fmt_file , stringlist_type * filelist) {
  char       * file_pattern;
  char       * base_pattern;
  const char * extension = ecl_util_get_file_pattern( file_type , fmt_file );
  if (base == NULL)
    base_pattern = util_alloc_string_copy( "*" );
  else
    base_pattern = util_alloc_string_copy( base );
  
  file_pattern = util_alloc_filename( NULL , base_pattern , extension );
  stringlist_select_matching_files( filelist , path , file_pattern );
  if ((file_type == ECL_SUMMARY_FILE) || (file_type == ECL_RESTART_FILE))
    stringlist_sort( filelist , ecl_util_fname_report_cmp );
  
  free( base_pattern );
  free( file_pattern );
  return stringlist_get_size( filelist );
}


bool ecl_util_unified_file(const char *filename) {
  int report_nr;
  ecl_file_enum file_type;
  bool fmt_file;
  file_type = ecl_util_get_file_type(filename , &fmt_file , &report_nr);
  
  if ((file_type == ECL_UNIFIED_RESTART_FILE) || (file_type == ECL_UNIFIED_SUMMARY_FILE))
    return true;
  else
    return false;
}


bool ecl_util_fmt_file(const char *filename) {
  /*const int min_size = 32768;*/
  const int min_size = 256; /* Veeeery small */
  
  int report_nr;
  ecl_file_enum file_type;

  bool fmt_file;
  if (util_file_exists(filename)) {
    file_type = ecl_util_get_file_type(filename , &fmt_file , &report_nr);
    if (file_type == ECL_OTHER_FILE) {
      if (util_file_size(filename) > min_size)
        fmt_file = util_fmt_bit8(filename);
      else 
        util_abort("%s: sorry could not determine formatted|unformatted of file:%s file_size:%d - aborting \n",__func__ , filename , util_file_size(filename));
    }
  } else {
    file_type = ecl_util_get_file_type(filename , &fmt_file , &report_nr);
    if (file_type == ECL_OTHER_FILE) 
      util_abort("%s: sorry could not determine formatted|unformatted of file:%s - aborting \n",__func__ , filename);
  }
  
  return fmt_file;
}



/*****************************************************************/




/**
 This function copies size elements from _src_data to target_data. If
 src_type == target_type the copy is a simple memcpy, otherwise the
 appropriate numerical conversion is applied.
*/

void ecl_util_memcpy_typed_data(void *_target_data , const void * _src_data , ecl_type_enum target_type , ecl_type_enum src_type, int size) {
  int i;

  if (target_type == src_type) 
    memcpy(_target_data , _src_data , size * ecl_util_get_sizeof_ctype(src_type));
  else {
    switch (target_type) {
    case(ECL_DOUBLE_TYPE):
      {
        double * target_data = (double *) _target_data;
        switch(src_type) {
        case(ECL_FLOAT_TYPE):
          util_float_to_double(target_data , (const float *) _src_data , size);
          break;
        case(ECL_INT_TYPE):
          for (i = 0; i < size; i++) 
            target_data[i] = ((int *) _src_data)[i];
          break;
        default:
          util_abort("%s: double type can only load from int/float/double - aborting \n",__func__);
        }
        break;
      }
    case(ECL_FLOAT_TYPE):
      {
        float * target_data = (float *) _target_data;
        switch(src_type) {
        case(ECL_FLOAT_TYPE):
          util_double_to_float(target_data , (const double *) _src_data , size);
          break;
        case(ECL_INT_TYPE):
          for (i = 0; i < size; i++) 
            target_data[i] = ((int *) _src_data)[i];
          break;
        default:
          util_abort("%s: float type can only load from int/float/double - aborting \n",__func__);
        }
        break;
      }
    default:
      util_abort("%s con not convert %d -> %d \n",__func__ , src_type , target_type);
    }
  }
}


/*
  The stringlist will be cleared before the actual matching process
  starts. Observe that in addition to the @path input parameter the
  @base input can contain an embedded path component.
*/

void ecl_util_alloc_summary_data_files(const char * path , const char * base , bool fmt_file , stringlist_type * filelist) {
  char  * unif_data_file = ecl_util_alloc_exfilename(path , base , ECL_UNIFIED_SUMMARY_FILE , fmt_file , -1);
  int files = ecl_util_select_filelist( path , base , ECL_SUMMARY_FILE , fmt_file , filelist);

  if ((files > 0) && (unif_data_file != NULL)) {
    /* 
       We have both a unified file AND a list of files: BASE.S0000,
       BASE.S0001, BASE.S0002, ..., must check which is newest and
       load accordingly.
    */
    bool unified_newest = true;
    int file_nr = 0;
    while (unified_newest && (file_nr < files)) {
      if (util_file_difftime( stringlist_iget(filelist , file_nr) , unif_data_file ) > 0) 
        unified_newest = false;
      file_nr++;
    }
    
    if (unified_newest) {
      stringlist_clear( filelist ); /* Clear out all the BASE.Snnnn selections. */
      stringlist_append_copy( filelist , unif_data_file );
    } 
  } else if (unif_data_file != NULL) {
    /* Found a unified summary file :  Clear out all the BASE.Snnnn selections. */
    stringlist_clear( filelist );      /* Clear out all the BASE.Snnnn selections. */
    stringlist_append_copy( filelist , unif_data_file );
  } 
  util_safe_free( unif_data_file );
}



/**
   This routine allocates summary header and data files from a
   directory, and return them by reference; path and base are
   input. If the function can not find BOTH a summary header file and
   summary data it will return false and not update the reference
   variables.

   For the header file there are two possible files:

     1. X.FSMSPEC
     2. X.SMSPEEC

   For the data there are four different possibilities:

     1. X.A0001, X.A0002, X.A0003, ... 
     2. X.FUNSMRY
     3. X.S0001, X.S0002, X.S0003, ... 
     4. X.UNSMRY
  
   In principle a directory can contain all different (altough that is
   probably not typical). The algorithm is a a two step algorithm:

     1. Determine wether to use X.FSMSPEC or X.SMSPEC based on which
        is the newest. This also implies a decision of wether to use
        formatted, or unformatted filed.
 
     2. Use formatted or unformatted files according to 1. above, and
        then choose either a list of files or unified files according
        to which is the newest.
   
   This algorithm should work in most practical cases, but it is
   surely possible to fool it. 
*/


bool ecl_util_alloc_summary_files(const char * path , const char * _base , const char * ext , char ** _header_file , stringlist_type * filelist) {
  bool    fmt_input      = false;
  bool    fmt_set        = false;
  bool    fmt_file       = true; 
  bool    unif_input     = false;
  bool    unif_set       = false;

  
  char  * header_file    = NULL;
  char  * base;

  *_header_file = NULL;

  /* 1: We start by inspecting the input extension and see if we can
     learn anything about formatted/unformatted and
     unified/non-unified from this. The input extension can be NULL,
     in which case we learn nothing.
  */

  if (_base == NULL)
    base = ecl_util_alloc_base_guess(path);
  else
    base = (char *) _base;
  
  if (ext != NULL) {
    ecl_file_enum input_type;

    {
      char * test_name = util_alloc_filename( NULL , base, ext );
      input_type = ecl_util_get_file_type( test_name , &fmt_input , NULL);
      free( test_name );
    }

    if ((input_type != ECL_OTHER_FILE) && (input_type != ECL_DATA_FILE)) {
      /* 
         The file has been recognized as a file type from which we can
         at least infer formatted/unformatted inforamtion.
      */
      fmt_set = true;
      switch (input_type) {
      case(ECL_SUMMARY_FILE):
      case(ECL_RESTART_FILE):
        unif_input = false;
        unif_set   = true;
        break;
      case(ECL_UNIFIED_SUMMARY_FILE):
      case(ECL_UNIFIED_RESTART_FILE):
        unif_input = true;
        unif_set   = true;
        break;
      default:  /* Nothing wrong with this */
        break;
      }
    } 
  }
  
  
  /*
    2: We continue by looking for header files. 
  */
  
  {
    char * fsmspec_file = ecl_util_alloc_exfilename(path , base , ECL_SUMMARY_HEADER_FILE , true  , -1);
    char *  smspec_file = ecl_util_alloc_exfilename(path , base , ECL_SUMMARY_HEADER_FILE , false , -1);
    
    if ((fsmspec_file == NULL) && (smspec_file == NULL))   /* Neither file exists */
      return false;

    
    if (fmt_set)  /* The question of formatted|unformatted has already been settled based on the input filename. */
      fmt_file = fmt_input;
    else {
      if ((fsmspec_file != NULL) && (smspec_file != NULL)) {   /* Both fsmspec and smspec exist - we take the newest. */
        if (util_file_difftime(fsmspec_file , smspec_file) < 0) 
          fmt_file = true;
        else 
          fmt_file = false;
      } else {                                                /* Only one of fsmspec / smspec exists */
        if (fsmspec_file != NULL)
          fmt_file = true;
        else
          fmt_file = false;
      }
    }
    
    if (fmt_file) {
      header_file = fsmspec_file;
      util_safe_free( smspec_file );
    } else {
      header_file = smspec_file;
      util_safe_free( fsmspec_file );
    }

    if (header_file == NULL)
      return false;                                           /* If you insist on e.g. unformatted and only fsmspec exists - no results for you. */
  }
  
  

  /* 
     3: OK - we have found a SMSPEC / FMSPEC file - continue to look for
     XXX.Snnnn / XXX.UNSMRY files.
  */

  if (unif_set) { /* Based on the input file we have inferred whether to look for unified or
                     non-unified input files. */

    if ( unif_input ) {
      char  * unif_data_file = ecl_util_alloc_exfilename(path , base , ECL_UNIFIED_SUMMARY_FILE , fmt_file , -1);
      if (unif_data_file != NULL) {
        stringlist_append_copy( filelist , unif_data_file );
        free( unif_data_file );
      }
    } else 
      ecl_util_select_filelist( path , base , ECL_SUMMARY_FILE , fmt_file , filelist);
  } else          
    ecl_util_alloc_summary_data_files( path , base , fmt_file , filelist );
    
  if (_base == NULL)
    free(base);

  *_header_file    = header_file;
    
  return (stringlist_get_size(filelist) > 0) ? true : false;
}




//const char * ecl_util_get_extension( ecl_file_enum_type file_type , bool fmt_file) {
//
//}
//
//
///** 
//    Based on the ordinary util_alloc_file_components function, but if
//    file_type is used specify the type file we know what extension to
//    expect, and an optional "." can potentially be included as part of
//    the filename. 
//*/
//
//void ecl_util_alloc_file_components( const char * file, ecl_file_enum file_type , char **_path , char **_basename , char **_extension) {
//  
//}





void ecl_util_alloc_restart_files(const char * path , const char * _base , char *** _restart_files , int * num_restart_files , bool * _fmt_file , bool * _unified) {
  
  util_exit("Function:%s currently not implemented - sorry \n",__func__);

  //char * base = NULL;
  //if (_base == NULL)
  //  base = ecl_util_alloc_base_guess(path);
  //else
  //  base = (char *) _base;
  //{
  //  int num_F_files;
  //  int num_X_files;
  //
  //  char *  unrst_file  = ecl_util_alloc_filename(path , base , ECL_UNIFIED_RESTART_FILE , false , -1);  
  //  char *  funrst_file = ecl_util_alloc_filename(path , base , ECL_UNIFIED_RESTART_FILE , true  , -1);
  //  char *  unif_file   = NULL; 
  //
  //  char ** F_files     = ecl_util_alloc_scandir_filelist(path , base , ECL_RESTART_FILE , true  , &num_F_files); 
  //  char ** X_files     = ecl_util_alloc_scandir_filelist(path , base , ECL_RESTART_FILE , false , &num_X_files); 
  //  char *  FX_file      = NULL;
  //  char *  final_file; 
  //
  //  /*
  //    Ok now we have formatted/unformatted unified and not
  //    unified: Time to check what exists in the filesystem, and which
  //    is the newest.
  //  */
  //  unif_file = util_newest_file(unrst_file , funrst_file);
  //  
  //  if (num_F_files > 0 || num_X_files > 0) {    
  //    if (num_F_files > 0 && num_X_files > 0) {
  //      /* 
  //         We have both a list of .Fnnnn and a list of .Xnnnn files; if
  //         the length of lists is not equal we take the longest,
  //         otherwise we compare the dates of the last files in the
  //         list. 
  //      */
  //      if (num_F_files == num_X_files) {
  //        FX_file = util_newest_file( F_files[num_F_files - 1] , X_files[num_X_files - 1]);
  //      } else if (num_F_files > num_X_files)
  //        FX_file = F_files[num_F_files - 1];
  //      else
  //        FX_file = X_files[num_X_files - 1];
  //    } else if (num_F_files > 0)
  //      FX_file = F_files[num_F_files - 1];
  //    else
  //      FX_file = X_files[num_X_files - 1];
  //
  //    if (unif_file != NULL)
  //      final_file = util_newest_file(unif_file , FX_file);
  //    else
  //      final_file = FX_file;
  //  } else
  //    final_file = unif_file;
  //    
  //  
  //  if (final_file == NULL) 
  //    util_abort("%s: could not find any restart data in %s/%s \n",__func__ , path , base);
  //  
  //
  //  /* 
  //     Determine type of final_file. Thois block is where the return
  //     values are actually set.
  //  */
  //  {
  //    char ** restart_files;
  //    bool fmt_file , unified;
  //    ecl_file_enum file_type;
  //    
  //    ecl_util_get_file_type( final_file , &file_type , &fmt_file , NULL);
  //    if (file_type == ECL_UNIFIED_RESTART_FILE) {
  //      *num_restart_files = 1;
  //      restart_files = util_malloc(sizeof * restart_files, __func__);
  //      restart_files[0] = util_alloc_string_copy( final_file );
  //      unified = true;
  //    } else {
  //      restart_files = ecl_util_alloc_scandir_filelist( path , base , ECL_RESTART_FILE , fmt_file , num_restart_files);
  //      unified = false;
  //    }
  //    *_restart_files = restart_files;
  //
  //    if (_fmt_file != NULL) *_fmt_file = fmt_file;
  //    if (_unified  != NULL) *_unified  = unified; 
  //  }
  //
  //  util_free_stringlist(F_files , num_F_files);
  //  util_free_stringlist(X_files , num_X_files);
  //  free(unrst_file);
  //  free(funrst_file);
  //}
  //
  //if (_base == NULL)
  //  free(base);
}





/**
This little function escapes eclipse keyword names so that they can be
safely used as filenames, i.e for instance the substitution:

   1/FVFGAS -> 1-FVFGAS

The escape process is done 'in-place' memory-wise.
*/
void ecl_util_escape_kw(char * kw) {
  int index;
  for (index = 0; index < strlen(kw); index++) {
    switch (kw[index]) {
    case('/'):
      kw[index] = '-';
      break;
    case('\\'):
      kw[index] = '-';
      break;
    }
  }
}




/**
   Will return -1 for an unrecognized month name.
*/

static int ecl_util_get_month_nr__(const char * _month_name) {
  int month_nr = -1;
  char * month_name = util_alloc_string_copy(_month_name);
  util_strupr(month_name);
  
  if (strncmp(month_name , "JAN" , 3)      == 0) 
    month_nr = 1;
  else if (strncmp(month_name , "FEB" , 3) == 0) 
    month_nr = 2;
  else if (strncmp(month_name , "MAR" , 3) == 0) 
    month_nr = 3;
  else if (strncmp(month_name , "APR" , 3) == 0) 
    month_nr = 4;
  else if (strncmp(month_name , "MAI" , 3) == 0) 
    month_nr = 5;
  else if (strncmp(month_name , "MAY" , 3) == 0) 
    month_nr = 5;
  else if (strncmp(month_name , "JUN" , 3) == 0) 
    month_nr = 6;
  else if (strncmp(month_name , "JUL" , 3) == 0) 
    month_nr = 7;
  else if (strncmp(month_name , "JLY" , 3) == 0)   /* ECLIPSE ambigus on July. */
    month_nr = 7;
  else if (strncmp(month_name , "AUG" , 3) == 0) 
    month_nr = 8;
  else if (strncmp(month_name , "SEP" , 3) == 0) 
    month_nr = 9;
  else if (strncmp(month_name , "OCT" , 3) == 0) 
    month_nr = 10;
  else if (strncmp(month_name , "OKT" , 3) == 0) 
    month_nr = 10;
  else if (strncmp(month_name , "NOV" , 3) == 0) 
    month_nr = 11;
  else if (strncmp(month_name , "DEC" , 3) == 0) 
    month_nr = 12;
  else if (strncmp(month_name , "DES" , 3) == 0) 
    month_nr = 12;
  free(month_name);
  return month_nr;
}


int ecl_util_get_month_nr(const char * month_name) {
  int month_nr = ecl_util_get_month_nr__(month_name);
  if (month_nr < 0) 
    util_abort("%s: %s not a valid month name - aborting \n",__func__ , month_name);
  
  return month_nr;
}




/*
  I have *intentionally* dived straight at the problem of extracting
  the start_date; otherwise one might quite quickly end up with a
  half-baked DATA-file parser. I think that path leads straight to an
  asylum. But of course - not many points are awarded for pointing out
  that this parsing is extremly ugly.

    ECLIPSE100 has default date: 1. of january 1983.
    ECLIPSE300 has default date: 1. of january 1990.

  They don't have much style those fuckers at Schlum ...
*/


time_t ecl_util_get_start_date(const char * data_file) { 
  parser_type * parser = parser_alloc(" \t\r\n" , "\"\'" , NULL , NULL , "--" , "\n");
  time_t start_date  = -1;
  FILE * stream      = util_fopen(data_file , "r");
  char * buffer;
  
  if (!parser_fseek_string( parser , stream , "START" , true , true))   /* Seeks case insensitive. */
    util_abort("%s: sorry - could not find START in DATA file %s \n",__func__ , data_file);
  
  {
    long int start_pos = ftell( stream );
    int buffer_size;

    /* Look for terminating '/' */
    if (!parser_fseek_string( parser , stream , "/" , false , true))
      util_abort("%s: sorry - could not find \"/\" termination of START keyword in data_file: \n",__func__ , data_file);
    
    buffer_size = (ftell(stream) - start_pos)  ;
    buffer = util_calloc( buffer_size + 1 , sizeof * buffer  );
    fseek( stream , start_pos , SEEK_SET);
    util_fread( buffer , sizeof * buffer , buffer_size ,stream ,  __func__);
    buffer[buffer_size] = '\0';
  }
  
  
  {
    stringlist_type * tokens = parser_tokenize_buffer( parser , buffer , true );
    int day, year, month_nr;
    if ( util_sscanf_int( stringlist_iget( tokens , 0 ) , &day)   &&   util_sscanf_int( stringlist_iget(tokens , 2) , &year)) {
      month_nr   = ecl_util_get_month_nr(stringlist_iget( tokens , 1));
      start_date = util_make_date(day , month_nr , year );
    } else
      util_abort("%s: failed to parse DAY MONTH YEAR from : \"%s\" \n",__func__ , buffer);
    stringlist_free( tokens );
  }
  
  free( buffer );
  parser_free( parser );
  fclose(stream);
  
  return start_date;
}



int ecl_util_get_num_cpu(const char * data_file) { 
  parser_type * parser = parser_alloc(" \t\r\n" , "\"\'" , NULL , NULL , "--" , "\n");
  int num_cpu        = 1;
  FILE * stream      = util_fopen(data_file , "r");
  char * buffer;
  
  if (parser_fseek_string( parser , stream , "PARALLEL" , true , true)) {  /* Seeks case insensitive. */
    long int start_pos = ftell( stream );
    int buffer_size;

    /* Look for terminating '/' */
    if (!parser_fseek_string( parser , stream , "/" , false , true))
      util_abort("%s: sorry - could not find \"/\" termination of PARALLEL keyword in data_file: \n",__func__ , data_file);
    
    buffer_size = (ftell(stream) - start_pos)  ;
    buffer = util_calloc( buffer_size + 1  , sizeof * buffer );
    fseek( stream , start_pos , SEEK_SET);
    util_fread( buffer , sizeof * buffer , buffer_size ,stream ,  __func__);
    buffer[buffer_size] = '\0';
  
    {
      stringlist_type * tokens = parser_tokenize_buffer( parser , buffer , true );
      int i;
      char * item = NULL;
      for (i=0; i < stringlist_get_size( tokens ); i++) {
        item = util_realloc_string_copy( item , stringlist_iget( tokens , i ));
        util_strupr( item );
        if ( util_string_equal( item , "DISTRIBUTED" )) {
          num_cpu = atoi( stringlist_iget( tokens , i - 1));
          break;
        }
      }
      free( item );  
      stringlist_free( tokens );
    }
    free( buffer );
  }

  parser_free( parser );
  fclose(stream);
  return num_cpu;
}


/**
   This function checks that all the characters in the input @basename
   are either lowercase, or uppercase. If presented with a mixed-case
   basename the multimillion $$ program ECLIPSE will die a horrible
   death - impressive ehh?!
*/


bool ecl_util_valid_basename( const char * basename ) {
  int upper_count = 0;
  int lower_count = 0;
  int index;

  for (index = 0; index < strlen( basename ); index++) {
    int c = basename[index];
    if (isalpha(c)) {
      if (isupper(c))
        upper_count++;
      else
        lower_count++;
    }
  }

  if ((lower_count * upper_count) != 0)
    return false;
  else
    return true;
}


/*
  Will append time_t values corresponding to the first day in every
  month in the open interval (start_date , end_date). Iff start_date
  corresponds to the first date in a month the list will start with
  start_date, otherwise the list will start with the first day in the
  month following after start_date.
  
  If end_date corresponds to the first day of the month the list will
  end with end_date, otherwise it will ende with the first day in the
  month prior to end_date:

     (1,1,2000)  , (10,3,2000) => {(1,1,2000) , (1,2,2000) , (1,3,2000) }
     (10,1,2000) , (1,4,2000)  => {(1,2,2000) , (1,3,2000) , (1,4,2000) }

  All time_t values added to the date list will be pure dates,
  i.e. the time part will be 00:00:00; that also applies to start_date
  and end_date where possible time parts will be normalized away prior
  to insertion.  
*/


void ecl_util_append_month_range( time_t_vector_type * date_list , time_t start_date , time_t end_date , bool force_append_end) {
  start_date = util_make_pure_date( start_date );
  end_date   = util_make_pure_date( end_date );

  if (util_is_first_day_in_month( start_date))
    time_t_vector_append( date_list , start_date );
  
  {
    time_t current_date = start_date;
    while (true) {
      int month,year;
      util_set_date_values( current_date , NULL , &month , &year);
      if (month == 12) {
        month = 1;
        year += 1;
      } else
        month += 1;
      
      current_date = util_make_date( 1 , month , year );
      if (current_date < end_date)
        time_t_vector_append( date_list , current_date );
      else {
        if (current_date == end_date)
          time_t_vector_append( date_list , current_date );
        else if (force_append_end)
          time_t_vector_append( date_list , end_date );
        break;
      }
    }
  }
}



void ecl_util_init_month_range( time_t_vector_type * date_list , time_t start_date , time_t end_date) {
  time_t_vector_reset( date_list );
  if (!util_is_first_day_in_month( start_date ))
    time_t_vector_append( date_list , util_make_pure_date(start_date));
  
  ecl_util_append_month_range( date_list , start_date , end_date , true );
}


/*****************************************************************/
/* Small functions to support enum introspection. */

#ifdef HAVE_FORK
const char * ecl_util_file_enum_iget( int index, int * value) {
  return util_enum_iget( index , ECL_FILE_ENUM_SIZE , (const util_enum_element_type []) { ECL_FILE_ENUM_DEFS }, value);
}

const char * ecl_util_phase_enum_iget( int index, int * value) {
  return util_enum_iget( index , ECL_PHASE_ENUM_SIZE , (const util_enum_element_type []) { ECL_PHASE_ENUM_DEFS }, value);
}

const char * ecl_util_type_enum_iget( int index, int * value) {
  return util_enum_iget( index , ECL_TYPE_ENUM_SIZE , (const util_enum_element_type []) { ECL_TYPE_ENUM_DEFS }, value);
}
#endif

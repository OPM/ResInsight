/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_report.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/template.h>
#include <ert/util/latex.h>
#include <ert/util/subst_list.h>

#include <ert/enkf/ert_report.h>

/*
  The LATEX_PATH_FMT is a format string for where the latex
  compilation should take place. This format string will be passed to
  the mkdtemp() function, where the last 'XXXXXX' characters will be
  replaced with random characters. 

  The mkdtemp() function will only create one - directory, i.e. the
  format string "/tmp/latex/XXXXXX" will fail unless the /tmp/latex
  directory already exists prior to the call.  
*/
#define LATEX_PATH_FMT "/tmp/latex-XXXXXX"  
#define ERT_REPORT_TYPE_ID 919191653


/*
  Observe that the concept of target and source can be a bit
  misleading in this implementation. Starting with a latex template
  there is a three step process before a complete pdf report is ready:


    1. [templates/report1.tex] We start with a latex template file,
         this template is instantiated to a new latex file with
         case/field specific values substituted in.

    2. [/tmp/latex-XXXXX/my_field.tex] The latex file which is
         actually compiled is located in a /tmp directory; the file
         can also optionally undergo a namechange from the original
         template name. 

         If the compilation suceeds the resultfile (a pdf file) is
         copied to the target directory, and the temporary directory
         used for compilation is removed. If the compilation fails the
         compilation directory is left dangling.

    3. [reports/case/my_field.pdf] The final product.

  I.e. in the second step the latex file in the temporary directory
  serves both as a source file and as a target file. 
*/


struct ert_report_struct {
  UTIL_TYPE_ID_DECLARATION;
  template_type * template;

  char          * latex_basename;
  char          * input_path;
  bool            with_xref;
  bool            ignore_errors;

  char          * target_file;   // Set by the latex class AFTER a successfull compilation
  char          * work_path;     
};


static UTIL_SAFE_CAST_FUNCTION( ert_report , ERT_REPORT_TYPE_ID )

static void ert_report_set_work_path( ert_report_type * report ) {
  report->work_path = util_realloc_string_copy( report->work_path , LATEX_PATH_FMT );
  if (mkdtemp( report->work_path ) == NULL)
    util_abort("%s: failed to create runpath for LaTeX \n",__func__);
}

ert_report_type * ert_report_alloc( const char * source_file , const char * target_file ) {
  ert_report_type * report = util_malloc( sizeof * report );
  UTIL_TYPE_ID_INIT(report , ERT_REPORT_TYPE_ID);
  report->template = template_alloc( source_file , false , NULL );
  
  {
    char * input_path;
    if (target_file == NULL) {
      util_alloc_file_components( source_file , &input_path , &report->latex_basename , NULL);
    } else {
      util_alloc_file_components( source_file , &input_path , NULL , NULL);
      util_alloc_file_components( target_file , NULL , &report->latex_basename , NULL);
    }
    
    if (input_path == NULL)
      report->input_path = util_alloc_cwd();
    else 
      report->input_path = util_alloc_abs_path(input_path);
    
    util_safe_free( input_path );
  }
  
  report->ignore_errors = true;
  report->target_file = NULL;
  report->work_path = NULL;
  ert_report_set_work_path( report );
  return report;
}

static void ert_report_clear( ert_report_type * ert_report , bool mkdir_work_path) {
  if (ert_report->work_path != NULL)
    util_clear_directory( ert_report->work_path , true , true );

  if (mkdir_work_path)
    ert_report_set_work_path( ert_report );
  else
    ert_report->work_path = NULL;
}




bool ert_report_create( ert_report_type * ert_report , int latex_timeout , const subst_list_type * context , const char * plot_path , const char * target_path ) {
  bool   success;
  {
    char * latex_file = util_alloc_filename( ert_report->work_path , ert_report->latex_basename , LATEX_EXTENSION );
    template_instantiate( ert_report->template , latex_file , context , true );
    {
      latex_type * latex = latex_alloc( latex_file , true );
      char * target_file = util_alloc_filename( target_path , ert_report->latex_basename , "pdf");

      latex_set_timeout( latex , latex_timeout );
      latex_link_directory_content( latex , ert_report->input_path );
      latex_link_directory_content( latex , plot_path );
      latex_set_target_file( latex , target_file );
      success = latex_compile( latex , ert_report->ignore_errors , ert_report->with_xref , true);
      
      if (success) 
        ert_report->target_file = util_realloc_string_copy( ert_report->target_file , latex_get_target_file( latex ));

      free( target_file );
      latex_free( latex );
    }
    free( latex_file );
  }
  /*
    Unfortunately the latex class will return lot's of incorrect
    succes == True.
  */
  //if (success) 
  //  ert_report_clear( ert_report , true);
  return success;
}



void ert_report_free( ert_report_type * ert_report ) {

  template_free( ert_report->template );
  free( ert_report->latex_basename );
  free( ert_report->input_path );
  free( ert_report->target_file );
  ert_report_clear( ert_report , false );
  
  free( ert_report );
}


void ert_report_free__(void * arg) {
  ert_report_type * ert_report = ert_report_safe_cast( arg );
  ert_report_free( ert_report );
}


const char * ert_report_get_basename( const ert_report_type * ert_report ) {
  return ert_report->latex_basename;
}

const char * ert_report_get_work_path( const ert_report_type * ert_report ) {
  return ert_report->work_path;
}

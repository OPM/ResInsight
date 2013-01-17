/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'main.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <util.h>
#include <hash.h>
#include <stringlist.h>

#include <config.h>

#include <ext_joblist.h>
#include <local_driver.h>
#include <lsf_driver.h>

#include <enkf_fs.h>
#include <enkf_main.h>
#include <enkf_types.h>
#include <enkf_sched.h>
#include <enkf_tui_main.h>
#include <ert_build_info.h>
#include <site_config_file.h>

void text_splash() {
  const int usleep_time = 1000;
  int i;
  {
#include "ERT.h"
    printf("\n\n");
    for (i = 0; i < SPLASH_LENGTH; i++) {
      printf("%s\n" , splash_text[i]);
      util_usleep(usleep_time);
    }
    printf("\n\n");

    sleep(1);
#undef SPLASH_LENGTH
  }
}


void devel_warning() {
#ifdef DEVEL_VERSION
  printf("\n");
  printf("  ***************************************************************\n");
  printf("  ** You have started a development version of ERT. If you are **\n");
  printf("  ** not an advanced user, it might be better to use a stable  **\n");
  printf("  ** version which has been better tested. The stable version  **\n");
  printf("  ** should be available with the command:                     **\n");
  printf("  **                                                           **\n");
  printf("  **      bash%% ert config_file                                **\n");
  printf("  **                                                           **\n");
  printf("  ***************************************************************\n");
#endif
}


/*
  SVN_VERSION and COMPILE_TIME_STAMP are env variables set by the
  makefile. Will exit if the config file does not exist.
*/
void enkf_welcome(const char * config_file) {
  if (util_file_exists( config_file )) {
    char * svn_version           = util_alloc_sprintf("svn version..........: %s \n",SVN_VERSION);
    char * compile_time          = util_alloc_sprintf("Compile time.........: %s \n",COMPILE_TIME_STAMP);
    char * abs_path              = util_alloc_realpath( config_file );
    char * config_file_msg       = util_alloc_sprintf("Configuration file...: %s \n",abs_path);
    
    /* This will be printed if/when util_abort() is called on a later stage. */
    /* The svn_version and compile_time are added with the functione enkf_main_init_debug(). */
    util_abort_append_version_info(config_file_msg);
    
    free(config_file_msg);
    free(abs_path);
    free(svn_version);
    free(compile_time);
  } else util_exit(" ** Sorry: can not locate configuration file: %s \n\n" , config_file);
}


void enkf_usage() {
  printf("\n");
  printf(" *********************************************************************\n");
  printf(" **                                                                 **\n");
  printf(" **                            E R T                                **\n");
  printf(" **                                                                 **\n");
  printf(" **-----------------------------------------------------------------**\n");
  printf(" ** You have sucessfully started the ert program developed at       **\n");
  printf(" ** Statoil. Before you can actually start using the program, you   **\n");
  printf(" ** must create a configuration file. When the configuration file   **\n");
  printf(" ** has been created, you can start the ert application with:       **\n");
  printf(" **                                                                 **\n");
  printf(" **   bash> ert config_file                                         **\n");
  printf(" **                                                                 **\n");
  printf(" ** Instructions on how to create the configuration file can be     **\n");
  printf(" ** found at: http://ert.nr.no                                      **\n");
  printf(" *********************************************************************\n");
}






int main (int argc , char ** argv) {
  devel_warning();
  text_splash();
  printf("\n");
  printf("Documentation : %s \n","http://ert.nr.no");
  printf("svn version   : %s \n",SVN_VERSION);
  printf("compile time  : %s \n",COMPILE_TIME_STAMP);
  printf("site config   : %s \n\n",SITE_CONFIG_FILE);
  enkf_main_install_SIGNALS();                     /* Signals common to both tui and gui. */
  signal(SIGINT , util_abort_signal);              /* Control C - tui only.               */
  enkf_main_init_debug( NULL );
  if (argc != 2) {
    enkf_usage();
    exit(1);
  } else {
    const char * site_config_file  = SITE_CONFIG_FILE;  /* The variable SITE_CONFIG_FILE should be defined on compilation ... */
    const char * model_config_file = argv[1]; 
    
    enkf_welcome( model_config_file );
    {
      enkf_main_type * enkf_main = enkf_main_bootstrap(site_config_file , model_config_file , true , true);
      enkf_tui_main_menu(enkf_main); 
      enkf_main_free(enkf_main);
    }
    
    util_abort_free_version_info(); /* No fucking leaks ... */
  }
}

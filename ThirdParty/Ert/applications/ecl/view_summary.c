/*
   Copyright (C) 2011  Statoil ASA, Norway.
   The file 'view_summary.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/ert_api_config.h>

#ifdef ERT_HAVE_GETOPT
#include <getopt.h>
#endif

#include <ert/util/util.h>
#include <ert/util/stringlist.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_sum.hpp>



void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the program with SIGTERM (the default kill signal) you will get a backtrace.
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
}


void print_help_and_exit()  {
  printf("\nThe summary.x program is used to quickly extract summary vectors\n");
  printf("from ECLIPSE summary files. The program is invoked as:\n");
  printf("\n");
  printf("computer> summary.x /Path/to/ECLIPSE key1 key2 key3 ....\n");
  printf("\n");
  printf("Here ECLIPSE is the name of an existing case, you can give it with\n");
  printf("extension, or without; the case need not be in the current directory. \n");
  printf("\n");
  printf("The keys are formed by combining ECLIPSE variable names and\n");
  printf("qualifiers from the WGNAMES and NUMS arrays. Examples of keys are:\n");
  printf("\n");
  printf("   WWCT:F-36          - The watercut in the well F-36.\n");
  printf("   FOPT               - The total field oil production.\n");
  printf("   RPR:3              - The region pressure in region 3.\n");
  printf("   GGIT:NORTH         - The total gas injection group NORTH.\n");
  printf("   SPR:F-12:18        - The segment pressure in well F-12, segment 18.\n");
  printf("   BPR:10,10,10       - The block pressure in cell 10,10,10.\n");
  printf("   LBPR:LGR3:10,10,10 - The block pressure in cell 10,10,10 - in LGR3\n");
  printf("\n");

#ifdef ERT_HAVE_GETOPT
  printf("The option --list can be used to list all available keys.\n");
  printf("\n");
  printf("Options:\n");
  printf("\n");
  printf("  --list : The program will list available keys.\n");
  printf("\n");
  printf("  --no-restart: If the simulation in question is a restart, i.e a prediction \n");
  printf("      which starts at the end of the historical period, the summary.x \n");
  printf("      program will by default also load historical data. If the --no-restart \n");
  printf("      option is used the program will not look for old results.\n");
  printf("\n");
  printf("  --no-header: By default summary.x will print a header line at the top, with the\n");
  printf("               option --no-header this will be suppressed.   \n");
  printf("\n");
  printf("  --report-only: Will only report results at report times (i.e. DATES).\n");
  printf("\n");
  printf("  --help: Print this message and exit.\n");
  printf("\n");
  printf("The options should come before the ECLIPSE basename.\n");
  printf("\n");
  printf("Example1:\n");
  printf("\n");
  printf("  computer> summary.x  CASE1_XXX WWCT:F-36   FOPT   FWPT\n");
  printf("\n");
  printf("  This example will load results from case 'CASE1_XXX' and print the\n");
  printf("  results for keys 'WWCT:F-36', 'FOPT' and 'FWPT' on standard out.\n");
  printf("\n");
  printf("\n");
  printf("Example2:\n");
  printf("\n");
  printf("  computer> summary.x  --list CASE2_XXX \"*:F-36\"  \"BPR:*\"\n");
  printf("\n");
  printf("  This example will list all the available keys which end with\n");
  printf("  ':F-36' and those which start with 'BPR:'. Observe the use of \n");
  printf("  quoting characters \"\" when using shell wildcards.\n");
#endif
  printf("\n");
  printf("The summary.x program will look for and load both unified and\n");
  printf("non-unified and formatted and non-formatted files. The default\n");
  printf("search order is: UNSMRY, Snnnn, FUNSMRY, Annnn, however you can\n");
  printf("manipulate this with the extension to the basename:\n");
  printf("\n");
  printf("* If the extension corresponds to an unformatted file, summary.x\n");
  printf("  will only look for unformatted files.\n");
  printf("\n");
  printf("* If the extension corresponds to a unified file, summary.x will\n");
  printf("  only look for unified files.\n");
  printf("\n");
  printf("\n");
  printf("Contact Joakim Hove / joaho@statoil.com / 92 68 57 04 for bugs\n");
  printf("and feature requests.\n");
  exit(1);
}


static void build_key_list( const ecl_sum_type * ecl_sum , stringlist_type * key_list , int argc , const char ** argv) {
  int iarg;
  for (iarg = 0; iarg < argc; iarg++) {
    /**
       If the string does not contain wildcards we add it
       unconditionally; and then subsequently let the
       ecl_sum_fprintf() function print a message on stderr about
       missing keys.
    */

    if (util_string_has_wildcard( argv[iarg] ))
      ecl_sum_select_matching_general_var_list( ecl_sum , argv[iarg] , key_list);
    else
      stringlist_append_copy( key_list , argv[iarg] );
  }
}


int main(int argc , char ** argv) {
  install_SIGNALS();
  {
    bool           report_only     = false;
    bool           list_mode       = false;
    bool           include_restart = true;
    bool           print_header    = true;
    int            arg_offset      = 1;

#ifdef ERT_HAVE_GETOPT
    if (argc == 1)
      print_help_and_exit();
    else {

      static struct option long_options[] = {
        {"no-restart"  , 0 , 0 , 'n'} ,
        {"list"        , 0 , 0 , 'l'} ,
        {"report-only" , 0 , 0 , 'r'} ,
        {"no-header"   , 0 , 0 , 'x'} ,
        {"help"        , 0 , 0 , 'h'} ,
        { 0            , 0 , 0 ,   0} };

      while (1) {
        int c;
        int option_index = 0;

        c = getopt_long (argc, argv, "nlRrx", long_options, &option_index);
        if (c == -1)
          break;

        switch (c) {
        case 'n':
          include_restart = false;
          break;
        case 'r':
          report_only = true;
          break;
        case 'l':
          list_mode = true;
          break;
        case 'x':
          print_header = false;
          break;
        case 'h':
          print_help_and_exit();
          break;
        case '?':
          printf("Hmmm - unrecognized option???");
          break;
        }
      }
      arg_offset = optind;  /* External static variable in the getopt scope*/
    }
#endif

    if (arg_offset >= argc)
      print_help_and_exit();

    {
      char         * data_file = argv[arg_offset];
      ecl_sum_type * ecl_sum;
      int            num_keys  = argc - arg_offset - 1;
      const char  ** arg_list  = (const char **) &argv[arg_offset + 1];


      ecl_sum = ecl_sum_fread_alloc_case2__( data_file , ":" , include_restart, true, 0);
      /** If no keys have been presented the function will list available keys. */
      if (num_keys == 0)
        list_mode = true;

      if (ecl_sum != NULL) {
        if (list_mode) {
          /*
             The program is called in list mode, we only print the
             (matching) keys in a table on stdout. If no arguments
             have been given on the commandline, all internalized keys
             will be printed.
          */

          stringlist_type * keys = stringlist_alloc_new();
          if (num_keys == 0) {
            ecl_sum_select_matching_general_var_list( ecl_sum , "*" , keys);
            stringlist_sort(keys , NULL );
          } else
            build_key_list( ecl_sum , keys , num_keys , arg_list);

          {
            int columns = 5;
            int i;
            for (i=0; i< stringlist_get_size( keys );  i++) {
              printf("%-24s ",stringlist_iget( keys , i ));
              if ((i % columns) == 4)
                printf("\n");
            }
            printf("\n");
          }
          stringlist_free( keys );
        } else {
          /* Normal operation print results for the various keys on stdout. */
          ecl_sum_fmt_type fmt;
          stringlist_type * key_list = stringlist_alloc_new( );
          build_key_list( ecl_sum , key_list , num_keys , arg_list);

          if (print_header)
            ecl_sum_fmt_init_summary_x(ecl_sum , &fmt );
          else
            fmt.print_header = false;

          ecl_sum_fprintf(ecl_sum , stdout , key_list , report_only , &fmt);

          stringlist_free( key_list );
        }
        ecl_sum_free(ecl_sum);
      } else
        fprintf(stderr,"summary.x: No summary data found for case:%s\n", data_file );
    }
  }
}

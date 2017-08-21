/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'vprofile.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <math.h>

#include <util.h>
#include <path_fmt.h>
#include <double_vector.h>

#include <ecl_grid.h>
#include <ecl_file.h>
#include <ecl_kw.h>
#include <ecl_util.h>





static void usage() {
  printf("-----------------------------------------------------------------\n");
  printf("This little program can be used to extract vertical properties of\n");
  printf("solution data from ECLIPSE restart files. The program needs the\n");
  printf("following input, from the commandline:\n");
  printf("\n");
  printf(" 1. The i,j coordinates you are interested in.\n");
  printf("\n");
  printf(" 2. The ECLIPSE basename - this can either be just the basename, or an\n");
  printf("    arbitrary ECLIPSE file with the correct basename. (Can contain a\n");
  printf("    leading path part).\n");
  printf("\n");
  printf(" 3. The keyword you want to extract, this will typically be SWAT, SGAS\n");
  printf("    or PRESSURE.\n");
  printf("\n");
  printf(" 4. A filename (can contain a path component) for the profile\n");
  printf("    files. This filename MUST contain a %%d format specifier which will\n");
  printf("    be replaced with the report step.\n");
  printf("\n");
  printf(" 5. The timesteps you want to consider.\n");
  printf("\n");
  printf("\n");
  printf("Example:\n");
  printf("--------\n");
  printf("\n");
  printf("  bash%% vprofile 10 5 MODEL1.DATA SWAT  profiles/SWAT_%%04d  0 10 20 30 40 50 60 \n");
  printf("\n");
  printf("This will load vertical profiles for i=10 and j=5. The results will be\n");
  printf("loaded from a simulation with basename 'MODEL1'. It will load the\n");
  printf("keyword 'SWAT', the profiles will be written to the the files:\n");
  printf("\n");
  printf("  profiles/swat_0000\n");
  printf("  profiles/swat_0010\n");
  printf("  profiles/swat_0020\n");
  printf("  ....\n");
  printf("  profiles/swat_0060\n");
  printf("\n");
  printf("The profile files will contain two columns, the keyword you have\n");
  printf("have asked for and the depth. The i,j must be from the global grid, but (if\n");
  printf("available) LGR information will be used for the profiles. The program\n");
  printf("works roughly as follows:\n");
  printf("\n");
  printf(" 1. Based on the ECLIPSE basename the program will look for an EGRID,\n");
  printf("    alternatively GRID file, and load this.\n");
  printf("\n");
  printf(" 2. Will load restart information based on the timesteps given by the\n");
  printf("    user. The program will first look for a unified restart file, and\n");
  printf("    then subsequently for non unified files.\n");
  printf("\n");
  printf(" 3. Scans thorough the timesteps/profiles and creates output files.   \n");
  printf("  \n");
  printf("Observe that the program ONLY looks for unformatted files.\n");
  printf("-----------------------------------------------------------------\n");
  exit(1);
}


static void vprofile__( int i , int j , 
                        const ecl_grid_type * ecl_grid , 
                        const ecl_file_type * ecl_file , 
                        const char * kw , 
                        double_vector_type * depth , 
                        double_vector_type * profile) {
  int k;
  int nz = ecl_grid_get_nz( ecl_grid );
  ecl_kw_type * ecl_kw = ecl_file_iget_named_kw( ecl_file , kw , ecl_grid_get_grid_nr( ecl_grid ));
  for (k=0; k < nz; k++) {
    int active_index = ecl_grid_get_active_index3( ecl_grid , i,j,k);
    if (active_index >= 0) {
      ecl_grid_type * lgr = ecl_grid_get_cell_lgr3( ecl_grid , i,j,k);
      if (lgr != NULL) {
        /* Recursive dive .. */
        double x,y,z;
        int lgr_i , lgr_j , lgr_k , lgr_global;
        
        ecl_grid_get_xyz3( ecl_grid, i ,j , k, &x , &y , &z);
        lgr_global = ecl_grid_get_global_index_from_xyz( lgr , x , y , z , 0);
        if (lgr_global < 0) 
          util_exit("Hmmmm - could not locate point: %g,%g,%g in lgr:%s \n",x,y,z,ecl_grid_get_name( lgr ));

        ecl_grid_get_ijk1( lgr , lgr_global , &lgr_i , &lgr_j , &lgr_k);
        vprofile__(lgr_i , lgr_j , lgr , ecl_file , kw , depth , profile);
      } 
      
      double_vector_append( depth   , ecl_grid_get_cdepth3( ecl_grid , i,j,k));
      double_vector_append( profile , ecl_kw_iget_as_double( ecl_kw , active_index ));
    } 
  }
}



static void vprofile( int i , int j , const ecl_grid_type * ecl_grid , const ecl_file_type * ecl_file , const char * kw , int tstep , path_fmt_type * output_fmt) {
  if (ecl_file_has_kw( ecl_file , kw )) {
    double_vector_type * depth   = double_vector_alloc(0 , 0);
    double_vector_type * profile = double_vector_alloc(0 , 0);
    vprofile__(i , j , ecl_grid , ecl_file , kw , depth , profile );
    
    {
      char * filename = path_fmt_alloc_file( output_fmt , true , tstep );
      int  * perm     = double_vector_alloc_sort_perm( depth );
      FILE * stream   = util_fopen( filename, "w");
      int l;
      double_vector_permute( depth , perm );
      double_vector_permute( profile , perm );
      for (l=0; l < double_vector_size( depth ); l++) 
        fprintf(stream , "%12.5f  %12.5f\n", double_vector_iget( profile , l ) , -double_vector_iget( depth , l));

      free( perm );
      fclose( stream );
      free( filename);
    }
  }
}



int main (int argc , char ** argv) {
  if (argc < 6) 
    usage();
  {
    const char * base_file  = argv[3];
    const char * kw         = argv[4];
    const char * fmt        = argv[5];
    const char ** tlist     = (const char **) &argv[6];
    const int    num_tstep  = argc - 6;
    int i,j;


    if (util_sscanf_int( argv[1] , &i) && util_sscanf_int( argv[2] , &j)) {
      bool   unified;
      path_fmt_type * output_fmt;
      char * restart_file;
      char * path;
      char * eclbase;
      ecl_grid_type * ecl_grid;
      
      util_alloc_file_components( base_file , &path , &eclbase , NULL );
      {
        char * grid_file = ecl_util_alloc_filename( path , eclbase , ECL_EGRID_FILE , false , -1);
        if (util_file_exists( grid_file )) {
          ecl_grid = ecl_grid_alloc( grid_file );
          printf("Loading grid file: %s \n", grid_file);
        } else {
          free( grid_file );
          grid_file = ecl_util_alloc_filename( path , eclbase , ECL_GRID_FILE , false , -1);
          if (util_file_exists( grid_file )) {
            ecl_grid = ecl_grid_alloc( grid_file );
            printf("Loading grid file: %s \n", grid_file);
          } else {
            fprintf(stderr,"Could not locate %s.EGRID or %s.GRID file in %s.\n",eclbase , eclbase , path);
            exit(1);
          }
        }
        free( grid_file );
      }
      
      restart_file = ecl_util_alloc_filename( path , eclbase , ECL_UNIFIED_RESTART_FILE , false , -1 );
      if (util_file_exists( restart_file ))
        unified = true;
      else {
        unified = false;
        free( restart_file );
      }

      output_fmt = path_fmt_alloc_path_fmt( fmt );
      
      {
        int it;
        for (it = 0; it < num_tstep; it++) {
          int tstep;
          if (util_sscanf_int( tlist[it] , &tstep)) {
            ecl_file_type     * ecl_file = NULL;
            
            if (unified) {
              ecl_file = ecl_file_open( restart_file );
              if (!ecl_file_select_rstblock_report_step( ecl_file , tstep )) {
                fprintf(stderr , "** Failed to load restart information for step:%d \n", tstep);
                exit(1);
              }
              else
                printf("Loading report step:%d from:%s \n",tstep , restart_file );
            } else {
              restart_file = ecl_util_alloc_exfilename( path , eclbase , ECL_RESTART_FILE , false , tstep );
              if (restart_file != NULL) {
                ecl_file = ecl_file_open( restart_file );
                printf("Loading report step:%d from:%s \n",tstep , restart_file );
              }
            }
            
            vprofile( i - 1, j - 1, ecl_grid , ecl_file , kw , tstep , output_fmt );
            
            if (!unified)
              util_safe_free( restart_file );
          } else
            fprintf(stderr,"** The string: \'%s\' was not interpreted as a time-step - ignored \n",tlist[it]);
        }
      }
    } else
      fprintf(stderr,"Failed to interpret \'%s\' and \'%s\' as integers.\n",argv[1] , argv[2]);
  }
}

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
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <util.h>
#include <hash.h>
#include <stringlist.h>
#include <block_fs.h>
#include <msg.h>

#include <ecl_sum.h>
#include <ecl_smspec.h>

#include <config.h>

#include <local_driver.h>
#include <lsf_driver.h>
#include <ext_joblist.h>

#include <enkf_fs.h>
#include <enkf_main.h>
#include <enkf_types.h>
#include <enkf_sched.h>
#include <config_keys.h>
#include <enkf_defaults.h>
#include <enkf_tui_main.h>
#include <ert_build_info.h>
#include <site_config_file.h>
#include <block_fs_driver.h>
#include <fs_driver.h>


#define BLOCK_FS_DRIVER_INDEX_ID 3002


config_type * create_config( ) {
  config_type * config = config_alloc( );
  config_schema_item_type * item;

  item = config_add_schema_item(config , ENSPATH_KEY , true , false);
  config_schema_item_set_argc_minmax(item , 1 , 1 , 0 , NULL);
  
  item = config_add_schema_item(config , NUM_REALIZATIONS_KEY , true , false);
  config_schema_item_set_argc_minmax(item , 1 , 1 , 1, (const config_item_types [1]) {CONFIG_INT});
  
  item = config_add_schema_item(config , REFCASE_KEY , true , false);
  config_schema_item_set_argc_minmax(item , 1 , 1 , 1 , NULL );

  return config;
}


void fskip_block_fs( FILE * stream ) {
  util_fread_int( stream );
  util_fread_int( stream );
}


void fskip_block_fs_index( FILE * stream ) {
  return;
}


char * check_enspath( const char * ens_path , stringlist_type * case_list ) {
  char * mount_file = util_alloc_filename( ens_path , "enkf_mount_info" , NULL);
  FILE * stream = util_fopen( mount_file , "r");
  int version;

  util_fread_long( stream );
  version = util_fread_int( stream );

  if (version != 104)
    util_exit("This application is only for upgrading fs from version 104\n");
  
  {
    for (int driver_nr = 0; driver_nr < 5; driver_nr++) {
      int driver_id;
      util_fread_int( stream );
      
      driver_id  = util_fread_int( stream );
      if (driver_id == BLOCK_FS_DRIVER_ID)
        fskip_block_fs(stream);
      else if (driver_id == BLOCK_FS_DRIVER_INDEX_ID)
        fskip_block_fs_index(stream);
      else
        util_abort("%s: sorry can only convert BLOCK_FS cases\n",__func__);
    }
  }
  
  {
    int num_case = util_fread_int( stream );
    for (int icase = 0; icase < num_case; icase++) {
      char * case_name = util_fread_alloc_string( stream );
      stringlist_append_owned_ref( case_list , case_name );
    }
  }


  fclose( stream );
  return mount_file;
}





void upgrade_case( int ens_size , const ecl_sum_type * refcase , const char * enspath , const char * case_path , const char * file) {
  int num_drivers = 32;
  int length = ecl_sum_get_last_report_step( refcase );
  block_fs_type ** fs_list = util_calloc( num_drivers , sizeof * fs_list );
  buffer_type * buffer = buffer_alloc(100);
  double_vector_type * vector = double_vector_alloc( 0,0 );
  int driver_nr;

  
  for (driver_nr = 0; driver_nr < num_drivers; driver_nr++) {
    char * mount_file = util_alloc_sprintf( "%s/%s/mod_%d/%s.mnt" , enspath , case_path , driver_nr , file);
    fs_list[driver_nr] = block_fs_mount( mount_file , 32 , 0 , 1 , 1 , true , false );
    free( mount_file );
  }
  
  {
    const ecl_smspec_type * smspec = ecl_sum_get_smspec( refcase );
    int num_params = ecl_smspec_get_params_size( smspec );
    msg_type * msg;
    {
      char * prefix = util_alloc_sprintf("Upgrading %s/mod_nnn/%s: " , case_path , file );
      msg = msg_alloc( prefix , false );
      free( prefix );
    }
    msg_show( msg );
    
    for (int i=0; i < num_params; i++) {
      const smspec_node_type * smspec_node = ecl_smspec_iget_node( smspec , i );
      const char * gen_key = smspec_node_get_gen_key1( smspec_node );
      {
        char * progress = util_alloc_sprintf("%4.1f %s" , i * 100.0 / num_params , "%");
        msg_update( msg , progress );
        free( progress );
      }
      if (gen_key != NULL) {
        for (int iens = 0; iens < ens_size; iens++) {
          block_fs_type * fs = fs_list[ iens % num_drivers ];
          double_vector_reset( vector );
          for (int tstep = 0; tstep <= length; tstep++) {
            char * node_key = util_alloc_sprintf("%s.%d.%d" , gen_key , tstep , iens );
            
            if (block_fs_has_file( fs , node_key )) {
              block_fs_fread_realloc_buffer( fs , node_key , buffer );
              buffer_fskip( buffer , 12 );
              double_vector_iset( vector , tstep , buffer_fread_double( buffer ));
              block_fs_unlink_file( fs , node_key );
            }
            free( node_key );
          }

          if (double_vector_size( vector ) > 0) {
            char * vector_key = util_alloc_sprintf("%s.%d" , gen_key , iens );
            buffer_rewind( buffer );
            buffer_fwrite_time_t( buffer , time(NULL));
            buffer_fwrite_int( buffer , SUMMARY );
            double_vector_buffer_fwrite( vector , buffer );
            
            block_fs_fwrite_buffer( fs , vector_key , buffer );
            free( vector_key );
          }
        }
      }
    }
    msg_free( msg , false);
  }
    
  double_vector_free( vector );
  buffer_free( buffer );
  for (driver_nr = 0; driver_nr < num_drivers; driver_nr++) {
    block_fs_close( fs_list[driver_nr] , true );
  }
  free( fs_list );
}


void update_index( int ens_size, int length, const char * ens_path , const char * case_path ) {
  char * index_path = util_alloc_sprintf("%s/%s/Index" , ens_path , case_path );
  char * old_path   = util_alloc_sprintf("%s/%s"       , ens_path , case_path );
  
  util_make_path( index_path );
  util_move_file4( "INDEX.data_0" , NULL , old_path , index_path );
  util_move_file4( "INDEX.mnt"    , NULL , old_path , index_path );
  
  free( old_path );

  {
    char * mount_file = util_alloc_sprintf("%s/INDEX.mnt" , index_path);
    block_fs_type * index = block_fs_mount( mount_file , 32 , 0 , 1 , 1 , true , false );
    buffer_type * buffer = buffer_alloc( 512 );
    for (int iens = 0; iens < ens_size; iens++)
      for (int tstep = 0; tstep < length; tstep++) {
        char * old_key = util_alloc_sprintf("kw_list_%d.%d" , tstep , iens);
        char * new_key = util_alloc_sprintf("kw_list.%d.%d" , tstep , iens);

        if (block_fs_has_file( index , old_key )) {
          block_fs_fread_realloc_buffer( index , old_key , buffer );
          block_fs_fwrite_buffer( index , new_key , buffer );
          block_fs_unlink_file( index , old_key );
        }
        
        free( old_key );
        free( new_key );
      }
    
    free( mount_file );
    block_fs_close( index , true );
  }
}


void create_fstab( const char * ens_path , const char * case_path ) {
  int num_drivers = 32;
  char * mount_point = util_alloc_sprintf("%s/%s" , ens_path , case_path );
  FILE * stream = fs_driver_open_fstab( mount_point , true );
  fs_driver_init_fstab( stream, BLOCK_FS_DRIVER_ID);
  
  block_fs_driver_create_fs( stream , mount_point , DRIVER_PARAMETER        , num_drivers , "mod_%d" , "PARAMETER");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_STATIC           , num_drivers , "mod_%d" , "STATIC");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_DYNAMIC_FORECAST , num_drivers , "mod_%d" , "FORECAST");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_DYNAMIC_ANALYZED , num_drivers , "mod_%d" , "ANALYZED");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_INDEX            , 1           , "Index"  , "INDEX");
  
  fclose( stream );
  free( mount_point );
}





int main (int argc , char ** argv) {
  enkf_main_install_SIGNALS();                     /* Signals common to both tui and gui. */
  signal(SIGINT , util_abort_signal);              /* Control C - tui only.               */
  enkf_main_init_debug( NULL );
  if (argc != 2) {
    printf("Usage: upgrade_fs104 config_file\n");
    exit(1);
  } else {
    const char * model_config_file = argv[1]; 
    char * enspath;
    int ens_size;
    ecl_sum_type * refcase;
    {
      config_type * config = create_config();
      if (!config_parse( config , model_config_file , "--" , "INCLUDE" , "DEFINE" , CONFIG_UNRECOGNIZED_IGNORE , true )) {
        config_fprintf_erors( config , stderr );
        exit(1);
      }
        
      {
        char * path;
        util_alloc_file_components(model_config_file , &path , NULL , NULL);
        if (path != NULL) {
          printf("Changing to directory:%s\n" , path);
          chdir(path);
        }
        util_safe_free( path );
      }

      ens_size = config_get_value_as_int( config , NUM_REALIZATIONS_KEY );
      enspath  = util_alloc_string_copy( config_get_value( config , ENSPATH_KEY ));
      refcase  = ecl_sum_fread_alloc_case( config_get_value( config , REFCASE_KEY ) , ":");
      config_free( config );
    }

    {
      stringlist_type * case_list = stringlist_alloc_new();
      int ic;
      char * mount_file = check_enspath( enspath , case_list );
      
      for (ic = 0; ic < stringlist_get_size( case_list ); ic++) {
        const char * case_path = stringlist_iget( case_list , ic);
        
        upgrade_case( ens_size, refcase , enspath , case_path , "FORECAST");
        upgrade_case( ens_size, refcase , enspath , case_path , "ANALYZED");
        
        update_index( ens_size, ecl_sum_get_last_report_step( refcase ), enspath , case_path );
        create_fstab( enspath , case_path );
      }
      {
        FILE * stream = util_fopen( mount_file , "w");
        util_fwrite_long(FS_MAGIC_ID , stream);
        util_fwrite_int( 105 , stream);
        fclose( stream );
      }
      stringlist_free( case_list );
      free( mount_file );
      
    }
    free( enspath );
    ecl_sum_free( refcase );
    util_abort_free_version_info(); /* No fucking leaks ... */
  }
}

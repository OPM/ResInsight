void ecl_file_push_block( ecl_file_type * ecl_file ) {
  vector_append_ref( ecl_file->map_stack , ecl_file->active_view );
}

void ecl_file_pop_block( ecl_file_type * ecl_file ) {
  ecl_file->active_view = vector_pop_back( ecl_file->map_stack );
}


static ecl_file_view_type * ecl_file_get_relative_blockview( ecl_file_type * ecl_file , const char * kw , int occurence) {
  ecl_file_view_type * view = ecl_file_view_add_blockview( ecl_file->active_view , kw , occurence );
  return view;
}



bool ecl_file_subselect_block( ecl_file_type * ecl_file , const char * kw , int occurence) {
  ecl_file_view_type * blockmap = ecl_file_get_relative_blockview( ecl_file , kw , occurence);
  if (blockmap != NULL) {
    ecl_file->active_view = blockmap;
    return true;
  } else
    return false;
}


bool ecl_file_select_block( ecl_file_type * ecl_file , const char * kw , int occurence ) {
  ecl_file_view_type * blockmap = ecl_file_get_global_blockview( ecl_file , kw , occurence);
  if (blockmap != NULL) {
    ecl_file->active_view = blockmap;
    return true;
  } else
    return false;
}


/*
  Will select restart block nr @seqnum_index - without considering
  report_steps or simulation time.
*/
bool ecl_file_iselect_rstblock( ecl_file_type * ecl_file , int seqnum_index ) {
  return ecl_file_select_block( ecl_file , SEQNUM_KW , seqnum_index );
}


bool ecl_file_select_rstblock_sim_time( ecl_file_type * ecl_file , time_t sim_time) {
  int seqnum_index = ecl_file_view_seqnum_index_from_sim_time( ecl_file->global_view , sim_time );

  if (seqnum_index >= 0)
    return ecl_file_iselect_rstblock( ecl_file , seqnum_index);
  else
    return false;
}


bool ecl_file_select_rstblock_report_step( ecl_file_type * ecl_file , int report_step) {
  int global_index = ecl_file_view_find_kw_value( ecl_file->global_view , SEQNUM_KW , &report_step);
  if ( global_index >= 0) {
    int seqnum_index = ecl_file_view_iget_occurence( ecl_file->global_view , global_index );
    return ecl_file_iselect_rstblock( ecl_file ,  seqnum_index);
  } else
    return false;
}


/******************************************************************/

static ecl_file_type * ecl_file_open_rstblock_report_step__( const char * filename , int report_step , int flags) {
  ecl_file_type * ecl_file = ecl_file_open( filename , flags );
  if (ecl_file) {
    if (!ecl_file_select_rstblock_report_step( ecl_file , report_step )) {
      ecl_file_close( ecl_file );
      ecl_file = NULL;
    }
  }
  return ecl_file;
}

ecl_file_type * ecl_file_open_rstblock_report_step( const char * filename , int report_step , int flags) {
  return ecl_file_open_rstblock_report_step__(filename , report_step , flags );
}


/******************************************************************/

static ecl_file_type * ecl_file_open_rstblock_sim_time__( const char * filename , time_t sim_time, int flags ) {
  ecl_file_type * ecl_file = ecl_file_open( filename , flags );
  if (ecl_file) {
    if (!ecl_file_select_rstblock_sim_time( ecl_file , sim_time)) {
      ecl_file_close( ecl_file );
      ecl_file = NULL;
    }
  }
  return ecl_file;
}

ecl_file_type * ecl_file_open_rstblock_sim_time( const char * filename , time_t sim_time, int flags) {
  return ecl_file_open_rstblock_sim_time__( filename , sim_time , flags );
}

/******************************************************************/

static ecl_file_type * ecl_file_iopen_rstblock__( const char * filename , int seqnum_index, int flags ) {
  ecl_file_type * ecl_file = ecl_file_open( filename , flags );
  if (ecl_file) {
    if (!ecl_file_iselect_rstblock( ecl_file , seqnum_index )) {
      ecl_file_close( ecl_file );
      ecl_file = NULL;
    }
  }
  return ecl_file;
}


ecl_file_type * ecl_file_iopen_rstblock( const char * filename , int seqnum_index , int flags) {
  return ecl_file_iopen_rstblock__(filename , seqnum_index , flags );
}

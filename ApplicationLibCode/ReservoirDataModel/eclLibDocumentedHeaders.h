
#if 0 // Simplified ecl library access methods
typedef double (block_function_ftype) ( const double_vector_type *); 
typedef struct ecl_grid_struct ecl_grid_type;

/////////////////////////////
// Open file commands

// Read file and allocate internal grid data structures
ecl_grid_type * ecl_grid_alloc(const char * );
ecl_grid_type * ecl_grid_load_case( const char * case_input );

// Check whether there exist a eclipse grid file corresponding to the supplied name or base name on disk
bool            ecl_grid_exists( const char * case_input );
// Find such grid file name
char          * ecl_grid_alloc_case_filename( const char * case_input );

// Create a new grid based on previously read keyword read from file (or manually created )
ecl_grid_type * ecl_grid_alloc_GRDECL_kw( const ecl_kw_type * gridhead_kw , const ecl_kw_type * zcorn_kw , const ecl_kw_type * coord_kw , const ecl_kw_type * actnum_kw , const ecl_kw_type * mapaxes_kw );
ecl_grid_type * ecl_grid_alloc_GRDECL_data(int , int , int , const float *  , const float *  , const int * , const float * mapaxes);

// Free memory allocated 
void            ecl_grid_free(ecl_grid_type * );

/////////////////////////////
// Query dimensions 

// Get dimensions 
void            ecl_grid_get_dims(const ecl_grid_type *grid , int *nx , int *ny , int *nz , int *active_size);
int             ecl_grid_get_nz( const ecl_grid_type * grid );
int             ecl_grid_get_nx( const ecl_grid_type * grid );
int             ecl_grid_get_ny( const ecl_grid_type * grid );

// Get size of global indexing array for the cells in this grid
int             ecl_grid_get_global_size( const ecl_grid_type * ecl_grid );
// Get the number of active cells in grid
int             ecl_grid_get_active_size( const ecl_grid_type * ecl_grid );


// Get position of the cell corners
void            ecl_grid_get_corner_xyz3(const ecl_grid_type * grid , int i , int j , int k, int corner_nr , double * xpos , double * ypos , double * zpos );
void            ecl_grid_get_corner_xyz1(const ecl_grid_type * grid , int global_index , int corner_nr , double * xpos , double * ypos , double * zpos );

////////////////////////////////////
// Cell Indexing lookup and conversion
//

// Get global Cell Index 
int             ecl_grid_get_global_index3(const ecl_grid_type * , int  , int , int );
int             ecl_grid_get_global_index1A(const ecl_grid_type * ecl_grid , int active_index);

// Get global cell index from geometric point
int             ecl_grid_get_global_index_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index);
// Get global cell index in layer k from geometric point
int             ecl_grid_get_global_index_from_xy( const ecl_grid_type * ecl_grid , int k , bool lower_layer , double x , double y);
// Get global cell index in top/bottom layer from geometric point
int             ecl_grid_get_global_index_from_xy_top( const ecl_grid_type * ecl_grid , double x , double y);
int             ecl_grid_get_global_index_from_xy_bottom( const ecl_grid_type * ecl_grid , double x , double y);

// Get index to active cell. Running index to all the active cells
int             ecl_grid_get_active_index(const ecl_grid_type *  , int  , int  , int );
int             ecl_grid_get_active_index3(const ecl_grid_type * ecl_grid , int i , int j , int k);
int             ecl_grid_get_active_index1(const ecl_grid_type * ecl_grid , int global_index);

// Check if cell is active
bool            ecl_grid_cell_active3(const ecl_grid_type * ecl_grid, int i , int j , int k);
bool            ecl_grid_cell_active1(const ecl_grid_type * ecl_grid, int global_index);

// Check that i,j,k has correct bounds
bool            ecl_grid_ijk_valid(const ecl_grid_type * , int  , int , int ); 

// Get i,j,k from other indexes
void            ecl_grid_get_ijk1( const ecl_grid_type *grid , int global_index , int *i , int *j , int *k);
void            ecl_grid_get_ijk1A(const ecl_grid_type *grid , int active_index , int *i , int *j , int *k);
void            ecl_grid_get_ijk_from_active_index(const ecl_grid_type *grid , int active_index, int *i , int *j , int *k );

// Get "true" (UTM based?) position of center? of cell
void            ecl_grid_get_xyz3( const ecl_grid_type * grid , int i, int j, int x , double *xpos , double *ypos , double *zpos);
void            ecl_grid_get_xyz1( const ecl_grid_type * grid , int global_index    , double *xpos , double *ypos , double *zpos);
void            ecl_grid_get_xyz1A(const ecl_grid_type * grid , int active_index    , double *xpos , double *ypos , double *zpos);

// Check if cell contains the point
bool            ecl_grid_cell_contains_xyz1( const ecl_grid_type * ecl_grid , int global_index , double x , double y , double z);
bool            ecl_grid_cell_contains_xyz3( const ecl_grid_type * ecl_grid , int i , int j , int k, double x , double y , double z );

// Cell volume calculations
double          ecl_grid_get_cell_thickness3( const ecl_grid_type * grid , int i , int j , int k);
double          ecl_grid_get_cell_thickness1( const ecl_grid_type * grid , int global_index );
double          ecl_grid_get_cell_volume1( const ecl_grid_type * ecl_grid, int global_index );
double          ecl_grid_get_cell_volume3( const ecl_grid_type * ecl_grid, int i , int j , int k);

// Get z position of cell center
double          ecl_grid_get_cdepth1(const ecl_grid_type * grid , int global_index);
double          ecl_grid_get_cdepth3(const ecl_grid_type * grid , int i, int j , int k);

// ????
double          ecl_grid_get_depth3(const ecl_grid_type * grid , int i, int j , int k);

// Get Cell mean top value
double          ecl_grid_get_top1(const ecl_grid_type * grid , int global_index);
double          ecl_grid_get_top3(const ecl_grid_type * grid , int i, int j , int k);
double          ecl_grid_get_top1A(const ecl_grid_type * grid , int active_index);
double          ecl_grid_get_top2(const ecl_grid_type * grid , int i, int j);

// Get Cell mean bottom
double          ecl_grid_get_bottom1(const ecl_grid_type * grid , int global_index);
double          ecl_grid_get_bottom3(const ecl_grid_type * grid , int i, int j , int k);
double          ecl_grid_get_bottom1A(const ecl_grid_type * grid , int active_index);
double          ecl_grid_get_bottom2(const ecl_grid_type * grid , int i, int j);

int             ecl_grid_locate_depth( const ecl_grid_type * grid , double depth , int i , int j );

// Compare two grids
bool            ecl_grid_compare(const ecl_grid_type * g1 , const ecl_grid_type * g2);
// Output metainfo
void            ecl_grid_summarize(const ecl_grid_type * );

/////////////////////////
// Blocking, not actually documented
void            ecl_grid_alloc_blocking_variables(ecl_grid_type * , int );
void            ecl_grid_init_blocking(ecl_grid_type * );
double          ecl_grid_block_eval2d(ecl_grid_type * grid , int i, int j , block_function_ftype * blockf );
double          ecl_grid_block_eval3d(ecl_grid_type * grid , int i, int j , int k ,block_function_ftype * blockf );
int             ecl_grid_get_block_count3d(const ecl_grid_type * ecl_grid , int i , int j, int k);
int             ecl_grid_get_block_count2d(const ecl_grid_type * ecl_grid , int i , int j);
bool            ecl_grid_block_value_2d(ecl_grid_type * , double  , double  ,double );
bool            ecl_grid_block_value_3d(ecl_grid_type * , double  , double  ,double , double);


//////////////////////////////////////////
// Local Grid Refinement (lgr) related functions 
//

// Get total number of Lgr's in this main grid (includes sub subs etc)
int                     ecl_grid_get_num_lgr(const ecl_grid_type * main_grid );
// Get lgr with index in this main grid
ecl_grid_type         * ecl_grid_iget_lgr(const ecl_grid_type * main_grid , int lgr_nr);

// Get lgr's names, and access them by names
stringlist_type       * ecl_grid_alloc_lgr_name_list(const ecl_grid_type * main_grid);
bool                    ecl_grid_has_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
ecl_grid_type         * ecl_grid_get_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);

// Get name of grid
const  char           * ecl_grid_get_name( const ecl_grid_type * ecl_grid );
// Get grid number in Grid file. Main grid will be 0
int                     ecl_grid_get_grid_nr( const ecl_grid_type * ecl_grid );

// Get lgr from cell in grid
const ecl_grid_type   * ecl_grid_get_cell_lgr3(const ecl_grid_type * grid , int i, int j , int k);
const ecl_grid_type   * ecl_grid_get_cell_lgr1A(const ecl_grid_type * grid , int active_index);
const ecl_grid_type   * ecl_grid_get_cell_lgr1(const ecl_grid_type * grid , int global_index );

// Get the running index of the cells position in the parent grid
int                     ecl_grid_get_parent_cell1( const ecl_grid_type * grid , int global_index);
int                     ecl_grid_get_parent_cell3( const ecl_grid_type * grid , int i , int j , int k);

const ecl_grid_type   * ecl_grid_get_global_grid( const ecl_grid_type * grid );
bool                    ecl_grid_is_lgr( const ecl_grid_type * ecl_grid );

/////////////////////////////////////////////////////////////////
// Property values foe the cells. Typically static properties or analysis results data

double                  ecl_grid_get_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k);
void                    ecl_grid_get_column_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j, double_vector_type * column);

///////////////////////////////////////////////
// Some debug print function ?
void                    ecl_grid_grdecl_fprintf_kw( const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , FILE * stream , double double_default);



//////////////////////////////////////////////
// ecl_file header
//

typedef struct ecl_file_struct ecl_file_type;

// General load file function. ECLIPSE files with ecl_kw format
ecl_file_type * ecl_file_fread_alloc( const char * filename );
// Allocate an empty ecl_file
ecl_file_type * ecl_file_alloc_empty( );

// Free a ecl_file structure
void            ecl_file_free( ecl_file_type * ecl_file );
void            ecl_file_free__(void * arg);

// Get the i'th keyword with name "kw" in the ecl_file structure
ecl_kw_type   * ecl_file_iget_named_kw(     const ecl_file_type * ecl_file , const char * kw , int ith);
ecl_kw_type   * ecl_file_icopy_named_kw(    const ecl_file_type * ecl_file , const char * kw , int ith);
int             ecl_file_get_num_named_kw(  const ecl_file_type * ecl_file , const char * kw);

// Get keyword number "index" in the ecl file structure
ecl_kw_type   * ecl_file_iget_kw(           const ecl_file_type * ecl_file , int index);
ecl_kw_type   * ecl_file_icopy_kw(          const ecl_file_type * ecl_file , int index);
int             ecl_file_get_num_kw(        const ecl_file_type * ecl_fil );

// Check if the keyword exists in the ecl_file structure
bool            ecl_file_has_kw(            const ecl_file_type * ecl_file , const char * kw);

// Access the list of keyword names occurring in the ecl_file structure
int             ecl_file_get_num_distinct_kw( const ecl_file_type * ecl_file);
const char    * ecl_file_iget_distinct_kw   ( const ecl_file_type * ecl_file , int index);

// Get the filename of the origin file
const char    * ecl_file_get_src_file(      const ecl_file_type * ecl_file );

// Get the keyword at "globKWIndex" and return the position it has among the other keywords with same name
int             ecl_file_iget_occurrence(    const ecl_file_type * ecl_file , int globalKWindex);


time_t           ecl_file_iget_restart_sim_date( const ecl_file_type * restart_file , int occurrence );
ecl_version_enum ecl_file_get_ecl_version(  const ecl_file_type * file );

int              ecl_file_get_restart_index(const ecl_file_type * restart_file , time_t sim_time);

// Get the reservoir phase enum (Oil, Water, Gas ...)
int             ecl_file_get_phases( const ecl_file_type * init_file );


ecl_file_type * ecl_file_copy_restart_section( const ecl_file_type * src_file , int report_step );
ecl_file_type * ecl_file_copy_summary_section( const ecl_file_type * src_file , int report_step );

ecl_file_type * ecl_file_fread_alloc_unsmry_section     (const char * filename , int index );
ecl_file_type * ecl_file_fread_alloc_unrst_section      (const char * filename , int report_step);
ecl_file_type * ecl_file_fread_alloc_unrst_section_time (const char * filename , time_t sim_time);

ecl_file_type * ecl_file_fread_alloc_restart_section(fortio_type * fortio);
ecl_file_type * ecl_file_fread_alloc_summary_section(fortio_type * fortio);
ecl_file_type * ecl_file_fread_alloc_RFT_section(fortio_type *     fortio);

// Manage ecl_kw instances in the ecl_file structure "manually"
void            ecl_file_delete_kw( ecl_file_type * ecl_file , const char * name , int occurrence );
void            ecl_file_insert_kw( ecl_file_type * ecl_file , ecl_kw_type * ecl_kw , bool after , const char * neighbour_name , int neighbour_occurrence );
void            ecl_file_replace_kw( ecl_file_type * ecl_file , ecl_kw_type * old_kw , const ecl_kw_type * new_kw , bool insert_copy);

bool            ecl_file_has_kw_ptr(const ecl_file_type * ecl_file , const ecl_kw_type * ecl_kw);

// Write to file
void            ecl_file_fwrite_fortio(const ecl_file_type * ec_file  , fortio_type * fortio , int offset);
void            ecl_file_fwrite(const ecl_file_type * ecl_file , const char * , bool fmt_file );

// Report/Debug
void            ecl_file_fprintf_kw_list( const ecl_file_type * ecl_file , FILE * stream );

////////////////////////////////////////////////////////////////////////////////
// Utils
//

// Find all files in a directory of specified ECLIPSE type and return as a list of strings
int ecl_util_select_filelist( const char * path , const char * base , ecl_file_enum file_type , bool fmt_file , stringlist_type * filelist) {

#endif

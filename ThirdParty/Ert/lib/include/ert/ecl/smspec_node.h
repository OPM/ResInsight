/*
  Warning: The libecl code has changed to be compiled as a C++ project. This
  header file is retained for a period for compatibility, but you are encouraged
  to switch to include the new hpp header directly in your code.
*/

#include <stdbool.h>
#include <stdio.h>

#ifndef ERT_SMSPEC_NODE_H
#define ERT_SMSPEC_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DUMMY_WELL ":+:+:+:+"
#define IS_DUMMY_WELL(well) (strcmp((well) , DUMMY_WELL) == 0)
#define SMSPEC_PARAMS_INDEX_INVALID -77


#define SMSPEC_TIME_KEYWORD "TIME"
#define SMSPEC_TIME_NUMS_VALUE     -32676

#define SMSPEC_YEARS_KEYWORD "YEARS"
#define SMSPEC_YEARS_NUMS_VALUE     -32676

typedef enum {ECL_SMSPEC_INVALID_VAR            =  0 ,
              ECL_SMSPEC_FIELD_VAR              =  1 ,   /* X */
              ECL_SMSPEC_REGION_VAR             =  2 ,   /* X */
              ECL_SMSPEC_GROUP_VAR              =  3 ,   /* X */
              ECL_SMSPEC_WELL_VAR               =  4 ,   /* X */
              ECL_SMSPEC_SEGMENT_VAR            =  5 ,   /* X */
              ECL_SMSPEC_BLOCK_VAR              =  6 ,   /* X */
              ECL_SMSPEC_AQUIFER_VAR            =  7 ,
              ECL_SMSPEC_COMPLETION_VAR         =  8 ,   /* X */
              ECL_SMSPEC_NETWORK_VAR            =  9 ,
              ECL_SMSPEC_REGION_2_REGION_VAR    = 10 ,
              ECL_SMSPEC_LOCAL_BLOCK_VAR        = 11 ,   /* X */
              ECL_SMSPEC_LOCAL_COMPLETION_VAR   = 12 ,   /* X */
              ECL_SMSPEC_LOCAL_WELL_VAR         = 13 ,   /* X */
              ECL_SMSPEC_MISC_VAR               = 14     /* X */}  ecl_smspec_var_type;

#define SMSPEC_NUMS_INVALID   -991199
#define SMSPEC_NUMS_WELL       1
#define SMSPEC_NUMS_GROUP      2
#define SMSPEC_NUMS_FIELD      0

#define SMSPEC_TYPE_ID 61550451

  char * smspec_alloc_block_ijk_key( const char * join_string , const char * keyword , int i , int j , int k);
  char * smspec_alloc_completion_ijk_key( const char * join_string , const char * keyword, const char * wgname , int i , int j , int k);
  char * smspec_alloc_completion_num_key( const char * join_string , const char * keyword, const char * wgname , int num);
  char * smspec_alloc_group_key( const char * join_string , const char * keyword , const char * wgname);
  char * smspec_alloc_well_key( const char * join_string , const char * keyword , const char * wgname);
  char * smspec_alloc_region_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_region_2_region_r1r2_key( const char * join_string , const char * keyword , int r1, int r2);
  char * smspec_alloc_region_2_region_num_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_segment_key( const char * join_string , const char * keyword , const char * wgname , int num);
  char * smspec_alloc_block_num_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_local_well_key( const char * join_string , const char * keyword , const char * lgr_name , const char * wgname);
  char * smspec_alloc_local_block_key( const char * join_string , const char * keyword , const char * lgr_name , int i , int j , int k);
  char * smspec_alloc_local_completion_key( const char * join_string, const char * keyword , const char * lgr_name , const char * wgname , int i , int j , int k);

  bool smspec_node_identify_total(const char * keyword, ecl_smspec_var_type var_type);
  bool smspec_node_identify_rate(const char * keyword);

  bool smspec_node_equal( const void * node1,  const void * node2);

  void smspec_node_init( void * smspec_node,
                         ecl_smspec_var_type var_type ,
                         const char * wgname  ,
                         const char * keyword ,
                         const char * unit    ,
                         const char * key_join_string ,
                         const int grid_dims[3] ,
                         int num);

  void * smspec_node_alloc( int param_index,
                            const char * keyword ,
                            const char * wgname,
                            int num,
                            const char * unit    ,
                            const int grid_dims[3] ,
                            float default_value,
                            const char * key_join_string);

  void * smspec_node_alloc_lgr( ecl_smspec_var_type var_type ,
                                            const char * wgname  ,
                                            const char * keyword ,
                                            const char * unit    ,
                                            const char * lgr ,
                                            const char * key_join_string ,
                                            int   lgr_i, int lgr_j , int lgr_k,
                                            int param_index,
                                            float default_value);

  void *  smspec_node_alloc_copy( const void* );

  void                smspec_node_free( void * index );
  void                smspec_node_free__(void * arg);
  void                smspec_node_set_params_index( void * smspec_node , int params_index);
  int                 smspec_node_get_params_index( const void * smspec_node );
  const char        * smspec_node_get_gen_key1( const void * smspec_node);
  const char        * smspec_node_get_gen_key2( const void * smspec_node);
  ecl_smspec_var_type smspec_node_get_var_type( const void * smspec_node);
  int                 smspec_node_get_num( const void * smspec_node);
  const char        * smspec_node_get_wgname( const void * smspec_node);
  const char        * smspec_node_get_keyword( const void * smspec_node);
  const char        * smspec_node_get_unit( const void * smspec_node);
  bool                smspec_node_is_rate( const void * smspec_node );
  bool                smspec_node_is_total( const void * smspec_node );
  bool                smspec_node_is_historical( const void * smspec_node );
  bool                smspec_node_need_nums( const void * smspec_node );
  void                smspec_node_fprintf( const void * smspec_node , FILE * stream);

  float               smspec_node_get_default( const void * smspec_node);

  const int*  smspec_node_get_ijk( const void * smpsec_node );
  const char* smspec_node_get_lgr_name( const void * smpsec_node );
  const int*  smspec_node_get_lgr_ijk( const void * smpsec_node );

  int smspec_node_get_R1( const void * smpsec_node );
  int smspec_node_get_R2( const void * smpsec_node );

  bool smspec_node_lt( const void * node1,  const void * node2);
  bool smspec_node_gt( const void * node1,  const void * node2);
  int smspec_node_cmp( const void * node1, const void * node2);
  int smspec_node_cmp__( const void * node1, const void * node2);

#ifdef __cplusplus
}
#endif
#endif



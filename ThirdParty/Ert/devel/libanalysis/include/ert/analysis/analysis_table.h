#ifndef __ANALYSIS_TABLE_H__
#define __ANALYSIS_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <ert/util/matrix.h>
#include <ert/util/rng.h>


  typedef void (analysis_updateA_ftype) (void * module_data , 
                                         matrix_type * A , 
                                         matrix_type * S , 
                                         matrix_type * R , 
                                         matrix_type * dObs , 
                                         matrix_type * E ,
                                         matrix_type * D );
  
  
  typedef void (analysis_initX_ftype)       (void * module_data , 
                                             matrix_type * X , 
                                             matrix_type * A , 
                                             matrix_type * S , 
                                             matrix_type * R , 
                                             matrix_type * dObs , 
                                             matrix_type * E , 
                                             matrix_type * D );
  
  
  typedef bool (analysis_set_int_ftype)       (void * module_data , const char * flag , int value);
  typedef bool (analysis_set_bool_ftype)      (void * module_data , const char * flag , bool value);
  typedef bool (analysis_set_double_ftype)    (void * module_data , const char * var , double value);
  typedef bool (analysis_set_string_ftype)    (void * module_data , const char * var , const char * value);
  typedef void   (analysis_free_ftype) (void * );
  typedef void * (analysis_alloc_ftype) ( rng_type * rng );


  typedef void (analysis_init_update_ftype) (void * module_data, 
                                             const matrix_type * S , 
                                             const matrix_type * R , 
                                             const matrix_type * dObs , 
                                             const matrix_type * E , 
                                             const matrix_type * D);
  
  typedef void (analysis_complete_update_ftype) (void * module_data );
  
  typedef long (analysis_get_options_ftype) (void * module_data , long option);

  typedef bool   (analysis_has_var_ftype)    (const void * module_data , const char * var_name);
  typedef int    (analysis_get_int_ftype)    (const void * module_data , const char * var_name );
  typedef double (analysis_get_double_ftype) (const void * module_data , const char * var_name );
  typedef void * (analysis_get_ptr_ftype)    (const void * module_data , const char * var_name );

/*****************************************************************/




typedef struct {
  const char                     * name;
  analysis_updateA_ftype         * updateA;
  analysis_initX_ftype           * initX;
  analysis_init_update_ftype     * init_update;
  analysis_complete_update_ftype * complete_update;

  analysis_free_ftype            * freef;
  analysis_alloc_ftype           * alloc;

  analysis_set_int_ftype         * set_int;
  analysis_set_double_ftype      * set_double;
  analysis_set_bool_ftype        * set_bool; 
  analysis_set_string_ftype      * set_string;
  analysis_get_options_ftype     * get_options;

  analysis_has_var_ftype         * has_var;
  analysis_get_int_ftype         * get_int;
  analysis_get_double_ftype      * get_double;
  analysis_get_ptr_ftype         * get_ptr;
} analysis_table_type;


#ifdef __cplusplus
}
#endif
#endif

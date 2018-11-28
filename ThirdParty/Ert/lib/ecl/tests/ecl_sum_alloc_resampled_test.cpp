
#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_sum.hpp>
#include <ert/ecl/smspec_node.hpp>

ecl_sum_type * test_alloc_ecl_sum() {
  time_t start_time = util_make_date_utc( 1,1,2010 );
  ecl_sum_type * ecl_sum = ecl_sum_alloc_writer( "/tmp/CASE" , false , true , ":" , start_time , true , 10 , 10 , 10 );
  double sim_seconds = 0;

  int num_dates = 4;
  double ministep_length = 86400; // seconds in a day

  smspec_node_type * node1 = ecl_sum_add_var( ecl_sum , "FOPT" , NULL   , 0   , "Barrels" , 99.0 );
  smspec_node_type * node2 = ecl_sum_add_var( ecl_sum , "BPR"  , NULL   , 567 , "BARS"    , 0.0  );
  smspec_node_type * node3 = ecl_sum_add_var( ecl_sum , "WWCT" , "OP-1" , 0   , "(1)"     , 0.0  );

  for (int report_step = 0; report_step < num_dates; report_step++) {
      {
        ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 1 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , node1 , report_step*2.0 );
        ecl_sum_tstep_set_from_node( tstep , node2 , report_step*4.0 + 2.0 );
        ecl_sum_tstep_set_from_node( tstep , node3 , report_step*6.0 + 4.0 );
      }
      sim_seconds += ministep_length * 3;

  }
  return ecl_sum;
}

void test_correct_time_vector() {

  ecl_sum_type * ecl_sum = test_alloc_ecl_sum();
  time_t_vector_type * t = time_t_vector_alloc( 0 , 0 );
  time_t_vector_append(t, util_make_date_utc( 2,1,2010 ));
  time_t_vector_append(t, util_make_date_utc( 4,1,2010 ));
  time_t_vector_append(t, util_make_date_utc( 6,1,2010 ));
  time_t_vector_append(t, util_make_date_utc( 8,1,2010 ));
  ecl_sum_type * ecl_sum_resampled = ecl_sum_alloc_resample(ecl_sum, "kk", t);
  test_assert_int_equal(  ecl_sum_get_report_time(ecl_sum_resampled, 2)  , util_make_date_utc( 6,1,2010 ));

  const ecl_smspec_type * smspec_resampled = ecl_sum_get_smspec(ecl_sum_resampled);
  const smspec_node_type * node1 = ecl_smspec_iget_node(smspec_resampled, 1);
  const smspec_node_type * node2 = ecl_smspec_iget_node(smspec_resampled, 2);
  const smspec_node_type * node3 = ecl_smspec_iget_node(smspec_resampled, 3);
  test_assert_string_equal( "BPR" , smspec_node_get_keyword(node2) );
  test_assert_string_equal( "BARS" , smspec_node_get_unit(node2) );

  test_assert_double_equal(3.33333, ecl_sum_get_from_sim_time( ecl_sum_resampled, util_make_date_utc( 6,1,2010 ), node1) );
  test_assert_double_equal(3.33333, ecl_sum_get_from_sim_time( ecl_sum_resampled, util_make_date_utc( 2,1,2010 ), node2) );
  test_assert_double_equal(10.0000, ecl_sum_get_from_sim_time( ecl_sum_resampled, util_make_date_utc( 4,1,2010 ), node3) );


  ecl_sum_free(ecl_sum_resampled);
  time_t_vector_free(t);
  ecl_sum_free(ecl_sum);
}

void test_time_before() {
  ecl_sum_type * ecl_sum = test_alloc_ecl_sum();
  time_t_vector_type * t = time_t_vector_alloc( 0 , 0 );
  time_t_vector_append(t, util_make_date_utc( 1,1,2009 ));
  test_assert_NULL( ecl_sum_alloc_resample(ecl_sum, "kk", t) );
  time_t_vector_free(t);
  ecl_sum_free(ecl_sum);
}

void test_time_after() {
  ecl_sum_type * ecl_sum = test_alloc_ecl_sum();
  time_t_vector_type * t = time_t_vector_alloc( 0 , 0 );
  time_t_vector_append(t, util_make_date_utc( 1,1,2010 ));
  time_t_vector_append(t, util_make_date_utc( 11,1,2010 ));
  test_assert_NULL( ecl_sum_alloc_resample(ecl_sum, "kk", t) );
  time_t_vector_free(t);
  ecl_sum_free(ecl_sum);
}

void test_not_sorted() {
  ecl_sum_type * ecl_sum = test_alloc_ecl_sum();
  time_t_vector_type * t = time_t_vector_alloc( 0 , 0 );
  time_t_vector_append(t, util_make_date_utc( 1,1,2010 ));
  time_t_vector_append(t, util_make_date_utc( 3,1,2010 ));
  time_t_vector_append(t, util_make_date_utc( 2,1,2010 ));
  test_assert_NULL( ecl_sum_alloc_resample( ecl_sum, "kk", t) );
  time_t_vector_free(t);
  ecl_sum_free(ecl_sum);
}


int main() {
  test_correct_time_vector();
  test_time_before();
  test_time_after();
  test_not_sorted();
  return 0;
}


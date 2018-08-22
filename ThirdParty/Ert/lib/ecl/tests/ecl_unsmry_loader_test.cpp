#include <vector>

#include <ert/util/TestArea.hpp>
#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_sum.hpp>

#include "detail/ecl/ecl_unsmry_loader.hpp"



ecl_sum_type * write_ecl_sum() {
  time_t start_time = util_make_date_utc( 1,1,2010 );
  ecl_sum_type * ecl_sum = ecl_sum_alloc_writer( "CASE" , false , true , ":" , start_time , true , 10 , 10 , 10 );
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
  ecl_sum_fwrite( ecl_sum );
  return ecl_sum;
}

void test_load() {
  ERT::TestArea work_area("unsmry_loader");
  ecl_sum_type * ecl_sum = write_ecl_sum();
  test_assert_true( util_file_exists("CASE.SMSPEC") );
  test_assert_true( util_file_exists("CASE.UNSMRY") );
  ecl::unsmry_loader * loader = new ecl::unsmry_loader(ecl_sum_get_smspec(ecl_sum), "CASE.UNSMRY", 0);

  const std::vector<double> FOPT_value = loader->get_vector(1);
  const std::vector<double> BPR_value  = loader->get_vector(2);
  const std::vector<double> WWCT_value = loader->get_vector(3);
  test_assert_int_equal( FOPT_value.size(), 4 );
  test_assert_double_equal( FOPT_value[3] , 6.0 );
  test_assert_double_equal( BPR_value[2]  , 10.0 );
  test_assert_double_equal( WWCT_value[1] , 10.0 );

  delete loader;
  ecl_sum_free(ecl_sum);
}

int main() {
  test_load();
  return 0;
}

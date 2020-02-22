#include <vector>

#include <ert/util/test_work_area.h>
#include <ert/util/test_util.hpp>
#include <ert/util/double_vector.h>
#include <ert/util/vector.h>

#include <ert/ecl/ecl_endian_flip.hpp>
#include <ert/ecl/ecl_type.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_sum.hpp>
#include <ert/ecl/ecl_sum_vector.hpp>


/*

CASE1
---------
1 : BPR:1     100      200      300      400     500   600   700   800   900   ....
2 : BPR:2     110      210      310      410     510   610   710   810   910   ....
3 : BRP:3     120      220      320      420     520   620   720   820   920   ....



CASE2
---------
1 : BRP:2                            1100      2100     3100    4100
2 : BPR:1                            1000      2000     3000    4000



Total CASE2:
------------
1 : BPR:2   110  210  310  1100   2100   3100  4100
2 : BRP:1   100  200  300  1000   2000   3000  4000



CASE3
-----
1 : BPR:3                                            12000   22000   32000   42000
2 : BRP:2                                            11000   21000   31000   41000
3 : BPR:1                                            10000   20000   30000   40000


Total CASE3:
------------
1 : BPR:3   120   220   320  0     0     12000  22000  32000  42000
2 : BPR:2   110   210   310  1100  2100  11000  21000  31000  41000
3 : BPR:1   100   200   300  1000  2000  10000  20000  30000  40000

*/


#define ieq(d,i,v) test_assert_double_equal(double_vector_iget(d,(i)), v)


void verify_CASE1(int length) {
  ecl_sum_type * sum = ecl_sum_fread_alloc_case("CASE1", ":");
  int params_size = 4;  // TIME, BPR:1, BPR:2, BPR:3

  for (int i=1; i < params_size; i++) {
    double_vector_type * d = ecl_sum_alloc_data_vector(sum, i, false);
    test_assert_int_equal(length, double_vector_size(d));
    for (int j=0; j < length; j++) {
      test_assert_double_equal( double_vector_iget(d, j), (i - 1)*10 + (j + 1)*100);
    }
    double_vector_free(d);
  }
  ecl_sum_free(sum);
}


void verify_CASE2(int length) {
  ecl_sum_type * sum = ecl_sum_fread_alloc_case2__("CASE2", ":", false, true, 0);
  const int params_size = 2; // TIME, BPR:2, BPR:1

  for (int i=0; i < params_size; i++) {
    double_vector_type * d = ecl_sum_alloc_data_vector(sum, i+1, false);
    test_assert_int_equal(length, double_vector_size(d));
    for (int j=1; j < length; j++)
      test_assert_double_equal( double_vector_iget(d, j-1), (1 - i)*100 + j*1000);
    double_vector_free(d);
  }

  ecl_sum_free(sum);

  sum = ecl_sum_fread_alloc_case("CASE2", ":");
  for (int i=0; i < params_size; i++) {
    double_vector_type * d = ecl_sum_alloc_data_vector(sum, i+1, false);
    test_assert_int_equal(double_vector_size(d), 4);
    ieq(d,0,(1 - i)*100 + 1000);
    ieq(d,1,(1 - i)*100 + 2000);
    ieq(d,2,(1 - i)*100 + 3000);
    ieq(d,3,(1 - i)*100 + 4000);
    double_vector_free(d);
  }

  ecl_sum_free(sum);
}


void write_CASE1(bool unified) {
  time_t start_time = util_make_date_utc( 1,1,2010 );
  ecl_sum_type * ecl_sum = ecl_sum_alloc_writer( "CASE1" , false , unified, ":" , start_time , true , 10 , 10 , 10 );
  double sim_seconds = 0;

  int num_dates = 100;
  double ministep_length = 86400; // seconds in a day

  const ecl::smspec_node * node1 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 1   , "BARS"    , 0.0  );
  const ecl::smspec_node * node2 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 2   , "BARS"    , 0.0  );
  const ecl::smspec_node * node3 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 3   , "BARS"    , 0.0  );

  for (int report_step = 0; report_step < num_dates; report_step++) {
      {
        ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 1 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , *node1 , (1 + report_step) * 100 );
        ecl_sum_tstep_set_from_node( tstep , *node2 , (1 + report_step) * 100 + 10.0 );
        ecl_sum_tstep_set_from_node( tstep , *node3 , (1 + report_step) * 100 + 20.0 );
      }
      sim_seconds += ministep_length * 3;
  }
  ecl_sum_fwrite( ecl_sum );
  ecl_sum_free(ecl_sum);

  verify_CASE1(num_dates);
}




void write_CASE2() {
  bool unified = false;
  write_CASE1(unified);
  {
    time_t start_time = util_make_date_utc( 1,1,2010 );
    int num_dates = 4;
    double ministep_length = 86400; // seconds in a day
    double sim_seconds = ministep_length * 2.5 * 3;
    ecl_sum_type * ecl_sum = ecl_sum_alloc_restart_writer( "CASE2" , "CASE1", false , true , ":" , start_time , true , 10 , 10 , 10 );

    const ecl::smspec_node * node2 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 2   , "BARS"    , 0.0  );
    const ecl::smspec_node * node1 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 1   , "BARS"    , 0.0  );

    for (int report_step = 1; report_step <= num_dates; report_step++) {
      {
        ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 3 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , *node1 , report_step*1000);
        ecl_sum_tstep_set_from_node( tstep , *node2 , report_step*1000 + 100);
      }
      sim_seconds += ministep_length * 3;
    }
    ecl_sum_fwrite( ecl_sum );
    ecl_sum_free(ecl_sum);

    util_unlink("CASE1.SMSPEC");

    verify_CASE2(num_dates);
  }
}

int main() {
  write_CASE2();
  return 0;
}

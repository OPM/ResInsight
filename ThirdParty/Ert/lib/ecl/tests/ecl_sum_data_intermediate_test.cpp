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
    test_assert_int_equal(double_vector_size(d), 7);
    ieq(d,0,(1 - i)*10  + 100);
    ieq(d,1,(1 - i)*10  + 200);
    ieq(d,2,(1 - i)*10  + 300);
    ieq(d,3,(1 - i)*100 + 1000);
    ieq(d,4,(1 - i)*100 + 2000);
    ieq(d,5,(1 - i)*100 + 3000);
    ieq(d,6,(1 - i)*100 + 4000);
    double_vector_free(d);
  }

  ecl_sum_free(sum);
}


void write_CASE2(bool unified) {
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
    verify_CASE2(num_dates);
  }
}

void verify_CASE3(int length) {
  const int params_size = 3;
  ecl_sum_type * sum = ecl_sum_fread_alloc_case2__("CASE3", ":", false, true, 0);

  for (int i=0; i < params_size; i++) {
    double_vector_type * d = ecl_sum_alloc_data_vector(sum, i+1, false);
    test_assert_int_equal(length, double_vector_size(d));
    for (int j=0; j < length; j++) {
      test_assert_double_equal( double_vector_iget(d, j), (2 - i)*1000 + (j + 1)*10000);
    }
    double_vector_free(d);
  }
  ecl_sum_free(sum);

  sum = ecl_sum_fread_alloc_case("CASE3", ":");
  for (int i=0; i < params_size; i++) {
    double_vector_type * d = ecl_sum_alloc_data_vector(sum, i+1, false);
    ieq(d,0,(2 - i)*10  + 100);
    ieq(d,1,(2 - i)*10  + 200);
    ieq(d,2,(2 - i)*10  + 300);

    if (i == 0) {
      const ecl::smspec_node& node = ecl_smspec_iget_node_w_params_index(ecl_sum_get_smspec(sum), i);
      double default_value = node.get_default();
      ieq(d,3,default_value);
      ieq(d,4,default_value);
    } else {
      ieq(d,3,(2 - i)*100 + 1000);
      ieq(d,4,(2 - i)*100 + 2000);
    }
    ieq(d,5,(2 - i)*1000 + 10000);
    ieq(d,6,(2 - i)*1000 + 20000);
    ieq(d,7,(2 - i)*1000 + 30000);
    ieq(d,8,(2 - i)*1000 + 40000);
    double_vector_free(d);
  }

  ecl_sum_vector_type * vector = ecl_sum_vector_alloc(sum, true);
  double frame[27]; //3 vectors X 9 data points pr. vector
  ecl_sum_init_double_frame(sum, vector, frame);
  ecl_sum_vector_free(vector);

  ecl_sum_free(sum);
}


void write_CASE3(bool unified) {
  write_CASE2(unified);
  {
    time_t start_time = util_make_date_utc( 1,1,2010 );
    int num_dates = 4;
    double ministep_length = 86400; // seconds in a day
    double sim_seconds = ministep_length * 4.0 * 3;
    ecl_sum_type * ecl_sum = ecl_sum_alloc_restart_writer( "CASE3" , "CASE2", false , true , ":" , start_time , true , 10 , 10 , 10 );

    const ecl::smspec_node * node3 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 3   , "BARS"    , 0.0  );
    const ecl::smspec_node * node2 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 2   , "BARS"    , 0.0  );
    const ecl::smspec_node * node1 = ecl_sum_add_var( ecl_sum , "BPR" , NULL   , 1   , "BARS"    , 0.0  );

    for (int report_step = 1; report_step <= num_dates; report_step++) {
      {
        ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 5 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , *node1 , report_step*10000);
        ecl_sum_tstep_set_from_node( tstep , *node2 , report_step*10000 + 1000);
        ecl_sum_tstep_set_from_node( tstep , *node3 , report_step*10000 + 2000);
      }
      sim_seconds += ministep_length * 3;
    }
    ecl_sum_fwrite( ecl_sum );
    ecl_sum_free(ecl_sum);
    verify_CASE3(num_dates);
  }
}

void verify_CASE4() {
  ecl_sum_type * sum = ecl_sum_fread_alloc_case("CASE4", ":");

  double_vector_type * d;
  d = ecl_sum_alloc_data_vector(sum, 0, false); double_vector_free(d);
  d = ecl_sum_alloc_data_vector(sum, 1, false); double_vector_free(d);
  d = ecl_sum_alloc_data_vector(sum, 2, false); double_vector_free(d);
  d = ecl_sum_alloc_data_vector(sum, 4, false);
  ieq(d, 0, -99);
  ieq(d, 4, -99);
  ieq(d, 5, 10000);
  ieq(d, 8, 40000);
  double_vector_free(d);

  ecl_sum_vector_type * vector = ecl_sum_vector_alloc(sum, true);
  double frame[27]; //3 vectors X 9 data points pr. vector
  ecl_sum_init_double_frame(sum, vector, frame);
  test_assert_double_equal(frame[26], 40000);
  ecl_sum_vector_free(vector);

  ecl_sum_free(sum);
}


void write_CASE4(bool unified) {
  ecl::util::TestArea ta("case4");
  write_CASE3(unified);
  {
    ecl_file_type * sum_file    = ecl_file_open("CASE3.UNSMRY", 0);
    ecl_file_type * smspec_file = ecl_file_open("CASE3.SMSPEC", 0);

    ecl_kw_type * keywords = ecl_file_iget_named_kw(smspec_file, "KEYWORDS", 0);
    ecl_kw_resize(keywords, 5);
    ecl_kw_iset_char_ptr(keywords, 3, "WTPRWI1");
    ecl_kw_iset_char_ptr(keywords, 4, "BPR");

    ecl_kw_type * nums = ecl_file_iget_named_kw(smspec_file, "NUMS", 0);
    ecl_kw_resize(nums, 5);
    unsigned int * nums_ptr = (unsigned int *)ecl_kw_get_int_ptr(nums);
    nums_ptr[3] = 5;
    nums_ptr[4] = 8;  //a different

    ecl_kw_type * wgnames = ecl_file_iget_named_kw(smspec_file, "WGNAMES", 0);
    ecl_kw_resize(wgnames, 5);
    ecl_kw_iset_char_ptr(wgnames, 4, ":+:+:+:+");

    ecl_kw_type * units = ecl_file_iget_named_kw(smspec_file, "UNITS", 0);
    ecl_kw_resize(units, 5);
    ecl_kw_iset_char_ptr(units, 4, "BARS");

    int num_params = ecl_file_get_num_named_kw(sum_file, "PARAMS");
    for (int i = 0; i < num_params; i++) {
      ecl_kw_type * params_kw = ecl_file_iget_named_kw(sum_file, "PARAMS", i);
      ecl_kw_resize(params_kw, 5);
      float * ptr = (float*)ecl_kw_get_void_ptr(params_kw);
      ptr[4] = ptr[3];
      ptr[3] = -1;
    }

    fortio_type * f;
    const char * filename_sum = "CASE4.UNSMRY";
    f = fortio_open_writer( filename_sum, false, ECL_ENDIAN_FLIP );
    ecl_file_fwrite_fortio(sum_file, f, 0);
    fortio_fclose( f );

    const char * filename_smspec = "CASE4.SMSPEC";
    f = fortio_open_writer( filename_smspec, false, ECL_ENDIAN_FLIP );
    ecl_file_fwrite_fortio(smspec_file, f, 0);
    fortio_fclose( f );

    ecl_file_close(smspec_file);
    ecl_file_close(sum_file);
    verify_CASE4();
  }
}

int main() {
  write_CASE4(true);
  write_CASE4(false);
  return 0;
}

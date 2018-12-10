#include <vector>

#include <ert/util/test_work_area.hpp>
#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_endian_flip.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_grid.hpp>


void test_1() {

  test_work_area_type * work_area = test_work_area_alloc("ext_actnum_main_grid");
  {
    const char * filename = "FILE.EGRID";

    ecl_grid_type * grid_write = ecl_grid_alloc_rectangular(2,3,1,   1,1,1,NULL);
    ecl_grid_fwrite_EGRID( grid_write , filename, true);
    ecl_grid_free(grid_write);

    ecl_file_type * ecl_file = ecl_file_open(filename, 0);
    ecl_kw_type * filehead_kw  = ecl_file_iget_named_kw( ecl_file , FILEHEAD_KW  , 0);
    int * filehead_data = ecl_kw_get_int_ptr(filehead_kw);
    filehead_data[FILEHEAD_DUALP_INDEX] = FILEHEAD_DUAL_POROSITY;

    ecl_kw_type * actnum_kw = ecl_file_iget_named_kw( ecl_file, ACTNUM_KW, 0);
    int * actnum_data = ecl_kw_get_int_ptr(actnum_kw);
    actnum_data[0] = 1;
    actnum_data[1] = 2;
    actnum_data[2] = 2;
    actnum_data[3] = 0;
    actnum_data[4] = 1;
    actnum_data[5] = 1;
    const char * filename1 = "FILE1.EGRID";
    fortio_type * f = fortio_open_writer( filename1, false, ECL_ENDIAN_FLIP );
    ecl_file_fwrite_fortio(ecl_file, f, 0);
    fortio_fclose( f );
    ecl_file_close(ecl_file);

    std::vector<int> ext_actnum = {0, 1, 0, 1, 1, 1};
    ecl_grid_type * grid = ecl_grid_alloc_ext_actnum(filename1, ext_actnum.data());
    test_assert_int_equal( 2, ecl_grid_get_nactive(grid) );
    test_assert_int_equal( 1, ecl_grid_get_nactive_fracture(grid) );
    test_assert_true( !ecl_grid_cell_active1(grid, 0) );

    test_assert_true( !ecl_grid_cell_active1(grid, 2) );
    test_assert_true( !ecl_grid_cell_active1(grid, 3) );
    test_assert_true(  ecl_grid_cell_active1(grid, 4) );
    test_assert_true(  ecl_grid_cell_active1(grid, 5) );

    ecl_grid_free( grid );

  }
  test_work_area_free( work_area );
  

}


int main() {
  test_1();
}

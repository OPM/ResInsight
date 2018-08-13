/*
  Copyright 2015 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdexcept>
#include <fstream>

#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_file.hpp>

#include <ert/ecl/EclKW.hpp>
#include <ert/ecl/FortIO.hpp>
#include <ert/util/TestArea.hpp>

void test_kw_name() {
    ERT::EclKW< int > kw1( "short", 1 );
    ERT::EclKW< int > kw2( "verylong", 1 );

    test_assert_string_equal( kw1.name(), "short" );
    test_assert_string_equal( kw2.name(), "verylong" );
}

void test_kw_vector_assign() {
    std::vector< int > vec = { 1, 2, 3, 4, 5 };
    ERT::EclKW< int > kw( "XYZ", vec );

    test_assert_size_t_equal( kw.size(), vec.size() );

    for( size_t i = 0; i < kw.size(); ++i ) {
        test_assert_int_equal( kw.at( i ), vec[ i ] );
        test_assert_int_equal( kw[ i ] , vec[ i ] );
    }

    for( size_t i = 0; i < kw.size(); ++i ) {
        kw[i] *= 2;
        test_assert_int_equal( kw[ i ] , 2*vec[ i ] );
    }
}

void test_kw_vector_string() {
    std::vector< const char* > vec = {
        "short",
        "sweet",
        "padded  "
    };

    std::vector<const char *> too_long = {"1234567890"};
    ERT::EclKW< const char* > kw( "XYZ", vec );

    test_assert_size_t_equal( kw.size(), vec.size() );

    test_assert_string_equal( kw.at( 0 ), "short   " );
    test_assert_string_equal( kw.at( 1 ), "sweet   " );
    test_assert_string_equal( kw.at( 2 ), vec.at( 2 ) );

    test_assert_throw( ERT::EclKW<const char*>("XY", too_long), std::range_error);
}

void test_kw_vector_std_string() {
  std::vector< const char* > vec = {
    "short",
    "sweet",
    "padded  ",
  };
  std::vector<std::string> too_long = {"1234567890"};
  ERT::EclKW< std::string > kw( "XYZ", vec );

  test_assert_size_t_equal( kw.size(), vec.size() );

  test_assert_string_equal( kw.at( 0 ).c_str(), "short   " );
  test_assert_string_equal( kw.at( 1 ).c_str(), "sweet   " );
  test_assert_string_equal( kw.at( 2 ).c_str(), vec.at( 2 ) );

  test_assert_throw( ERT::EclKW<std::string>("XY", too_long), std::range_error);
}

void test_logical() {
  //std::vector<bool> vec = {true,false,true,false};
  //  ERT::EclKW<bool> kw("BOOL", vec);
  //  test_assert_int_equal(kw.size(), vec.size());

  // for (size_t i=0; i < vec.size(); i++)
  //       test_assert_true( kw.at(i) == vec[i] );
}


void test_move_semantics_no_crash() {
    std::vector< int > vec = { 1, 2, 3, 4, 5 };
    ERT::EclKW< int > kw1( "XYZ", vec );

    ERT::EclKW< int > kw2( std::move( kw1 ) );
    test_assert_true( kw1.get() == nullptr );
}

void test_exception_assing_ref_wrong_type() {
    auto* ptr = ecl_kw_alloc( "XYZ", 1, ECL_INT );

    try {
        ERT::EclKW< double > kw( ptr );
        test_assert_true( false );
    } catch (...) {
        ERT::EclKW< int > kw( ptr );
    }
}

void test_resize() {
    ERT::EclKW< int > kw1( "short", 1 );

    test_assert_int_equal( kw1.size() , 1 );
    kw1.resize( 100 );
    test_assert_int_equal( kw1.size() , 100 );
}

void test_data() {
  std::vector<double> d_data = {1,2,3,4};
  std::vector<float> f_data = {10,20,30,40};
  std::vector<int> i_data = {100,200,300,400};
  std::vector<bool> b_data = {true,false};
  std::vector<std::string> s_data = {"S1", "S2", "S3"};

  ERT::EclKW<double> d_kw("DOUBLE", d_data);
  auto d_data2 = d_kw.data();
  for (size_t i=0; i < d_data.size(); i++)
    test_assert_true(d_data[i] == d_data2[i]);

  ERT::EclKW<float> f_kw("FLOATx", f_data);
  auto f_data2 = f_kw.data();
  for (size_t i=0; i < f_data.size(); i++)
    test_assert_true(f_data[i] == f_data2[i]);

  ERT::EclKW<int> i_kw("INT", i_data);
  auto i_data2 = i_kw.data();
  for (size_t i=0; i < i_data.size(); i++)
    test_assert_true(i_data[i] == i_data2[i]);

  //ERT::EclKW<bool> b_kw("bbb", b_data);
  //auto b_data2 = b_kw.data();
  //for (size_t i=0; i < b_data.size(); i++)
  //  test_assert_true(b_data[i] == b_data2[i]);

  ERT::EclKW<std::string> s_kw("sss", s_data);
  auto s_data2 = s_kw.data();
  for (size_t i=0; i < s_data.size(); i++)
    test_assert_true(s_data[i] == s_data2[i]);

}


void test_read_write() {
    std::vector<double> d_data = {1,2,3,4};
    std::vector<float> f_data = {10,20,30,40};
    std::vector<int> i_data = {100,200,300,400};
    std::vector<bool> b_data = {true,false};
    std::vector<std::string> s_data = {"S1", "S2", "S3"};

    {
        ERT::TestArea ta("test_fwrite");
        {
          ERT::FortIO f("test_file", std::ios_base::out);
          ERT::write_kw(f, "DOUBLE", d_data);
          ERT::write_kw(f, "FLOAT", f_data);
          ERT::write_kw(f, "INT", i_data);
          ERT::write_kw(f, "BOOL", b_data);
          ERT::write_kw(f, "STRING", s_data);
        }

        {
          ecl_file_type * f = ecl_file_open("test_file", 0);
          ecl_kw_type * d_kw = ecl_file_iget_named_kw(f, "DOUBLE", 0);
          ecl_kw_type * f_kw = ecl_file_iget_named_kw(f, "FLOAT", 0);
          ecl_kw_type * i_kw = ecl_file_iget_named_kw(f, "INT", 0);
          ecl_kw_type * b_kw = ecl_file_iget_named_kw(f, "BOOL", 0);
          ecl_kw_type * s_kw = ecl_file_iget_named_kw(f, "STRING", 0);

          for (size_t i=0; i < d_data.size(); i++)
            test_assert_true(d_data[i] == ecl_kw_iget_double(d_kw,i));

          for (size_t i=0; i < f_data.size(); i++)
            test_assert_true(f_data[i] == ecl_kw_iget_float(f_kw,i));

          for (size_t i=0; i < i_data.size(); i++)
            test_assert_true(i_data[i] == ecl_kw_iget_int(i_kw,i));

          for (size_t i=0; i < b_data.size(); i++)
            test_assert_true(b_data[i] == ecl_kw_iget_bool(b_kw,i));

          for (size_t i=0; i < s_data.size(); i++) {
            std::string s8 = ecl_kw_iget_char_ptr(s_kw, i);
            test_assert_int_equal(s8.size(), 8);
            s8.erase(s8.find_last_not_of(' ')+1);
            test_assert_true( s_data[i] == s8);
          }

          ecl_file_close(f);
        }
    }
}



int main (int argc, char **argv) {
    test_kw_name();
    test_kw_vector_assign();
    test_kw_vector_string();
    test_kw_vector_std_string();
    test_logical();
    test_move_semantics_no_crash();
    test_exception_assing_ref_wrong_type();
    test_resize();
    test_data();
    test_read_write();
}


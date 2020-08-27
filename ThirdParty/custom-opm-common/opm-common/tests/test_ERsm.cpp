/*
  Copyright 2020 Equinor ASA.

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
#define BOOST_TEST_MODULE EclipseRSMLoader

#include <boost/test/unit_test.hpp>

#include <fstream>
#include <opm/io/eclipse/ERsm.hpp>
#include <tests/WorkArea.cpp>
#include <opm/common/utility/FileSystem.hpp>

Opm::EclIO::ERsm create(const std::string& rsm_data) {
    WorkArea work_area("test_ERsm");
    {
        std::ofstream os("TEST.RSM");
        os << rsm_data;
    }
    return Opm::EclIO::ERsm("TEST.RSM");
}


// Dont touch the string literals - it is all part of the format; down to the number of whitespace ... :-(

std::string block1_days = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------
 SUMMARY OF RUN nor01-temp01-rsm-all OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                                
 -------------------------------------------------------------------------------------------------------------------------------
 TIME         YEARS        FOPR         GOPR         GOPR         GOPR         GOPR         GOPR         GOPR         GOPR         
 DAYS         YEARS        SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      
                                        MANI-C       B1-DUMMY     MANI-D1      INJE         PROD         MANI-B2      MANI-B1      
                                                                                                                                   
 -------------------------------------------------------------------------------------------------------------------------------
        0            0            0            0            0            0            0            0            0            0     
 1.000000     0.002738     4379.802            0            0     4379.802            0     4379.802            0            0     
 4.000000     0.010951     4380.562            0            0     4380.562            0     4380.562            0            0     
 8.000000     0.021903     4381.301            0            0     4381.301            0     4381.301            0            0     
)";

std::string block1_hours = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------
 SUMMARY OF RUN nor01-temp01-rsm-all OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                                
 -------------------------------------------------------------------------------------------------------------------------------
 TIME         YEARS        FOPR         GOPR         GOPR         GOPR         GOPR         GOPR         GOPR         GOPR         
 HOURS        YEARS        SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      SM3/DAY      
                                        MANI-C       B1-DUMMY     MANI-D1      INJE         PROD         MANI-B2      MANI-B1      
                                                                                                                                   
 -------------------------------------------------------------------------------------------------------------------------------
        0            0            0            0            0            0            0            0            0            0     
 1.000000     0.002738     4379.802            0            0     4379.802            0     4379.802            0            0     
 4.000000     0.010951     4380.562            0            0     4380.562            0     4380.562            0            0     
 8.000000     0.021903     4381.301            0            0     4381.301            0     4381.301            0            0     
)";



std::string block1_date = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------
 SUMMARY OF RUN nor01-temp01-rsm-date OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                               
 -------------------------------------------------------------------------------------------------------------------------------
 DATE         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWCT         
              SM3          SM3          SM3          SM3          SM3          SM3          SM3          SM3                       
              *10**3                                              *10**3       *10**3       *10**3                                 
              C-2H         C-3H         C-4H         C-4AH        F-1H         F-2H         F-3H         F-4H         C-4H         
                                                                                                                                   
 -------------------------------------------------------------------------------------------------------------------------------
  1-MAR-1999  227.7381            0     46279.20            0            0            0            0            0            0     
  3-MAR-1999  238.5447            0     63147.95            0            0            0            0            0            0     
  4-MAR-1999  243.9480            0     71582.33            0            0            0            0            0            0     
  5-MAR-1999  249.3513            0     80016.70            0            0            0            0            0            0     
)";

std::string block2_date = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------
 SUMMARY OF RUN nor01-temp01-rsm-date OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                               
 -------------------------------------------------------------------------------------------------------------------------------
 DATE         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWCT         
              SM3          SM3          SM3          SM3          SM3          SM3          SM3          SM3                       
              *10**3                                              *10**3       *10**3       *10**3                                 
              C-2H         C-3H         C-4H         C-4AH        F-1H         F-2H         F-3H         F-4H         C-4H         
                                                                                                                                   
 -------------------------------------------------------------------------------------------------------------------------------
  1-MAR-1999  227.7381            0     46279.20            0            0            0            0            0            0     
  3-MAR-1999  238.5447            0     63147.95            0            0            0            0            0            0     
  4-MAR-1999  243.9480            0     71582.33            0            0            0            0            0            0     
)";

std::string missing_time = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------
 SUMMARY OF RUN nor01-temp01-rsm-date OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                               
 -------------------------------------------------------------------------------------------------------------------------------
 XDATE        WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWIT         WWCT         
              SM3          SM3          SM3          SM3          SM3          SM3          SM3          SM3                       
              *10**3                                              *10**3       *10**3       *10**3                                 
              C-2H         C-3H         C-4H         C-4AH        F-1H         F-2H         F-3H         F-4H         C-4H         
                                                                                                                                   
 -------------------------------------------------------------------------------------------------------------------------------
  1-MAR-1999  227.7381            0     46279.20            0            0            0            0            0            0     
)";


std::string no_wgnames = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------   
 SUMMARY OF RUN SPE1CASE1 OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                                           
 -------------------------------------------------------------------------------------------------------------------------------   
 TIME         DAY          MONTH        YEAR         BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        
 DAYS                                                                                                                              
                                                                                                                                   
                                                     1            10           100          101          110          200          
 -------------------------------------------------------------------------------------------------------------------------------   
        7            8            1         2015            0            0            0            0            0            0     
       14           15            1         2015            0            0            0            0            0            0     
       21           22            1         2015            0            0            0            0            0            0     
       28           29            1         2015            0            0            0            0            0            0     
       31            1            2         2015            0            0            0            0            0            0     
       38            8            2         2015            0            0            0            0            0            0     
       45           15            2         2015            0            0            0            0            0            0     
)";


std::string multiple_blocks = R"(1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------   
 SUMMARY OF RUN SPE1CASE1 OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                                           
 -------------------------------------------------------------------------------------------------------------------------------   
 DATE         BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        BGSAT        
                                                                                                                                   
                                                                                                                                   
              1            10           100          101          110          200          201          210          300          
 -------------------------------------------------------------------------------------------------------------------------------   
  8-JAN-2015         0            0            0            0            0            0            0            0            0     
 15-JAN-2015         0            0            0            0            0            0            0            0            0     
 22-JAN-2015         0            0            0            0            0            0            0            0            0     
 29-JAN-2015         0            0            0            0            0            0            0            0            0     
  1-FEB-2015         0            0            0            0            0            0            0            0            0     
1                                                                                                                                  
 -------------------------------------------------------------------------------------------------------------------------------   
 SUMMARY OF RUN SPE1CASE1 OPM FLOW VERSION 1910 ANYTHING CAN GO HERE: USER, MACHINE ETC.                                           
 -------------------------------------------------------------------------------------------------------------------------------   
 DATE         BPR          BPR          FGOR         FOPR         WBHP         WBHP         WGIR         WGIR         WGIT         
              PSIA         PSIA         MSCF/STB     STB/DAY      PSIA         PSIA         MSCF/DAY     MSCF/DAY     MSCF         
                                                      *10**3                                                                       
                                                                  INJ          PROD         INJ          PROD         INJ          
              1            300                                                                                                     
 -------------------------------------------------------------------------------------------------------------------------------   
  8-JAN-2015         0            0           -0     604.7999            0            0            0            0            0     
 15-JAN-2015         0            0           -0     1209.599            0            0            0            0            0     
 22-JAN-2015         0            0           -0     1814.400            0            0            0            0            0     
 29-JAN-2015         0            0           -0     2419.199            0            0            0            0            0     
  1-FEB-2015         0            0           -0     2678.399            0            0            0            0            0     
)";


BOOST_AUTO_TEST_CASE(ERsm) {
    BOOST_CHECK_THROW(Opm::EclIO::ERsm("file/does/not/exist"), std::invalid_argument);
    BOOST_CHECK_THROW( create("SomeThingInvalid"), std::invalid_argument);
    BOOST_CHECK_THROW( create( block1_date + block2_date ) , std::invalid_argument );
    BOOST_CHECK_THROW( create( block1_hours ) , std::invalid_argument );
    auto rsm1 = create(block1_days);
    auto rsm2 = create(block1_date);

    BOOST_CHECK_THROW( rsm1.dates(), std::invalid_argument);
    BOOST_CHECK_THROW( rsm2.days(),  std::invalid_argument);
    BOOST_CHECK_THROW( rsm1.get("NO_SUCH_KEY"), std::out_of_range);

    const auto& wwit_c_2h = rsm2.get("WWIT:C-2H");
    BOOST_CHECK_EQUAL( wwit_c_2h.size(), 4 );
    std::vector<double> expected = {227.7381, 238.5447, 243.9480, 249.3513};
    for (std::size_t index = 0; index < 4; index++)
        BOOST_CHECK_CLOSE( expected[index] * 1000 , wwit_c_2h[index], 1e-6);

    const auto& wwit_c_4h = rsm2.get("WWIT:C-4H");
    BOOST_CHECK_EQUAL( wwit_c_4h.size(), 4 );
    std::vector<double> expected2 = {46279.20, 63147.95, 71582.33, 80016.70};
    for (std::size_t index = 0; index < 4; index++)
        BOOST_CHECK_CLOSE( expected2[index] * 1 , wwit_c_4h[index], 1e-6);

    std::vector<double> expected_days = {0, 1, 4 , 8};
    BOOST_CHECK( rsm1.days() ==  expected_days);

    std::vector<Opm::TimeStampUTC> expected_dates = {{1999, 3, 1},
                                                     {1999, 3, 3},
                                                     {1999, 3, 4},
                                                     {1999, 3, 5}};
    BOOST_CHECK( expected_dates == rsm2.dates() );
    BOOST_CHECK( rsm2.has_dates() );
    BOOST_CHECK( !rsm1.has_dates() );
    BOOST_CHECK( rsm2.has("WWIT:C-2H") );
    BOOST_CHECK( !rsm2.has("NO_SUCH_KEY") );

    auto nums = create( no_wgnames );
    BOOST_CHECK( nums.has("BGSAT:1"));

    auto fopr_rsm = create( multiple_blocks );
    const auto& fopr = fopr_rsm.get("FOPR");
    std::vector<double> expected_fopr = {604799.9, 1209599, 1814400, 2419199, 2678399};
    BOOST_CHECK(fopr == expected_fopr);
}

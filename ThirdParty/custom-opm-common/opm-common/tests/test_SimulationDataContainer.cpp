/*
  Copyright 2016 Statoil ASA.

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

#include <config.h>

#define BOOST_TEST_MODULE SIMULATION_DATA_CONTAINER_TESTS
#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <iostream>
#include <opm/common/data/SimulationDataContainer.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(TestCreate) {
    SimulationDataContainer container(1000 , 10 , 2);

    BOOST_CHECK_EQUAL( 2U    , container.numPhases() );
    BOOST_CHECK_EQUAL( 1000U , container.numCells() );
    BOOST_CHECK_EQUAL( 10U   , container.numFaces() );
}




/*
  This test verifies that the default fields are correctly registered;
  this special behavior is deprecated - and the test should die; along
  with the behavior.
*/

BOOST_AUTO_TEST_CASE(TestRegisterDefaults) {
    SimulationDataContainer container(1000 , 10 , 2);

    BOOST_CHECK( container.hasCellData("PRESSURE") );
    BOOST_CHECK( container.hasCellData("SATURATION") );

    {
        auto pressure = container.getCellData("PRESSURE");
        BOOST_CHECK_EQUAL( pressure.size() , 1000U );
        BOOST_CHECK_EQUAL( container.numCellDataComponents( "PRESSURE") , 1U);

        auto sat = container.getCellData("SATURATION");
        BOOST_CHECK_EQUAL( sat.size() , 1000U*2 );
        BOOST_CHECK_EQUAL( container.numCellDataComponents( "SATURATION") , 2U);
    }

    {
        auto pressure = container.pressure();
        BOOST_CHECK_EQUAL( pressure.size() , 1000U );

        auto sat = container.saturation();
        BOOST_CHECK_EQUAL( sat.size() , 1000U*2 );
    }

    BOOST_CHECK( container.hasFaceData("FACEPRESSURE") );
    BOOST_CHECK( container.hasFaceData("FACEFLUX") );
}




BOOST_AUTO_TEST_CASE(TestRegisterFaceData) {
    SimulationDataContainer container(100 , 10 , 2);
    BOOST_CHECK( !container.hasFaceData("FLUX"));
    BOOST_CHECK_THROW( container.getFaceData("FLUX") , std::invalid_argument );

    container.registerFaceData("FLUX" , 1 , 99 );
    auto& flux = container.getFaceData("FLUX");
    BOOST_CHECK_EQUAL( flux.size() , 10U );
    BOOST_CHECK_EQUAL( flux[0] , 99 );
}



BOOST_AUTO_TEST_CASE(TestRegisterCellData) {

    SimulationDataContainer container(100 , 10 , 2);
    BOOST_CHECK( !container.hasCellData("FIELDX"));
    BOOST_CHECK_THROW( container.getCellData("FIELDX") , std::invalid_argument );

    container.registerCellData("FIELDX" , 1 , 123 );
    {
        auto& fieldx = container.getCellData("FIELDX");
        BOOST_CHECK_EQUAL( fieldx.size() , 100U );
        for (auto v : fieldx)
            BOOST_CHECK_EQUAL( v , 123 );

        fieldx[0] *= 2;
    }

    {
        auto fieldx = container.getCellData("FIELDX");
        BOOST_CHECK_EQUAL( fieldx[0] , 246 );
        BOOST_CHECK_EQUAL( fieldx[1] , 123 );
    }

}


BOOST_AUTO_TEST_CASE(Test_Equal) {
    {
        SimulationDataContainer container1(100 , 10 , 2);
        SimulationDataContainer container2(100 , 10 , 2);
        BOOST_CHECK( container1.equal( container2 ));
    }

    {
        SimulationDataContainer container1(100 , 10 , 2);
        SimulationDataContainer container2(100 , 10 , 1);
        BOOST_CHECK( !container1.equal( container2 ));
    }

    {
        SimulationDataContainer container1(100 , 10 , 2);
        SimulationDataContainer container2(100 , 10 , 2);

        container1.registerCellData( "FIELDX" , 1 , 123 );
        BOOST_CHECK( !container1.equal( container2 ));
        container2.registerCellData( "FIELDX" , 1 , 123 );
        BOOST_CHECK( container1.equal( container2 ));

        container1.registerFaceData( "FACEX" , 1 , 123 );
        BOOST_CHECK( !container1.equal( container2 ));
        container2.registerFaceData( "FACEX" , 1 , 123 );
        BOOST_CHECK( container1.equal( container2 ));
    }

    {
        SimulationDataContainer container1(100 , 10 , 2);
        SimulationDataContainer container2(100 , 10 , 2);

        container1.registerCellData( "FIELD1" , 1 , 123 );
        container2.registerCellData( "FIELD2" , 1 , 123 );
        BOOST_CHECK( !container1.equal( container2 ));
    }

    {
        SimulationDataContainer container1(100 , 10 , 2);
        SimulationDataContainer container2(100 , 10 , 2);

        container1.registerFaceData( "FIELD1" , 1 , 123 );
        container2.registerFaceData( "FIELD2" , 1 , 123 );
        BOOST_CHECK( !container1.equal( container2 ));
    }

    {
        SimulationDataContainer container1(100 , 10 , 2);
        SimulationDataContainer container2(100 , 10 , 2);

        container1.registerFaceData( "FIELD1" , 1 , 123 );
        container2.registerFaceData( "FIELD1" , 1 , 123 );
        BOOST_CHECK( container1.equal( container2 ));

        std::vector<double>& f = container1.getFaceData( "FIELD1" );
        f[0] *= 1.1;
        BOOST_CHECK( !container1.equal( container2 ));
    }
}



BOOST_AUTO_TEST_CASE(TestSetComponent) {

    SimulationDataContainer container(100 , 10 , 2);
    container.registerCellData("FIELDX" , 2 , 123 );
    std::vector<int> cells = { 1,2,3};
    std::vector<int> cells2 = { 1,2,3,4};
    std::vector<int> cells3 = { 1,2,100};
    std::vector<double> values0 = {20,30,40};
    std::vector<double> values1 = {2,3,4};

    BOOST_CHECK_THROW( container.setCellDataComponent( "FIELDY" , 0 , cells , values0 ) , std::invalid_argument );
    BOOST_CHECK_THROW( container.setCellDataComponent( "FIELDX" , 2 , cells , values0 ) , std::invalid_argument );
    BOOST_CHECK_THROW( container.setCellDataComponent( "FIELDX" , 0 , cells2 , values0 ) , std::invalid_argument );
    BOOST_CHECK_THROW( container.setCellDataComponent( "FIELDX" , 0 , cells3 , values0 ) , std::invalid_argument );

    container.setCellDataComponent( "FIELDX" , 0 , cells , values0 );
    container.setCellDataComponent( "FIELDX" , 1 , cells , values1 );
    const auto& data = container.getCellData( "FIELDX" );

    BOOST_CHECK_EQUAL( data[1*2 + 1] , 2 );
    BOOST_CHECK_EQUAL( data[2*2 + 1] , 3 );
    BOOST_CHECK_EQUAL( data[3*2 + 1] , 4 );

    BOOST_CHECK_EQUAL( data[1*2] , 20 );
    BOOST_CHECK_EQUAL( data[2*2] , 30 );
    BOOST_CHECK_EQUAL( data[3*2] , 40 );

}

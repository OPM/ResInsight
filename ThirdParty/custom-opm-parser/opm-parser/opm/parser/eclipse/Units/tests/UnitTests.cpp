/*
  Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE UnitTests

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/ConversionFactors.hpp>

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace Opm;

BOOST_AUTO_TEST_CASE(CreateDimension) {
    Dimension length("Length" , 1);
    BOOST_CHECK_EQUAL("Length" , length.getName());
    BOOST_CHECK_EQUAL(1 , length.getSIScaling());
}

BOOST_AUTO_TEST_CASE(makeComposite) {
    std::shared_ptr<Dimension> composite(Dimension::newComposite("Length*Length*Length/Time" , 100));
    BOOST_CHECK_EQUAL("Length*Length*Length/Time" , composite->getName());
    BOOST_CHECK_EQUAL(100 , composite->getSIScaling());
}


BOOST_AUTO_TEST_CASE(CreateDimensionInvalidNameThrows) {
    BOOST_CHECK_THROW(Dimension(" " , 1) , std::invalid_argument);
    BOOST_CHECK_THROW(Dimension(".LX" , 1) , std::invalid_argument);
    BOOST_CHECK_THROW(Dimension("*" , 1) , std::invalid_argument);
    BOOST_CHECK_THROW(Dimension("/" , 1) , std::invalid_argument);
    BOOST_CHECK_THROW(Dimension("2" , 1) , std::invalid_argument);
    BOOST_CHECK_NO_THROW(Dimension("1" , 1));
}


BOOST_AUTO_TEST_CASE(CreateUnitSystem) {
    UnitSystem system(UnitSystem::UNIT_TYPE_METRIC);
    BOOST_CHECK_EQUAL("Metric" , system.getName());
}


BOOST_AUTO_TEST_CASE(UnitSystemEmptyHasNone) {
    UnitSystem system(UnitSystem::UNIT_TYPE_METRIC);
    BOOST_CHECK_EQUAL( false , system.hasDimension("Length"));
    BOOST_CHECK_EQUAL( false , system.hasDimension("LXY"));
}



BOOST_AUTO_TEST_CASE(UnitSystemGetMissingDimensionThrows) {
    UnitSystem system(UnitSystem::UNIT_TYPE_METRIC);
    BOOST_CHECK_THROW( system.getDimension("Length") , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(UnitSystemGetNewOK) {
    UnitSystem system(UnitSystem::UNIT_TYPE_METRIC);
    system.addDimension("Length" , 10 );
    system.addDimension("Time" , 100);

    BOOST_CHECK_EQUAL( false , system.hasDimension("Length*Length/Time"));
    std::shared_ptr<const Dimension> comp = system.getNewDimension("Length*Length/Time");
    BOOST_CHECK_EQUAL( true , system.hasDimension("Length*Length/Time"));
    BOOST_CHECK_EQUAL(1 , comp->getSIScaling());
}




BOOST_AUTO_TEST_CASE(UnitSystemAddDimensions) {
    UnitSystem system(UnitSystem::UNIT_TYPE_METRIC);
    system.addDimension("Length" , 1 );
    system.addDimension("Time" , 86400 );
    system.addDimension("Temperature", 1.0, 273.15);

    std::shared_ptr<const Dimension> length = system.getDimension("Length");
    std::shared_ptr<const Dimension> time = system.getDimension("Time");
    std::shared_ptr<const Dimension> temperature = system.getDimension("Temperature");
    BOOST_CHECK_EQUAL(1     , length->getSIScaling());
    BOOST_CHECK_EQUAL(86400 , time->getSIScaling());
    BOOST_CHECK_EQUAL(1.0   , temperature->getSIScaling());
    BOOST_CHECK_EQUAL(273.15, temperature->getSIOffset());

    system.addDimension("Length" , 0.3048);
    length = system.getDimension("Length");
    BOOST_CHECK_EQUAL(0.3048 , length->getSIScaling());
}


BOOST_AUTO_TEST_CASE(UnitSystemParseInvalidThrows) {
    UnitSystem system(UnitSystem::UNIT_TYPE_METRIC);
    BOOST_CHECK_THROW( system.parse("//") , std::invalid_argument);
    BOOST_CHECK_THROW( system.parse("Length * Length / Time") , std::invalid_argument);

    system.addDimension("Length" , 3.00 );
    system.addDimension("Time" , 9.0 );

    std::shared_ptr<const Dimension> volumePerTime = system.parse("Length*Length*Length/Time");
    BOOST_CHECK_EQUAL("Length*Length*Length/Time" , volumePerTime->getName() );
    BOOST_CHECK_EQUAL(3.0 , volumePerTime->getSIScaling());
}



static void checkSystemHasRequiredDimensions(std::shared_ptr<const UnitSystem> system) {
    BOOST_CHECK( system->hasDimension("1"));
    BOOST_CHECK( system->hasDimension("Length"));
    BOOST_CHECK( system->hasDimension("Mass"));
    BOOST_CHECK( system->hasDimension("Time"));
    BOOST_CHECK( system->hasDimension("Permeability"));
    BOOST_CHECK( system->hasDimension("Pressure"));
    BOOST_CHECK( system->hasDimension("Temperature"));
}



BOOST_AUTO_TEST_CASE(CreateMetricSystem) {
    std::shared_ptr<UnitSystem> system = std::shared_ptr<UnitSystem>( UnitSystem::newMETRIC() );
    checkSystemHasRequiredDimensions( system );

    BOOST_CHECK_EQUAL( Metric::Length       , system->getDimension("Length")->getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Mass         , system->getDimension("Mass")->getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Time         , system->getDimension("Time")->getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Permeability , system->getDimension("Permeability")->getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Pressure     , system->getDimension("Pressure")->getSIScaling() );
}



BOOST_AUTO_TEST_CASE(CreateFieldSystem) {
    std::shared_ptr<UnitSystem> system = std::shared_ptr<UnitSystem>( UnitSystem::newFIELD() );
    checkSystemHasRequiredDimensions( system );

    BOOST_CHECK_EQUAL( Field::Length       , system->getDimension("Length")->getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Mass         , system->getDimension("Mass")->getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Time         , system->getDimension("Time")->getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Permeability , system->getDimension("Permeability")->getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Pressure     , system->getDimension("Pressure")->getSIScaling() );
}




BOOST_AUTO_TEST_CASE(DimensionEqual) {
    Dimension d1("Length" , 1);
    Dimension d2("Length" , 1);
    Dimension d3("Time" , 1);
    Dimension d4("Length" , 2);

    BOOST_CHECK_EQUAL( true  , d1.equal(d1) );
    BOOST_CHECK_EQUAL( true  , d1.equal(d2) );
    BOOST_CHECK_EQUAL( false , d1.equal(d3) );
    BOOST_CHECK_EQUAL( false , d1.equal(d4) );
}


BOOST_AUTO_TEST_CASE(UnitSystemEqual) {
    std::shared_ptr<UnitSystem> metric1 = std::shared_ptr<UnitSystem>( UnitSystem::newMETRIC() );
    std::shared_ptr<UnitSystem> metric2 = std::shared_ptr<UnitSystem>( UnitSystem::newMETRIC() );
    std::shared_ptr<UnitSystem> field = std::shared_ptr<UnitSystem>( UnitSystem::newFIELD() );

    BOOST_CHECK_EQUAL( true   , metric1->equal( *metric1 ));
    BOOST_CHECK_EQUAL( true   , metric1->equal( *metric2 ));
    BOOST_CHECK_EQUAL( false  , metric1->equal( *field ));
    metric1->addDimension("g" , 3.00 );
    BOOST_CHECK_EQUAL( false  , metric1->equal( *metric2 ));
    BOOST_CHECK_EQUAL( false  , metric2->equal( *metric1 ));

}




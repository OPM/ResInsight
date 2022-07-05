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

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#include <boost/test/unit_test.hpp>

#include <memory>
#include <ostream>

using namespace Opm;

BOOST_AUTO_TEST_CASE(DefDim) {
    Dimension dim;
    BOOST_CHECK_EQUAL(1.0, dim.getSIScaling());
    BOOST_CHECK( dim.isCompositable() );
}

BOOST_AUTO_TEST_CASE(CreateDimension) {
    Dimension length(1);
    BOOST_CHECK_EQUAL(1 , length.getSIScaling());
}


BOOST_AUTO_TEST_CASE(CreateUnitSystem) {
    UnitSystem system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    BOOST_CHECK_EQUAL("Metric" , system.getName());
}



BOOST_AUTO_TEST_CASE(UnitSystemGetMissingDimensionThrows) {
    UnitSystem system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    BOOST_CHECK_THROW( system.getDimension("Missing") , std::out_of_range );
}

BOOST_AUTO_TEST_CASE(UnitSystemGetNewOK) {
    UnitSystem system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    system.addDimension("Length" , 10 );
    system.addDimension("Time" , 100);

    BOOST_CHECK( !system.hasDimension("Length*Length/Time"));
    Dimension comp = system.getNewDimension( "Length*Length/Time" );
    BOOST_CHECK( system.hasDimension("Length*Length/Time"));
    BOOST_CHECK_EQUAL(1 , comp.getSIScaling());
}


BOOST_AUTO_TEST_CASE(UnitSystemAddDimensions) {
    UnitSystem system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    system.addDimension("Length" , 1 );
    system.addDimension("Time" , 86400 );
    system.addDimension("Temperature", 1.0, 273.15);

    auto length = system.getDimension( "Length" );
    auto time = system.getDimension( "Time" );
    auto temperature = system.getDimension( "Temperature" );
    BOOST_CHECK_EQUAL(1     , length.getSIScaling());
    BOOST_CHECK_EQUAL(86400 , time.getSIScaling());
    BOOST_CHECK_EQUAL(1.0   , temperature.getSIScaling());
    BOOST_CHECK_EQUAL(273.15, temperature.getSIOffset());

    system.addDimension("Length" , 0.3048);
    length = system.getDimension("Length");
    BOOST_CHECK_EQUAL(0.3048 , length.getSIScaling());
}


BOOST_AUTO_TEST_CASE(UnitSystemParseInvalidThrows) {
    UnitSystem system(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    BOOST_CHECK_THROW( system.parse("//") , std::invalid_argument);
    BOOST_CHECK_THROW( system.parse("Length * Length / Time") , std::out_of_range );

    system.addDimension("Length" , 3.00 );
    system.addDimension("Time" , 9.0 );

    auto volumePerTime = system.parse( "Length*Length*Length/Time" );
    BOOST_CHECK_EQUAL(3.0 , volumePerTime.getSIScaling());
}



static void checkSystemHasRequiredDimensions( const UnitSystem& system) {
    BOOST_CHECK( system.hasDimension("1"));
    BOOST_CHECK( system.hasDimension("Length"));
    BOOST_CHECK( system.hasDimension("Mass"));
    BOOST_CHECK( system.hasDimension("Time"));

    BOOST_CHECK( system.hasDimension("Permeability"));
    BOOST_CHECK( system.hasDimension("Pressure"));
    BOOST_CHECK( system.hasDimension("Temperature"));
}



BOOST_AUTO_TEST_CASE(CreateMetricSystem) {
    auto system = UnitSystem::newMETRIC();
    checkSystemHasRequiredDimensions( system );

    BOOST_CHECK_EQUAL( Metric::Length       , system.getDimension("Length").getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Mass         , system.getDimension("Mass").getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Time         , system.getDimension("Time").getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Permeability , system.getDimension("Permeability").getSIScaling() );
    BOOST_CHECK_EQUAL( Metric::Pressure     , system.getDimension("Pressure").getSIScaling() );
}



BOOST_AUTO_TEST_CASE(CreateFieldSystem) {
    auto system = UnitSystem::newFIELD();
    checkSystemHasRequiredDimensions( system );

    BOOST_CHECK_EQUAL( Field::Length       , system.getDimension("Length").getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Mass         , system.getDimension("Mass").getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Time         , system.getDimension("Time").getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Permeability , system.getDimension("Permeability").getSIScaling() );
    BOOST_CHECK_EQUAL( Field::Pressure     , system.getDimension("Pressure").getSIScaling() );
}




BOOST_AUTO_TEST_CASE(CreateInputSystem) {
    auto system = UnitSystem::newINPUT();
    checkSystemHasRequiredDimensions( system );

    BOOST_CHECK_EQUAL( 1.0, system.getDimension("Length").getSIScaling() );
    BOOST_CHECK_EQUAL( 1.0, system.getDimension("Mass").getSIScaling() );
    BOOST_CHECK_EQUAL( 1.0, system.getDimension("Time").getSIScaling() );
    BOOST_CHECK_EQUAL( 1.0, system.getDimension("Permeability").getSIScaling() );
    BOOST_CHECK_EQUAL( 1.0, system.getDimension("Pressure").getSIScaling() );

    BOOST_CHECK_EQUAL( static_cast<long int>(system.getType( )) , static_cast<long int>(UnitSystem::UnitType::UNIT_TYPE_INPUT) );
}




BOOST_AUTO_TEST_CASE(DimensionEqual) {
    Dimension d1(1);
    Dimension d2(1);
    Dimension d4(2);

    BOOST_CHECK_EQUAL( true  , d1.equal(d1) );
    BOOST_CHECK_EQUAL( true  , d1.equal(d2) );
    BOOST_CHECK_EQUAL( false , d1.equal(d4) );
}

namespace Opm {
inline std::ostream& operator<<( std::ostream& stream, const UnitSystem& us ) {
    return stream << us.getName() << " :: " << static_cast<int>(us.getType());
}
}


BOOST_AUTO_TEST_CASE(UnitSystemEqual) {
    auto metric1 = UnitSystem::newMETRIC();
    auto metric2 = UnitSystem::newMETRIC();
    auto field = UnitSystem::newFIELD();

    BOOST_CHECK_EQUAL( metric1, metric1 );
    BOOST_CHECK_EQUAL( metric1, metric2 );
    BOOST_CHECK_NE( metric1, field );
    metric1.addDimension("g" , 3.00 );
    BOOST_CHECK_NE( metric1, metric2 );
    BOOST_CHECK_NE( metric2, metric1 );
}



BOOST_AUTO_TEST_CASE(LabUnitConversions) {
    using Meas = UnitSystem::measure;

    auto lab = UnitSystem::newLAB();

    {
        const auto furlong = 660*unit::feet;
        BOOST_CHECK_CLOSE( 2.01168e4 , lab.from_si( Meas::length , furlong ) , 1.0e-10 );
        BOOST_CHECK_CLOSE( furlong   , lab.to_si( Meas::length , 2.01168e4 ) , 1.0e-10 );
    }

    struct Factor { Meas m; double f; };

    for (const auto& q : { Factor{ Meas::density               , 1.0e3  }   ,
                           Factor{ Meas::pressure              , 101325.0 } ,
                           Factor{ Meas::viscosity             , 1.0e-3 }   ,
                           Factor{ Meas::liquid_surface_volume , 1.0e-6 }   ,
                           Factor{ Meas::gas_surface_volume    , 1.0e-6 }   ,
                           Factor{ Meas::time                  , 3600.0 }   ,
                           Factor{ Meas::mass                  , 1.0e-3 }   })
    {
        BOOST_CHECK_CLOSE( q.f , lab.to_si( q.m , 1.0 )   , 1.0e-10 );
        BOOST_CHECK_CLOSE( 1.0 , lab.from_si( q.m , q.f ) , 1.0e-10 );
    }
}


BOOST_AUTO_TEST_CASE( VectorConvert ) {
    std::vector<double> d0 = {1,2,3};
    std::vector<double> d1 = {1,2,3};
    UnitSystem units = UnitSystem::newLAB();

    units.from_si( UnitSystem::measure::pressure , d0 );
    for (size_t i = 0; i < d1.size(); i++)
        BOOST_CHECK_EQUAL( units.from_si( UnitSystem::measure::pressure , d1[i] ) , d0[i]);
}

BOOST_AUTO_TEST_CASE( GasOilRatioNotIdentityForField ) {
    const double gas = 14233.4;
    const double oil = 4223;

    const double ratio = gas / oil;
    const auto units = UnitSystem::newFIELD();

    const auto field_gas = (gas * 35.314666721) / 1000;
    const auto field_oil = oil * 6.28981100;

    const auto gor = UnitSystem::measure::gas_oil_ratio;

    BOOST_CHECK_CLOSE( units.from_si( gor, ratio ), field_gas / field_oil, 1e-5 );
}

BOOST_AUTO_TEST_CASE ( UnitConstants ) {
    using namespace Opm::prefix;
    using namespace Opm::unit;

    BOOST_REQUIRE_EQUAL (meter, 1);
    BOOST_REQUIRE_EQUAL (kilogram, 1);
    BOOST_REQUIRE_EQUAL (second, 1);

    BOOST_REQUIRE_CLOSE (milli*darcy, 9.86923667e-16, 0.01);
    BOOST_REQUIRE_CLOSE (mega*darcy, 9.86923e-7, 0.01);
    BOOST_REQUIRE_CLOSE (convert::to(mega*darcy, milli*darcy), 1e9, 0.01);

    BOOST_REQUIRE_CLOSE (convert::to(convert::from(1.0, barsa), psia), 14.5038, 0.01);
    BOOST_REQUIRE_CLOSE (convert::to(1*atm, barsa), 1.01325, 0.01);

    const double flux_SI = 10000*cubic(meter)/year;
    BOOST_REQUIRE_CLOSE (flux_SI, 3.17098e-4, 0.01);
    const double flux_m3py = convert::to(flux_SI, cubic(meter)/year);
    BOOST_REQUIRE_CLOSE (flux_m3py, 1e4, 0.01);
}

BOOST_AUTO_TEST_CASE(METRIC_UNITS)
{
    using Meas = UnitSystem::measure;

    auto metric = UnitSystem::newMETRIC();

    BOOST_CHECK( metric.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC );

    // ----------------------------------------------------------------
    // METRIC -> SI

    BOOST_CHECK_CLOSE( metric.to_si( Meas::length , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::time , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::density , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::pressure , 1.0 ) , 100.0e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::temperature_absolute , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::temperature , 1.0 ) , 274.15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::viscosity , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::permeability , 1.0 ) , 9.869232667160129e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::liquid_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::gas_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::geometric_volume, 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::liquid_surface_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::gas_surface_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::geometric_volume_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::transmissibility , 1.0 ) , 1.157407407407407e-13 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::effective_Kh , 1.0 ) , 9.869232667160129e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::mass , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::mass_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::gas_oil_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::oil_gas_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::gas_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::liquid_productivity_index , 1.0 ) , 1.1574074074074073e-10 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::gas_productivity_index , 1.0 ) , 1.1574074074074073e-10 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.to_si( Meas::energy, 1.0), 1000, 1e-10);
    BOOST_CHECK_CLOSE( metric.to_si( Meas::icd_strength, 1.0), 7.46496e+14, 1e-10);

    // ----------------------------------------------------------------
    // SI -> METRIC

    BOOST_CHECK_CLOSE( metric.from_si( Meas::length , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::time , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::density , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::pressure , 1.0 ) , 1.0e-5 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::temperature_absolute , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::temperature , 274.15 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::viscosity , 1.0 ) , 1.0e+3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::permeability , 1.0 ) , 1.01325e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::liquid_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::gas_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::geometric_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::liquid_surface_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::gas_surface_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::geometric_volume_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::transmissibility , 1.0 ) , 8.64e+12 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::effective_Kh , 1.0 ) , 1.01325e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::mass , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::mass_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::gas_oil_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::oil_gas_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::gas_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::liquid_productivity_index , 1.0 ) , 86.400e8 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::gas_productivity_index , 1.0 ) , 86.400e8 , 1.0e-10 );
    BOOST_CHECK_CLOSE( metric.from_si( Meas::energy, 1000.0), 1, 1e-10);
    BOOST_CHECK_CLOSE( metric.from_si( Meas::icd_strength, 7.46496e+14), 1.0, 1e-10);
}

BOOST_AUTO_TEST_CASE(FIELD_UNITS)
{
    using Meas = UnitSystem::measure;

    auto field = UnitSystem::newFIELD();

    BOOST_CHECK( field.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD );

    // ----------------------------------------------------------------
    // FIELD -> SI

    BOOST_CHECK_CLOSE( field.to_si( Meas::length , 1.0 ) , 0.3048 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::time , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::density , 1.0 ) , 1.601846337396014e+01, 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::pressure , 1.0 ) , 6.894757293168360e+03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::temperature_absolute , 1.0 ) , 5.0/9.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::temperature , 1.0 ) , 255.9277777777778 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::viscosity , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::permeability , 1.0 ) , 9.869232667160129e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::liquid_surface_volume , 1.0 ) , 0.1589872949280001 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::gas_surface_volume , 1.0 ) , 28.31684659200000 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::volume , 1.0 ) , 0.1589872949280001 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::geometric_volume, 1.0 ) , 2.8316846592e-02 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::liquid_surface_rate , 1.0 ) , 1.840130728333334e-06 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::gas_surface_rate , 1.0 ) , 3.277412800000001e-04 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::rate , 1.0 ) , 1.840130728333334e-06 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::geometric_volume_rate, 1.0 ) , 3.2774128e-07 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::transmissibility , 1.0 ) , 2.668883979653090e-13 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::effective_Kh , 1.0 ) , 3.008142116950407e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::mass , 1.0 ) , 0.45359237 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::mass_rate , 1.0 ) , 5.249911689814815e-06 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::gas_oil_ratio , 1.0 ) , 178.1076066790352 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::oil_gas_ratio , 1.0 ) , 5.614583333333335e-03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::gas_formation_volume_factor , 1.0 ) , 5.614583333333335e-03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 178.1076066790352 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::liquid_productivity_index , 1.0 ) , 1.840130728333334e-06 / 6.894757293168360e+03  , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::gas_productivity_index , 1.0 ) , 3.277412800000001e-04 / 6.894757293168360e+03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::energy , 1.0 ) , 1054.3503 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.to_si( Meas::icd_strength , 1.0 ) , 6.418842091749854e+16 , 1.0e-10 );

    // ----------------------------------------------------------------
    // SI -> FIELD

    BOOST_CHECK_CLOSE( field.from_si( Meas::length , 1.0 ) , 3.280839895013123e+00 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::time , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::density , 1.0 ) , 6.242796057614462e-02 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::pressure , 1.0 ) , 1.450377377302092e-04 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::temperature_absolute , 1.0 ) , 1.8 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::temperature , 255.9277777777778 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::viscosity , 1.0 ) , 1.0e+3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::permeability , 1.0 ) , 1.01325e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::liquid_surface_volume , 1.0 ) , 6.289810770432102e+00 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::gas_surface_volume , 1.0 ) , 3.531466672148859e-02 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::volume , 1.0 ) , 6.289810770432102e+00 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::geometric_volume, 2.8316846592e-02 ), 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::liquid_surface_rate , 1.0 ) , 5.434396505653337e+05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::gas_surface_rate , 1.0 ) , 3.051187204736614e+03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::rate , 1.0 ) , 5.434396505653337e+05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::geometric_volume_rate, 3.2774128e-07 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::transmissibility , 1.0 ) , 3.746884494132199e+12 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::effective_Kh , 1.0 ) , 3.324311023622047e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::mass , 1.0 ) , 2.204622621848776e+00 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::gas_oil_ratio , 1.0 ) , 5.614583333333335e-03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::oil_gas_ratio , 1.0 ) , 178.1076066790352 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::gas_formation_volume_factor , 1.0 ) , 178.1076066790352 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 5.614583333333335e-03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::liquid_productivity_index , 1.0 ) , 5.434396505653337e+05 * 6.894757293168360e+03 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::gas_productivity_index , 1.0 ) , 3.051187204736614e+03 * 6.894757293168360e+03, 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::energy , 1054.3503 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( field.from_si( Meas::icd_strength , 6.418842091749854e+16 ) , 1.0 , 1.0e-10 );
}

BOOST_AUTO_TEST_CASE(LAB_UNITS)
{
    using Meas = UnitSystem::measure;

    auto lab = UnitSystem::newLAB();

    BOOST_CHECK( lab.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_LAB );

    // ----------------------------------------------------------------
    // LAB -> SI

    BOOST_CHECK_CLOSE( lab.to_si( Meas::length , 1.0 ) , 0.01 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::time , 1.0 ) , 3600.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::density , 1.0 ) , 1.0e3, 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::pressure , 1.0 ) , 101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::temperature_absolute , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::temperature , 1.0 ) , 274.15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::viscosity , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::permeability , 1.0 ) , 9.869232667160129e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::liquid_surface_volume , 1.0 ) , 1.0e-6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::gas_surface_volume , 1.0 ) , 1.0e-6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::volume , 1.0 ) , 1.0e-6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::geometric_volume, 1.0 ) , 1.0e-6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::liquid_surface_rate , 1.0 ) , 2.777777777777778e-10 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::gas_surface_rate , 1.0 ) , 2.777777777777778e-10 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::rate , 1.0 ) , 2.777777777777778e-10 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::geometric_volume_rate, 1.0 ) , 2.777777777777778e-10 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::transmissibility , 1.0 ) , 2.741453518655592e-18 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::effective_Kh , 1.0 ) , 9.869232667160130e-18 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::mass , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::mass_rate , 1.0 ) , 2.777777777777778e-07 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::gas_oil_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::oil_gas_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::gas_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::liquid_productivity_index , 1.0 ) , 2.777777777777778e-10 / 101325.0  , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::gas_productivity_index , 1.0 ) , 2.777777777777778e-10 / 101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::energy , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.to_si( Meas::icd_strength , 1.0 ) , 1.313172e+24 , 1.0e-10 );

    // ----------------------------------------------------------------
    // SI -> LAB

    BOOST_CHECK_CLOSE( lab.from_si( Meas::length , 1.0 ) , 100.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::time , 1.0 ) , 2.777777777777778e-04 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::density , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::pressure , 1.0 ) , 9.869232667160129e-06 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::temperature_absolute , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::temperature , 274.15 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::viscosity , 1.0 ) , 1.0e+3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::permeability , 1.0 ) , 1.01325e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::liquid_surface_volume , 1.0 ) , 1.0e6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::gas_surface_volume , 1.0 ) , 1.0e6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::volume , 1.0 ) , 1.0e6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::geometric_volume , 1.0 ) , 1.0e6 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::liquid_surface_rate , 1.0 ) , 3.6e9 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::gas_surface_rate , 1.0 ) , 3.6e9 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::rate , 1.0 ) , 3.6e9 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::geometric_volume_rate, 1.0 ) , 3.6e9 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::transmissibility , 1.0 ) , 3.647699999999999e+17 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::effective_Kh , 1.0 ) , 1.01325e+17 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::mass , 1.0 ) , 1.0e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::mass_rate , 1.0 ) , 3.6e+06 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::gas_oil_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::oil_gas_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::gas_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::liquid_productivity_index , 1.0 ) , 3.6e9 * 101325.0, 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::gas_productivity_index , 1.0 ) , 3.6e9 * 101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::energy , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( lab.from_si( Meas::icd_strength , 1.0 ) , 7.615148662932201e-25 , 1.0e-10 );
}

BOOST_AUTO_TEST_CASE(PVT_M_UNITS)
{
    using Meas = UnitSystem::measure;

    auto pvt_m = UnitSystem::newPVT_M();

    BOOST_CHECK( pvt_m.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_PVT_M );

    // ----------------------------------------------------------------
    // PVT-M -> SI

    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::length , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::time , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::density , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::pressure , 1.0 ) , 101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::temperature_absolute , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::temperature , 1.0 ) , 274.15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::viscosity , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::permeability , 1.0 ) , 9.869232667160129e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::liquid_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::gas_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::geometric_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::liquid_surface_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::gas_surface_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::geometric_volume_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::transmissibility , 1.0 ) , 1.142272299439830e-13 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::effective_Kh , 1.0 ) , 9.869232667160129e-16 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::mass , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::mass_rate , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::gas_oil_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::oil_gas_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::gas_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::liquid_productivity_index , 1.0 ) , 1.1574074074074073e-05 /101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::gas_productivity_index , 1.0 ) , 1.1574074074074073e-05 /101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::energy , 1.0 ) , 1.0e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.to_si( Meas::icd_strength , 1.0 ) , 7.56387072e+14 , 1.0e-10 );

    // ----------------------------------------------------------------
    // SI -> PVT-M

    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::length , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::time , 1.0 ) , 1.1574074074074073e-05 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::density , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::pressure , 1.0 ) , 9.869232667160129e-06 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::temperature_absolute , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::temperature , 274.15 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::viscosity , 1.0 ) , 1.0e+3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::permeability , 1.0 ) , 1.01325e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::liquid_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::gas_surface_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::geometric_volume , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::liquid_surface_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::gas_surface_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::geometric_volume_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::transmissibility , 1.0 ) , 8.75448e+12 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::effective_Kh , 1.0 ) , 1.01325e+15 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::mass , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::mass_rate , 1.0 ) , 86.400e3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::gas_oil_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::oil_gas_ratio , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::water_cut , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::gas_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::oil_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::water_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::gas_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::oil_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::water_inverse_formation_volume_factor , 1.0 ) , 1.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::liquid_productivity_index , 1.0 ) , 86.400e3 * 101325.0 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::gas_productivity_index , 1.0 ) , 86.400e3 * 101325.0, 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::energy , 1.0 ) , 1.0e-3 , 1.0e-10 );
    BOOST_CHECK_CLOSE( pvt_m.from_si( Meas::icd_strength , 1.0 ) , 1.322074420647951e-15 , 1.0e-10 );
}

BOOST_AUTO_TEST_CASE(TemperatureConversions)
{
    using Meas = UnitSystem::measure;

    // note that "metric" units are not SI units!
    auto metric = UnitSystem::newMETRIC();
    auto pvt_m = UnitSystem::newPVT_M();
    auto lab = UnitSystem::newLAB();
    auto field = UnitSystem::newFIELD();

    // check absolute temperature, i.e. °R for field, K for the rest
    BOOST_CHECK_CLOSE(metric.to_si(Meas::temperature_absolute , 1.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(metric.from_si(Meas::temperature_absolute , 1.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(pvt_m.to_si(Meas::temperature_absolute , 1.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(pvt_m.from_si(Meas::temperature_absolute , 1.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(lab.to_si(Meas::temperature_absolute , 1.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(lab.from_si(Meas::temperature_absolute , 1.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(field.to_si(Meas::temperature_absolute , 9.0/5.0), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(field.from_si(Meas::temperature_absolute , 1.0), 9.0/5.0, 1.0e-10);

    // check temperature, i.e. °F for field, °C for the rest
    BOOST_CHECK_CLOSE(metric.to_si(Meas::temperature , 1.0), 274.15, 1.0e-10);
    BOOST_CHECK_CLOSE(metric.from_si(Meas::temperature , 274.15), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(pvt_m.to_si(Meas::temperature , 1.0), 274.15, 1.0e-10);
    BOOST_CHECK_CLOSE(pvt_m.from_si(Meas::temperature , 274.15), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(lab.to_si(Meas::temperature , 1.0), 274.15, 1.0e-10);
    BOOST_CHECK_CLOSE(lab.from_si(Meas::temperature , 274.15), 1.0, 1.0e-10);
    BOOST_CHECK_CLOSE(field.to_si(Meas::temperature , 1.0), (459.67 + 1.0)*5.0/9.0, 1.0e-10);
    BOOST_CHECK_CLOSE(field.from_si(Meas::temperature , (459.67 + 1.0)*5.0/9.0), 1.0, 1.0e-10);
}



BOOST_AUTO_TEST_CASE(EclipseID) {
    BOOST_CHECK_THROW(UnitSystem(0), std::invalid_argument);
    BOOST_CHECK_THROW(UnitSystem(5), std::invalid_argument);

    UnitSystem metric1(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    UnitSystem metric2(metric1.ecl_id());
    BOOST_CHECK( metric1 == metric2 );

    UnitSystem field1(UnitSystem::UnitType::UNIT_TYPE_FIELD);
    UnitSystem field2(field1.ecl_id());
    BOOST_CHECK( field1 == field2 );
}


BOOST_AUTO_TEST_CASE(DECK_NAMES) {
    BOOST_CHECK( !UnitSystem::valid_name("INVALID"));
    BOOST_CHECK( UnitSystem::valid_name("FIELD"));
    BOOST_CHECK( UnitSystem::valid_name("METRIC"));
    BOOST_CHECK( UnitSystem::valid_name("LAB"));
    BOOST_CHECK( UnitSystem::valid_name("PVT-M"));


    UnitSystem us("METRIC");
    BOOST_CHECK_EQUAL( us.deck_name(), "METRIC");
}

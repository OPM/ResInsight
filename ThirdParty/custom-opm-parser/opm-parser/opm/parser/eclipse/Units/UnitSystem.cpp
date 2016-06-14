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


#include <iostream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/ConversionFactors.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <vector>
#include <limits>


namespace Opm {

namespace {
    /*
     * It is VERY important that the measure enum has the same order as the
     * metric and field arrays. C++ does not support designated initializers, so
     * this cannot be done in a declaration-order independent matter.
     */

    static const double to_metric[] = {
        1 / Metric::Length,
        1 / Metric::Time,
        1 / Metric::Density,
        1 / Metric::Pressure,
        1 / Metric::AbsoluteTemperature,
        1 / Metric::Temperature,
        1 / Metric::Viscosity,
        1 / Metric::Permeability,
        1 / Metric::LiquidSurfaceVolume,
        1 / Metric::GasSurfaceVolume,
        1 / Metric::ReservoirVolume,
        1 / ( Metric::LiquidSurfaceVolume / Metric::Time ),
        1 / ( Metric::GasSurfaceVolume / Metric::Time ),
        1 / ( Metric::ReservoirVolume / Metric::Time ),
        1 / Metric::Transmissibility,
        1 / Metric::Mass,
    };

    static const double from_metric[] = {
        Metric::Length,
        Metric::Time,
        Metric::Density,
        Metric::Pressure,
        Metric::AbsoluteTemperature,
        Metric::Temperature,
        Metric::Viscosity,
        Metric::Permeability,
        Metric::LiquidSurfaceVolume,
        Metric::GasSurfaceVolume,
        Metric::ReservoirVolume,
        Metric::LiquidSurfaceVolume / Metric::Time,
        Metric::GasSurfaceVolume / Metric::Time,
        Metric::ReservoirVolume / Metric::Time,
        Metric::Transmissibility,
        Metric::Mass,
    };

    static const double to_field[] = {
        1 / Field::Length,
        1 / Field::Time,
        1 / Field::Density,
        1 / Field::Pressure,
        1 / Field::AbsoluteTemperature,
        1 / Field::Temperature,
        1 / Field::Viscosity,
        1 / Field::Permeability,
        1 / Field::LiquidSurfaceVolume,
        1 / Field::GasSurfaceVolume,
        1 / Field::ReservoirVolume,
        1 / ( Field::LiquidSurfaceVolume / Field::Time ),
        1 / ( Field::GasSurfaceVolume / Field::Time ),
        1 / ( Field::ReservoirVolume / Field::Time ),
        1 / Field::Transmissibility,
        1 / Field::Mass,
    };

    static const double from_field[] = {
         Field::Length,
         Field::Time,
         Field::Density,
         Field::Pressure,
         Field::AbsoluteTemperature,
         Field::Temperature,
         Field::Viscosity,
         Field::Permeability,
         Field::LiquidSurfaceVolume,
         Field::GasSurfaceVolume,
         Field::ReservoirVolume,
         Field::LiquidSurfaceVolume / Field::Time,
         Field::GasSurfaceVolume / Field::Time,
         Field::ReservoirVolume / Field::Time,
         Field::Transmissibility,
         Field::Mass,
    };

}

    UnitSystem::UnitSystem(const UnitType unit) :
        m_unittype( unit )
    {
        switch(unit) {
            case(UNIT_TYPE_METRIC):
                m_name = "Metric";
                this->measure_table_from_si = to_metric;
                this->measure_table_to_si = from_metric;
                break;
            case(UNIT_TYPE_FIELD):
                m_name = "Field";
                this->measure_table_from_si = to_field;
                this->measure_table_to_si = from_field;
                break;
            case(UNIT_TYPE_LAB):
                m_name = "Lab";
                throw std::runtime_error( "Lab unit system is not supported" );
                break;
            default:
                //do nothing
                break;
        };
    }

    bool UnitSystem::hasDimension(const std::string& dimension) const {
        return (m_dimensions.find( dimension ) != m_dimensions.end());
    }


    std::shared_ptr<const Dimension> UnitSystem::getNewDimension(const std::string& dimension) {
        if (!hasDimension( dimension )) {
            std::shared_ptr<const Dimension> newDimension = parse( dimension );
            addDimension( newDimension );
        }
        return getDimension( dimension );
    }


    std::shared_ptr<const Dimension> UnitSystem::getDimension(const std::string& dimension) const {
        if (hasDimension( dimension ))
            return m_dimensions.at( dimension );
        else
            throw std::invalid_argument("Dimension: " + dimension + " not recognized ");
    }


    void UnitSystem::addDimension(std::shared_ptr<const Dimension> dimension) {
        if (hasDimension(dimension->getName()))
            m_dimensions.erase( dimension->getName() );

        m_dimensions.insert( std::make_pair(dimension->getName() , dimension));
    }

    void UnitSystem::addDimension(const std::string& dimension , double SIfactor, double SIoffset) {
        std::shared_ptr<const Dimension> dim( new Dimension(dimension , SIfactor, SIoffset) );
        addDimension(dim);
    }

    const std::string& UnitSystem::getName() const {
        return m_name;
    }

    UnitSystem::UnitType UnitSystem::getType() const {
        return m_unittype;
    }


    std::shared_ptr<const Dimension> UnitSystem::parseFactor(const std::string& dimension) const {
        std::vector<std::string> dimensionList;
        boost::split(dimensionList , dimension , boost::is_any_of("*"));
        double SIfactor = 1.0;
        for (auto iter = dimensionList.begin(); iter != dimensionList.end(); ++iter) {
            std::shared_ptr<const Dimension> dim = getDimension( *iter );

            // all constituing dimension must be compositable. The
            // only exception is if there is the "composite" dimension
            // consists of exactly a single atomic dimension...
            if (dimensionList.size() > 1 && !dim->isCompositable())
                throw std::invalid_argument("Composite dimensions currently cannot require a conversion offset");

            SIfactor *= dim->getSIScaling();
        }
        return std::shared_ptr<Dimension>(Dimension::newComposite( dimension , SIfactor ));
    }



    std::shared_ptr<const Dimension> UnitSystem::parse(const std::string& dimension) const {
        bool haveDivisor;
        {
            size_t divCount = std::count( dimension.begin() , dimension.end() , '/' );
            if (divCount == 0)
                haveDivisor = false;
            else if (divCount == 1)
                haveDivisor = true;
            else
                throw std::invalid_argument("Dimension string can only have one division sign /");
        }

        if (haveDivisor) {
            std::vector<std::string> parts;
            boost::split(parts , dimension , boost::is_any_of("/"));
            std::shared_ptr<const Dimension> dividend = parseFactor( parts[0] );
            std::shared_ptr<const Dimension> divisor = parseFactor( parts[1] );

            if (dividend->getSIOffset() != 0.0 || divisor->getSIOffset() != 0.0)
                throw std::invalid_argument("Composite dimensions cannot currently require a conversion offset");

            return std::shared_ptr<Dimension>( Dimension::newComposite( dimension , dividend->getSIScaling() / divisor->getSIScaling() ));
        } else {
            return parseFactor( dimension );
        }
    }


    bool UnitSystem::equal(const UnitSystem& other) const {
        bool equal_ = (m_dimensions.size() == other.m_dimensions.size());

        if (equal_) {
            for (auto iter = m_dimensions.begin(); iter != m_dimensions.end(); ++iter) {
                std::shared_ptr<const Dimension> dim = getDimension( iter->first );

                if (other.hasDimension( iter->first )) {
                    std::shared_ptr<const Dimension> otherDim = other.getDimension( iter->first );
                    if (!dim->equal(*otherDim))
                        equal_ = false;
                } else
                    equal_ = false;

            }
        }
        return equal_;
    }

    double UnitSystem::from_si( measure m, double val ) const {
        return this->measure_table_from_si[ static_cast< int >( m ) ] * val;
    }

    double UnitSystem::to_si( measure m, double val ) const {
        return this->measure_table_to_si[ static_cast< int >( m ) ] * val;
    }

    UnitSystem * UnitSystem::newMETRIC() {
        UnitSystem * system = new UnitSystem(UNIT_TYPE_METRIC);

        system->addDimension("1"         , 1.0);
        system->addDimension("Pressure"  , Metric::Pressure );
        system->addDimension("Temperature", Metric::Temperature, Metric::TemperatureOffset);
        system->addDimension("AbsoluteTemperature", Metric::AbsoluteTemperature);
        system->addDimension("Length"    , Metric::Length);
        system->addDimension("Time"      , Metric::Time );
        system->addDimension("Mass"         , Metric::Mass );
        system->addDimension("Permeability", Metric::Permeability );
        system->addDimension("Transmissibility", Metric::Transmissibility );
        system->addDimension("GasDissolutionFactor", Metric::GasDissolutionFactor);
        system->addDimension("OilDissolutionFactor", Metric::OilDissolutionFactor);
        system->addDimension("LiquidSurfaceVolume", Metric::LiquidSurfaceVolume );
        system->addDimension("GasSurfaceVolume" , Metric::GasSurfaceVolume );
        system->addDimension("ReservoirVolume", Metric::ReservoirVolume );
        system->addDimension("Density"   , Metric::Density );
        system->addDimension("PolymerDensity", Metric::PolymerDensity);
        system->addDimension("Salinity", Metric::Salinity);
        system->addDimension("Viscosity" , Metric::Viscosity);
        system->addDimension("Timestep"  , Metric::Timestep);
        system->addDimension("ContextDependent", std::numeric_limits<double>::quiet_NaN());
        return system;
    }



    UnitSystem * UnitSystem::newFIELD() {
        UnitSystem * system = new UnitSystem(UNIT_TYPE_FIELD);

        system->addDimension("1"    , 1.0);
        system->addDimension("Pressure", Field::Pressure );
        system->addDimension("Temperature", Field::Temperature, Field::TemperatureOffset);
        system->addDimension("AbsoluteTemperature", Field::AbsoluteTemperature);
        system->addDimension("Length", Field::Length);
        system->addDimension("Time" , Field::Time);
        system->addDimension("Mass", Field::Mass);
        system->addDimension("Permeability", Field::Permeability );
        system->addDimension("Transmissibility", Field::Transmissibility );
        system->addDimension("GasDissolutionFactor" , Field::GasDissolutionFactor);
        system->addDimension("OilDissolutionFactor", Field::OilDissolutionFactor);
        system->addDimension("LiquidSurfaceVolume", Field::LiquidSurfaceVolume );
        system->addDimension("GasSurfaceVolume", Field::GasSurfaceVolume );
        system->addDimension("ReservoirVolume", Field::ReservoirVolume );
        system->addDimension("Density", Field::Density );
        system->addDimension("PolymerDensity", Field::PolymerDensity);
        system->addDimension("Salinity", Field::Salinity);
        system->addDimension("Viscosity", Field::Viscosity);
        system->addDimension("Timestep", Field::Timestep);
        system->addDimension("ContextDependent", std::numeric_limits<double>::quiet_NaN());
        return system;
    }

}

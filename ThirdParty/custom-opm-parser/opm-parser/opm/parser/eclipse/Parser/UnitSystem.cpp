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

/*#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <vector>
#include <string>
*/

#include <iostream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>


#include <opm/parser/eclipse/Parser/UnitSystem.hpp>
#include <vector>


namespace Opm {

    UnitSystem::UnitSystem(const std::string& unitSystem) :
        m_name( unitSystem )
    {

    }

    bool UnitSystem::hasDimension(const std::string& dimension) const {
        return (m_dimensions.find( dimension ) != m_dimensions.end());
    }


    const Dimension& UnitSystem::getDimension(const std::string& dimension) const {
        if (hasDimension( dimension ))
            return m_dimensions.at( dimension );
        else
            throw std::invalid_argument("Dimension: " + dimension + " not recognized ");
    }


    void UnitSystem::addDimension(const std::string& dimension , double SI_factor) {
        if (hasDimension(dimension))
            m_dimensions.erase( dimension );

        m_dimensions.insert( std::make_pair(dimension , Dimension(dimension , SI_factor)));
    }


    const std::string& UnitSystem::getName() const {
        return m_name;
    }


    std::shared_ptr<const Dimension> UnitSystem::parseFactor(const std::string& dimension) const {
        std::vector<std::string> dimensionList;
        boost::split(dimensionList , dimension , boost::is_any_of("*"));
        double SIfactor = 1.0;
        for (auto iter = dimensionList.begin(); iter != dimensionList.end(); ++iter) {
            Dimension dim = getDimension( *iter );
            SIfactor *= dim.getSIScaling();
        }
        return Dimension::newComposite( dimension , SIfactor );
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
            Dimension dividend = parseFactor( parts[0] );
            Dimension divisor = parseFactor( parts[1] );

            return Dimension::newComposite( dimension , dividend.getSIScaling() / divisor.getSIScaling() );
        } else {
            return parseFactor( dimension );
        }
    }


    UnitSystem * UnitSystem::newMETRIC() {
        UnitSystem * system = new UnitSystem("Metric");

        system->addDimension("1" , 1);
        system->addDimension("Length" , 1);
        system->addDimension("Time" , 86400 );
        system->addDimension("Mass" , 1 );
        system->addDimension("Pressure" , 100000 );
        system->addDimension("Permeability" , 9.869233e-10 );
        system->addDimension("Viscosity" , 0.001); // viscosity. ECLiPSE uses cP for metric units
        system->addDimension("GasDissolutionFactor" , 1); // Gas dissolution factor. ECLiPSE uses m^3/m^3 for metric units

        return system;
    }



    UnitSystem * UnitSystem::newFIELD() {
        UnitSystem * system = new UnitSystem("Field");

        system->addDimension("1" , 1);
        system->addDimension("Length" , 0.3048);
        system->addDimension("Time" , 86400 );
        system->addDimension("Mass" , 0.45359237 );
        system->addDimension("Pressure" , 6894.76 );
        system->addDimension("Permeability" , 9.869233e-10 );
        system->addDimension("Viscosity" , 0.001); // viscosity. ECLiPSE uses cP for field units
        system->addDimension("GasDissolutionFactor" , 28.316847/0.15898729); // Gas dissolution factor. ECLiPSE uses Mscft/stb for field units

        return system;
    }

}




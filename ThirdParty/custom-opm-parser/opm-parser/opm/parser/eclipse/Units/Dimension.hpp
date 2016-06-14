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

#ifndef DIMENSION_H
#define DIMENSION_H

#include <string>

namespace Opm {

    class Dimension {
    public:
        Dimension(const std::string& name, double SIfactor, double SIoffset = 0.0);

        double getSIScaling() const;
        double getSIOffset() const;

        double convertRawToSi(double rawValue) const;
        double convertSiToRaw(double siValue) const;

        bool equal(const Dimension& other) const;
        const std::string& getName() const;
        bool isCompositable() const;
        static Dimension * newComposite(const std::string& dim, double SIfactor, double SIoffset = 0.0);

    private:
        Dimension();
        std::string m_name;
        double m_SIfactor;
        double m_SIoffset;
    };
}


#endif


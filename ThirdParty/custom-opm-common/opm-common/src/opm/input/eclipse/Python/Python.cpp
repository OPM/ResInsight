/*
  Copyright 2019 Equinor ASA.

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

#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include "PythonInterp.hpp"

namespace Opm {

Python::Python(Enable enable) :
    interp( std::make_shared<PythonInterp>(false))
{
    if (enable == Enable::OFF)
        return;

    if (enable == Enable::ON)
        this->interp = std::make_shared<PythonInterp>(true);
    else {
        try {
            this->interp = std::make_shared<PythonInterp>(true);
        } catch(const std::logic_error &exc) {}
    }
}

bool Python::supported() {
    return PythonInterp::supported();
}


bool Python::exec(const std::string& python_code) const {
    return this->interp->exec(python_code);
}


bool Python::exec(const std::string& python_code, const Parser& parser, Deck& deck) const {
    return this->interp->exec(python_code, parser, deck);
}

bool Python::enabled() const {
    return bool( *this->interp );
}

}

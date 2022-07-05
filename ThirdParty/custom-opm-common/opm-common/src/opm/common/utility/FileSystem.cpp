/*
  Copyright 2019 Equinor ASA

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

#include <opm/common/utility/FileSystem.hpp>

#include <algorithm>
#include <random>

namespace Opm
{

std::string unique_path(const std::string& input)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    auto randchar = [&gen]()
    {
        const std::string set = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::uniform_int_distribution<> select(0, set.size()-1);
        return set[select(gen)];
    };

    std::string ret;
    ret.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(ret),
                   [&randchar](const char c)
                   {
                       return (c == '%') ? randchar() : c;
                   });

    return ret;
}

}

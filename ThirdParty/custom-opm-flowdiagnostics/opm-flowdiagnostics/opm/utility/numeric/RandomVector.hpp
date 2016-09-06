/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#ifndef OPM_RANDOMVECTOR_HEADER_INCLUDED
#define OPM_RANDOMVECTOR_HEADER_INCLUDED

#include <memory>
#include <vector>

namespace Opm
{
    class RandomVector
    {
    public:
        using Sample = std::vector<double>;
        using Size   = Sample::size_type;

        RandomVector();
        ~RandomVector();

        RandomVector(const RandomVector& rhs) = delete;
        RandomVector(RandomVector&&      rhs) = delete;

        RandomVector& operator=(const RandomVector& rhs) = delete;
        RandomVector& operator=(RandomVector&&      rhs) = delete;

        Sample normal(const Size   n,
                      const double mean  = 0.0,
                      const double stdev = 1.0);

        std::vector<int> index(const Size n,
                               const int  maxIdx);

    private:
        class Impl;

        std::unique_ptr<Impl> pImpl_;
    };

} // namespace Opm

#endif // OPM_RANDOMVECTOR_HEADER_INCLUDED

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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/utility/numeric/RandomVector.hpp>

#include <random>

class Opm::RandomVector::Impl
{
public:
    Sample normal(const Size   n,
                  const double mean,
                  const double stdev);

    std::vector<int> integer(const Size n,
                             const int  min,
                             const int  max);

private:
    using BitGenerator = std::mt19937;

    BitGenerator gen_;
};

Opm::RandomVector::Sample
Opm::RandomVector::
Impl::normal(const Size n, const double mean, const double stdev)
{
    auto distr = std::normal_distribution<>{ mean, stdev };

    auto sample = Sample{};
    sample.reserve(n);

    for (auto i = 0*n; i < n; ++i) {
        sample.push_back(distr(gen_));
    }

    return sample;
}

std::vector<int>
Opm::RandomVector::
Impl::integer(const Size n, const int min, const int max
)
{
    auto distr = std::uniform_int_distribution<>{ min, max };

    auto idx = std::vector<int>{};
    idx.reserve(n);

    for (auto i = 0*n; i < n; ++i) {
        idx.push_back(distr(gen_));
    }

    return idx;
}

// =====================================================================

Opm::RandomVector::RandomVector()
    : pImpl_(new Impl())
{}

Opm::RandomVector::~RandomVector()
{}

Opm::RandomVector::Sample
Opm::RandomVector::normal(const Size   n,
                          const double mean,
                          const double stdev)
{
    return pImpl_->normal(n, mean, stdev);
}

std::vector<int>
Opm::RandomVector::index(const Size n, const int maxIdx)
{
    return pImpl_->integer(n, 0, maxIdx);
}

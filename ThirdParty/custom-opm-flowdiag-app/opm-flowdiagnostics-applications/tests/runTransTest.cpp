/*
  Copyright 2017 SINTEF ICT, Applied Mathematics.
  Copyright 2017 Statoil ASA.

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

#include <opm/utility/ECLGraph.hpp>

#include <examples/exampleSetup.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace {
    class VectorDifference
    {
    public:
        using Vector    = std::vector<double>;
        using size_type = Vector::size_type;

        VectorDifference(const Vector& x, const Vector& y)
            : x_(x), y_(y)
        {
            if (x_.size() != y_.size()) {
                std::ostringstream os;

                os << "Incompatible Array Sizes: Expected 2x"
                   << x_.size() << ", but got ("
                   << x_.size() << ", " << y_.size() << ')';

                throw std::domain_error(os.str());
            }
        }

        size_type size() const
        {
            return x_.size();
        }

        bool empty() const
        {
            return this->size() == 0;
        }

        double operator[](const size_type i) const
        {
            return x_[i] - y_[i];
        }

    private:
        const Vector& x_;
        const Vector& y_;
    };

    template <class Vector1, class Vector2>
    class VectorRatio
    {
    public:
        using size_type = typename std::decay<
            decltype(std::declval<Vector1>()[0])
        >::type;

        VectorRatio(const Vector1& x, const Vector2& y)
            : x_(x), y_(y)
        {
            if (x_.size() != y.size()) {
                std::ostringstream os;

                os << "Incompatible Array Sizes: Expected 2x"
                   << x_.size() << ", but got ("
                   << x_.size() << ", " << y_.size() << ')';

                throw std::domain_error(os.str());
            }
        }

        size_type size() const
        {
            return x_.size();
        }

        bool empty() const
        {
            return x_.empty();
        }

        double operator[](const size_type i) const
        {
            return x_[i] / y_[i];
        }

    private:
        const Vector1& x_;
        const Vector2& y_;
    };

    struct ErrorTolerance
    {
        double absolute;
        double relative;
    };

    template <class FieldVariable>
    double pointMetric(const FieldVariable& x)
    {
        static_assert(std::is_same<typename std::decay<decltype(std::abs(x[0]))>::type, double>::value,
                      "Field Variable Value Type Must be 'double'");

        if (x.empty()) {
            return 0;
        }

        auto max = 0*x[0] - 1;

        for (decltype(x.size())
                 i = 0, n = x.size(); i < n; ++i)
        {
            const auto t = std::abs(x[i]);

            if (t > max) {
                max = t;
            }
        }

        return max;
    }

    ErrorTolerance
    testTolerances(const ::Opm::ParameterGroup& param)
    {
        const auto atol = param.getDefault("atol", 1.0e-8);
        const auto rtol = param.getDefault("rtol", 5.0e-12);

        return ErrorTolerance{ atol, rtol };
    }

    std::vector<double>
    loadReference(const ::Opm::ParameterGroup& param)
    {
        namespace fs = boost::filesystem;

        const auto fname =
            fs::path(param.get<std::string>("ref-dir")) / "trans.txt";

        fs::ifstream input(fname);

        if (! input) {
            std::ostringstream os;

            os << "Unable to Open Reference Trans-Data File "
               << fname.filename();

            throw std::domain_error(os.str());
        }

        return {
            std::istream_iterator<double>(input),
            std::istream_iterator<double>()
        };
    }

    bool transfieldAcceptable(const ::Opm::ParameterGroup& param,
                              const std::vector<double>&              trans)
    {
        const auto Tref = loadReference(param);

        if (Tref.size() != trans.size()) {
            return false;
        }

        const auto diff = VectorDifference{ trans, Tref };

        using Vector1 = std::decay<decltype(diff)>::type;
        using Vector2 = std::decay<decltype(Tref)>::type;
        using Ratio   = VectorRatio<Vector1, Vector2>;

        const auto rat = Ratio(diff, Tref);
        const auto tol = testTolerances(param);

        return ! ((pointMetric(diff) > tol.absolute) ||
                  (pointMetric(rat)  > tol.relative));
    }
} // namespace Anonymous

int main(int argc, char* argv[])
try {
    const auto prm = example::initParam(argc, argv);
    const auto pth = example::FilePaths(prm);
    const auto G   = example::initGraph(pth);
    const auto T   = G.transmissibility();
    const auto ok  = transfieldAcceptable(prm, T);

    std::cout << (ok ? "OK" : "FAIL") << '\n';

    if (! ok) {
        return EXIT_FAILURE;
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}

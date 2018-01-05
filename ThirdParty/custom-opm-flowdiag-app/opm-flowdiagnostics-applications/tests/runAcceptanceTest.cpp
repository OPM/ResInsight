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

#include <examples/exampleSetup.hpp>

#include <opm/utility/ECLCaseUtilities.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_file_kw.h>
#include <ert/ecl/ecl_file_view.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/util/ert_unique_ptr.hpp>


namespace {
    struct PoreVolume
    {
        std::vector<double> data;
    };

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

    struct ErrorMeasurement
    {
        double volume;
        double inf;
    };

    struct ErrorTolerance
    {
        double absolute;
        double relative;
    };

    struct AggregateErrors
    {
        std::vector<ErrorMeasurement> absolute;
        std::vector<ErrorMeasurement> relative;
    };

    struct ReferenceToF
    {
        std::vector<double> forward;
        std::vector<double> reverse;
    };

    template <class FieldVariable>
    double volumeMetric(const PoreVolume&    pv,
                        const FieldVariable& x)
    {
        if (x.size() != pv.data.size()) {
            std::ostringstream os;

            os << "Incompatible Array Sizes: Expected 2x"
               << pv.data.size() << ", but got ("
               << pv.data.size() << ", " << x.size() << ')';

            throw std::domain_error(os.str());
        }

        auto num = 0.0;
        auto den = 0.0;

        for (decltype(pv.data.size())
                 i = 0, n = pv.data.size(); i < n; ++i)
        {
            num += std::abs(x[i]) * pv.data[i];
            den +=                  pv.data[i];
        }

        return num / den;
    }

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

    int numDigits(const std::vector<int>& steps)
    {
        if (steps.empty()) {
            return 1;
        }

        const auto m =
            *std::max_element(std::begin(steps), std::end(steps));

        if (m == 0) {
            return 1;
        }

        assert (m > 0);

        return std::floor(std::log10(static_cast<double>(m))) + 1;
    }

    ReferenceToF
    loadReference(const ::Opm::ParameterGroup& param,
                  const int                    step,
                  const int                    nDigits)
    {
        namespace fs = boost::filesystem;

        using VRef = std::reference_wrapper<std::vector<double>>;

        auto fname = fs::path(param.get<std::string>("ref-dir"));
        {
            std::ostringstream os;

            os << "tof-" << std::setw(nDigits) << std::setfill('0')
               << step << ".txt";

            fname /= os.str();
        }

        fs::ifstream input(fname);

        if (! input) {
            std::ostringstream os;

            os << "Unable to Open Reference Data File "
               << fname.filename();

            throw std::domain_error(os.str());
        }

        auto tof = ReferenceToF{};

        auto ref = std::array<VRef,2>{{ std::ref(tof.forward) ,
                                        std::ref(tof.reverse) }};

        {
            auto i = static_cast<decltype(ref[0].get().size())>(0);
            auto t = 0.0;

            while (input >> t) {
                ref[i].get().push_back(t);

                i = (i + 1) % 2;
            }
        }

        if (tof.forward.size() != tof.reverse.size()) {
            std::ostringstream os;

            os << "Unable to Read Consistent ToF Reference Data From "
               << fname.filename();

            throw std::out_of_range(os.str());
        }

        return tof;
    }

    void computeErrors(const PoreVolume&                       pv,
                       const std::vector<double>&              ref,
                       const ::Opm::FlowDiagnostics::Solution& fd,
                       AggregateErrors&                        E)
    {
        const auto tof  = fd.timeOfFlight();
        const auto diff = VectorDifference(tof, ref); //  tof - ref

        using Vector1  = std::decay<decltype(diff)>::type;
        using Vector2  = std::decay<decltype(ref)>::type;
        using Ratio    = VectorRatio<Vector1, Vector2>;

        const auto rat = Ratio(diff, ref); // (tof - ref) / ref

        auto abs = ErrorMeasurement{};
        {
            abs.volume = volumeMetric(pv, diff);
            abs.inf    = pointMetric (    diff);
        }

        auto rel = ErrorMeasurement{};
        {
            rel.volume = volumeMetric(pv, rat);
            rel.inf    = pointMetric (    rat);
        }

        E.absolute.push_back(std::move(abs));
        E.relative.push_back(std::move(rel));
    }

    std::array<AggregateErrors, 2>
    sampleDifferences(example::Setup&&        setup,
                      const std::vector<int>& steps)
    {
        const auto start =
            std::vector<Opm::FlowDiagnostics::CellSet>{};

        const auto nDigits = numDigits(steps);

        const auto pv = PoreVolume{ setup.graph.poreVolume() };

        auto E = std::array<AggregateErrors, 2>{};

        for (const auto& step : steps) {
            if (step == 0) {
                // Ignore initial condition
                continue;
            }

            if (! setup.selectReportStep(step)) {
                continue;
            }

            const auto ref = loadReference(setup.param, step, nDigits);

            {
                const auto fwd = setup.toolbox
                    .computeInjectionDiagnostics(start);

                computeErrors(pv, ref.forward, fwd.fd, E[0]);
            }

            {
                const auto rev = setup.toolbox
                    .computeProductionDiagnostics(start);

                computeErrors(pv, ref.reverse, rev.fd, E[1]);
            }
        }

        return E;
    }

    bool errorAcceptable(const std::vector<ErrorMeasurement>& E,
                         const double                         tol)
    {
        return std::accumulate(std::begin(E), std::end(E), true,
            [tol](const bool ok, const ErrorMeasurement& e)
            {
                // Fine if at least one of .volume or .inf <= tol.
                return ok && ! ((e.volume > tol) && (e.inf > tol));
            });
    }

    bool everythingFine(const AggregateErrors& E,
                        const ErrorTolerance&  tol)
    {
        return errorAcceptable(E.absolute, tol.absolute)
            && errorAcceptable(E.relative, tol.relative);
    }
} // namespace Anonymous

int main(int argc, char* argv[])
try {
    auto setup = example::Setup(argc, argv);

    const auto tol   = testTolerances(setup.param);
    const auto steps = setup.result_set.reportStepIDs();

    const auto E  = sampleDifferences(std::move(setup), steps);
    const auto ok =
        everythingFine(E[0], tol) && everythingFine(E[1], tol);

    std::cout << (ok ? "OK" : "FAIL") << '\n';

    if (! ok) {
        return EXIT_FAILURE;
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}

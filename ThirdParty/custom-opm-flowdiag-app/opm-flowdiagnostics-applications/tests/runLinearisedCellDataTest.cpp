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
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_file_kw.h>
#include <ert/ecl/ecl_file_view.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/util/ert_unique_ptr.hpp>

namespace StringUtils {
    namespace {
        std::string trim(const std::string& s)
        {
            const auto anchor_ws =
                boost::regex(R"~~(^\s+([^\s]+)\s+$)~~");

            auto m = boost::smatch{};

            if (boost::regex_match(s, m, anchor_ws)) {
                return m[1];
            }

            return s;
        }

        std::vector<std::string> split(const std::string& s)
        {
            if (s.empty()) {
                // Single element vector whose only element is the empty
                // string.
                return { "" };
            }

            const auto sep = boost::regex(R"~~([\s,;.|]+)~~");

            using TI = boost::sregex_token_iterator;

            // vector<string>(begin, end)
            //
            // Range is every substring (i.e., token) in input string 's'
            // that does NOT match 'sep'.
            return{ TI(s.begin(), s.end(), sep, -1), TI{} };
        }
    } // namespace Anonymous

    template <typename T>
    struct StringTo;

    template <>
    struct StringTo<int>
    {
        static int value(const std::string& s);
    };

    template <>
    struct StringTo<double>
    {
        static double value(const std::string& s);
    };

    template <>
    struct StringTo<std::string>
    {
        static std::string value(const std::string& s);
    };

    int StringTo<int>::value(const std::string& s)
    {
        return std::stoi(s);
    }

    double StringTo<double>::value(const std::string& s)
    {
        return std::stod(s);
    }

    std::string StringTo<std::string>::value(const std::string& s)
    {
        return trim(s);
    }

    namespace VectorValue {
        template <typename T>
        std::vector<T> get(const std::string& s, std::true_type)
        {
            return split(s);
        }

        template <typename T>
        std::vector<T> get(const std::string& s, std::false_type)
        {
            const auto tokens = split(s);

            auto ret = std::vector<T>{};
            ret.reserve(tokens.size());
            for (const auto& token : tokens) {
                ret.push_back(StringTo<T>::value(token));
            }

            return ret;
        }

        template <typename T>
        std::vector<T> get(const std::string& s)
        {
            return get<T>(s, typename std::is_same<T, std::string>::type());
        }
    } // namespace VectorValue
} // namespace StringUtils

namespace {
    struct Reference
    {
        std::vector<double> data;
    };

    struct Calculated
    {
        std::vector<double> data;
    };

    class VectorUnits
    {
    private:
        using USys = ::Opm::ECLUnits::UnitSystem;

    public:
        using UnitConvention = ::Opm::ECLGraph::UnitConvention;

        VectorUnits()
            : units_({ { "pressure", &USys::pressure } })
        {
        }

        UnitConvention getUnit(const std::string& vector) const
        {
            auto p = units_.find(vector);

            if (p == units_.end()) {
                std::ostringstream os;

                os << "Unsupported Vector Quantity '" << vector << '\'';

                throw std::domain_error(os.str());
            }

            return p->second;
        }

    private:
        std::map<std::string, UnitConvention> units_;
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

    struct ReferenceSolution
    {
        std::vector<double> raw;
        std::vector<double> SI;
    };

    template <class FieldVariable>
    double volumeMetric(const FieldVariable& x)
    {
        auto result = 0.0;

        for (decltype(x.size())
                 i = 0, n = x.size(); i < n; ++i)
        {
            const auto m = std::abs(x[i]);
            result += m * m;
        }

        return std::sqrt(result / x.size());
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

    std::vector<std::string>
    testQuantities(const ::Opm::ParameterGroup& param)
    {
        return StringUtils::VectorValue::
            get<std::string>(param.get<std::string>("quant"));
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

    ReferenceSolution
    loadReference(const ::Opm::ParameterGroup& param,
                  const std::string&           quant,
                  const int                    step,
                  const int                    nDigits)
    {
        namespace fs = boost::filesystem;

        using VRef = std::reference_wrapper<std::vector<double>>;

        auto x   = ReferenceSolution{};
        auto ref = std::array<VRef,2>{{ std::ref(x.raw) ,
                                        std::ref(x.SI ) }};

        auto i = 0;

        for (const auto* q : { "raw", "SI" }) {
            auto fname = fs::path(param.get<std::string>("ref-dir"))
                / boost::algorithm::to_lower_copy(quant);
            {
                std::ostringstream os;

                os << q                  << '-'
                   << std::setw(nDigits) << std::setfill('0')
                   << step               << ".txt";

                fname /= os.str();
            }

            fs::ifstream input(fname);

            if (input) {
                ref[i].get().assign(std::istream_iterator<double>(input),
                                    std::istream_iterator<double>());
            }

            i += 1;
        }

        if (x.raw.size() != x.SI.size()) {
            std::ostringstream os;

            os << "Unable to Read Consistent Reference Data From '"
               << param.get<std::string>("ref-dir") << "' In Step "
               << step;

            throw std::out_of_range(os.str());
        }

        return x;
    }

    void computeErrors(const Reference&  ref,
                       const Calculated& calc,
                       AggregateErrors&  E)
    {
        const auto diff =
            VectorDifference(calc.data, ref.data); //  calc - ref

        using Vector1  = std::decay<decltype(diff)>::type;
        using Vector2  = std::decay<decltype(ref.data)>::type;
        using Ratio    = VectorRatio<Vector1, Vector2>;

        const auto rat = Ratio(diff, ref.data); // (tof - ref) / ref

        auto abs = ErrorMeasurement{};
        {
            abs.volume = volumeMetric(diff);
            abs.inf    = pointMetric (diff);
        }

        auto rel = ErrorMeasurement{};
        {
            rel.volume = volumeMetric(rat);
            rel.inf    = pointMetric (rat);
        }

        E.absolute.push_back(std::move(abs));
        E.relative.push_back(std::move(rel));
    }

    std::unique_ptr<Opm::ECLRestartData>
    openRestartSet(const Opm::ECLCaseUtilities::ResultSet& rset,
                   const int                               step)
    {
        return std::unique_ptr<Opm::ECLRestartData> {
            new Opm::ECLRestartData(rset.restartFile(step))
        };
    }

    std::array<AggregateErrors, 2>
    sampleDifferences(const ::Opm::ECLGraph&                    graph,
                      const ::Opm::ECLCaseUtilities::ResultSet& rset,
                      const ::Opm::ParameterGroup&              param,
                      const std::string&                        quant,
                      const std::vector<int>&                   steps)
    {
        const auto ECLquant = boost::algorithm::to_upper_copy(quant);

        auto unit = VectorUnits()
            .getUnit(boost::algorithm::to_lower_copy(quant));

        const auto start =
            std::vector<Opm::FlowDiagnostics::CellSet>{};

        const auto nDigits = numDigits(steps);

        auto rstrt = std::unique_ptr<Opm::ECLRestartData>{};

        auto E = std::array<AggregateErrors, 2>{};

        for (const auto& step : steps) {
            if (! (rset.isUnifiedRestart() && bool(rstrt))) {
                // Separate (not unified) restart file or this is the first
                // time we're selecting a report step.
                rstrt = openRestartSet(rset, step);
            }

            if (! rstrt->selectReportStep(step)) {
                continue;
            }

            const auto ref = loadReference(param, quant, step, nDigits);

            {
                const auto raw = Calculated {
                    graph.rawLinearisedCellData<double>(*rstrt, ECLquant)
                };

                computeErrors(Reference{ ref.raw }, raw, E[0]);
            }

            {
                const auto SI = Calculated {
                    graph.linearisedCellData(*rstrt, ECLquant, unit)
                };

                computeErrors(Reference{ ref.SI }, SI, E[1]);
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

    ::Opm::ECLGraph
    constructGraph(const Opm::ECLCaseUtilities::ResultSet& rset)
    {
        const auto I = ::Opm::ECLInitFileData(rset.initFile());

        return ::Opm::ECLGraph::load(rset.gridFile(), I);
    }
} // namespace Anonymous

int main(int argc, char* argv[])
try {
    const auto prm  = example::initParam(argc, argv);
    const auto rset = example::identifyResultSet(prm);
    const auto tol  = testTolerances(prm);

    const auto steps = rset.reportStepIDs();
    const auto graph = constructGraph(rset);

    auto all_ok = true;
    for (const auto& quant : testQuantities(prm)) {
        const auto E =
            sampleDifferences(graph, rset, prm, quant, steps);

        const auto ok =
            everythingFine(E[0], tol) && everythingFine(E[1], tol);

        std::cout << quant << ": " << (ok ? "OK" : "FAIL") << '\n';

        all_ok = all_ok && ok;
    }

    if (! all_ok) {
        return EXIT_FAILURE;
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}

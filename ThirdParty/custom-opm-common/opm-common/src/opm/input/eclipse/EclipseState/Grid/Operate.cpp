/*
  Copyright 2019  Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <cmath>
#include <map>
#include <string>

#include "Operate.hpp"

namespace Opm {
namespace Operate {
namespace {
    /*
      The functions in this namespace are those listed as
      available operations in the OPERATE keyword.
    */

    double MULTA(double, double X, double alpha, double beta)
    {
        return alpha * X + beta;
    }

    // NB: The POLY function and the MULTIPLY function both use
    // the R value in the calculation. That implies that we should
    // ideally check that the R property has already been
    // initialized with a valid value, For all the other
    // operations R only appears on the left side of the equation,
    // and can be fully assigned to.
    double POLY(double R, double X, double alpha, double beta)
    {
        return R + alpha * std::pow(X, beta);
    }

    double MULTIPLY(double R, double X, double, double)
    {
        return R * X;
    }

    double SLOG(double, double X, double alpha, double beta)
    {
        return pow(10, alpha + beta * X);
    }

    double LOG10(double, double X, double, double)
    {
        return log10(X);
    }

    double LOGE(double, double X, double, double)
    {
        return log(X);
    }

    double INV(double, double X, double, double)
    {
        return 1.0 / X;
    }

    double MULTX(double, double X, double alpha, double)
    {
        return alpha * X;
    }

    double ADDX(double, double X, double alpha, double)
    {
        return alpha + X;
    }

    double COPY(double, double X, double, double)
    {
        return X;
    }

    double MAXLIM(double, double X, double alpha, double)
    {
        return std::min(alpha, X);
    }

    double MINLIM(double, double X, double alpha, double)
    {
        return std::max(alpha, X);
    }

    double MULTP(double, double X, double alpha, double beta)
    {
        return alpha * pow(X, beta);
    }

    double ABS(double, double X, double, double)
    {
        return std::abs(X);
    }

    using func4 = decltype(&MULTA);
    static const std::map<std::string, func4> operations = {{"MULTA", &MULTA},
                                                            {"POLY", &POLY},
                                                            {"SLOG", &SLOG},
                                                            {"LOG10", &LOG10},
                                                            {"LOGE", &LOGE},
                                                            {"INV", &INV},
                                                            {"MULTX", &MULTX},
                                                            {"ADDX", &ADDX},
                                                            {"COPY", &COPY},
                                                            {"MAXLIM", &MAXLIM},
                                                            {"MINLIM", &MINLIM},
                                                            {"MULTP", &MULTP},
                                                            {"ABS", &ABS},
                                                            {"MULTIPLY", &MULTIPLY}};
}

function get(const std::string& func, double alpha, double beta) {
    auto inner_func = operations.at(func);

    return [alpha, beta, inner_func](double R, double X)
           {
               return inner_func(R,X,alpha,beta);
           };
}

}
}

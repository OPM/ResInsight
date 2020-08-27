/*
  Copyright 2010, 2019 SINTEF Digital
  Copyright 2010, 2019 Equinor ASA

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

#ifndef OPM_ROOTFINDERS_HEADER
#define OPM_ROOTFINDERS_HEADER

#include <opm/common/ErrorMacros.hpp>

#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>

namespace Opm
{

    struct ThrowOnError
    {
        static double handleBracketingFailure(const double x0, const double x1, const double f0, const double f1)
        {
            OPM_THROW(std::runtime_error, "Error in parameters, zero not bracketed: [a, b] = ["
                  << x0 << ", " << x1 << "]    f(a) = " << f0 << "   f(b) = " << f1);
            return -1e100; // Never reached.
        }
        static double handleTooManyIterations(const double x0, const double x1, const int maxiter)
        {
            OPM_THROW(std::runtime_error, "Maximum number of iterations exceeded: " << maxiter << "\n"
                  << "Current interval is [" << std::min(x0, x1) << ", "
                  << std::max(x0, x1) << "] abs(x0-x1) " << std::abs(x0-x1));
            return -1e100; // Never reached.
        }
    };


    struct WarnAndContinueOnError
    {
        static double handleBracketingFailure(const double x0, const double x1, const double f0, const double f1)
        {
            OPM_REPORT;
            std::cerr << "Error in parameters, zero not bracketed: [a, b] = ["
                      << x0 << ", " << x1 << "]    f(a) = " << f0 << "   f(b) = " << f1
                      << "";
            return std::fabs(f0) < std::fabs(f1) ? x0 : x1;
        }
        static double handleTooManyIterations(const double x0, const double x1, const int maxiter)
        {
            OPM_REPORT;
            std::cerr << "Maximum number of iterations exceeded: " << maxiter
                      << ", current interval is [" << std::min(x0, x1) << ", "
                      << std::max(x0, x1) << "]  abs(x0-x1) " << std::abs(x0-x1);
            return 0.5*(x0 + x1);
        }
    };


    struct ContinueOnError
    {
        static double handleBracketingFailure(const double x0, const double x1, const double f0, const double f1)
        {
            return std::fabs(f0) < std::fabs(f1) ? x0 : x1;
        }
        static double handleTooManyIterations(const double x0, const double x1, const int /*maxiter*/)
        {
            return 0.5*(x0 + x1);
        }
    };



    template <class ErrorPolicy = ThrowOnError>
    class RegulaFalsi
    {
    public:


        /// Implements a modified regula falsi method as described in
        /// "Improved algorithms of Illinois-type for the numerical
        /// solution of nonlinear equations"
        /// by J. A. Ford.
        /// Current variant is the 'Pegasus' method.
        template <class Functor>
        inline static double solve(const Functor& f,
                                   const double a,
                                   const double b,
                                   const int max_iter,
                                   const double tolerance,
                                   int& iterations_used)
        {
            using namespace std;
            const double macheps = numeric_limits<double>::epsilon();
            const double eps = tolerance + macheps*max(max(fabs(a), fabs(b)), 1.0);

            double x0 = a;
            double x1 = b;
            double f0 = f(x0);
            const double epsF = tolerance + macheps*max(fabs(f0), 1.0);
            if (fabs(f0) < epsF) {
                return x0;
            }
            double f1 = f(x1);
            if (fabs(f1) < epsF) {
                return x1;
            }
            if (f0*f1 > 0.0) {
                return ErrorPolicy::handleBracketingFailure(a, b, f0, f1);
            }
            iterations_used = 0;
            // In every iteraton, x1 is the last point computed,
            // and x0 is the last point computed that makes it a bracket.
            while (fabs(x1 - x0) >= eps) {
                double xnew = regulaFalsiStep(x0, x1, f0, f1);
                double fnew = f(xnew);
// 		cout << "xnew = " << xnew << "    fnew = " << fnew << endl;
                ++iterations_used;
                if (iterations_used > max_iter) {
                    return ErrorPolicy::handleTooManyIterations(x0, x1, max_iter);
                }
                if (fabs(fnew) < epsF) {
                    return xnew;
                }
                // Now we must check which point we must replace.
                if ((fnew > 0.0) == (f0 > 0.0)) {
                    // We must replace x0.
                    x0 = x1;
                    f0 = f1;
                } else {
                    // We must replace x1, this is the case where
                    // the modification to regula falsi kicks in,
                    // by modifying f0.
                    // 1. The classic Illinois method
//                  const double gamma = 0.5;
                    // @afr: The next two methods do not work??!!?
                    // 2. The method called 'Method 3' in the paper.
//                  const double phi0 = f1/f0;
//                  const double phi1 = fnew/f1;
//                  const double gamma = 1.0 - phi1/(1.0 - phi0);
                    // 3. The method called 'Method 4' in the paper.
//                  const double phi0 = f1/f0;
//                  const double phi1 = fnew/f1;
//                  const double gamma = 1.0 - phi0 - phi1;
//                  cout << "phi0 = " << phi0 <<" phi1 = " << phi1 <<
//                  " gamma = " << gamma << endl;
                    // 4. The 'Pegasus' method
                    const double gamma = f1/(f1 + fnew);
                    f0 *= gamma;
                }
                x1 = xnew;
                f1 = fnew;
            }
            return 0.5*(x0 + x1);
        }


        /// Implements a modified regula falsi method as described in
        /// "Improved algorithms of Illinois-type for the numerical
        /// solution of nonlinear equations"
        /// by J. A. Ford.
        /// Current variant is the 'Pegasus' method.
        /// This version takes an extra parameter for the initial guess.
        template <class Functor>
        inline static double solve(const Functor& f,
                                   const double initial_guess,
                                   const double a,
                                   const double b,
                                   const int max_iter,
                                   const double tolerance,
                                   int& iterations_used)
        {
            using namespace std;
            const double macheps = numeric_limits<double>::epsilon();
            const double eps = tolerance + macheps*max(max(fabs(a), fabs(b)), 1.0);

            double f_initial = f(initial_guess);
            const double epsF = tolerance + macheps*max(fabs(f_initial), 1.0);
            if (fabs(f_initial) < epsF) {
                return initial_guess;
            }
            double x0 = a;
            double x1 = b;
            double f0 = f_initial;
            double f1 = f_initial;
            if (x0 != initial_guess) {
                f0 = f(x0);
                if (fabs(f0) < epsF) {
                    return x0;
                }
            }
            if (x1 != initial_guess) {
                f1 = f(x1);
                if (fabs(f1) < epsF) {
                    return x1;
                }
            }
            if (f0*f_initial < 0.0) {
                x1 = initial_guess;
                f1 = f_initial;
            } else {
                x0 = initial_guess;
                f0 = f_initial;
            }
            if (f0*f1 > 0.0) {
                return ErrorPolicy::handleBracketingFailure(a, b, f0, f1);
            }
            iterations_used = 0;
            // In every iteraton, x1 is the last point computed,
            // and x0 is the last point computed that makes it a bracket.
            while (fabs(x1 - x0) >= eps) {
                double xnew = regulaFalsiStep(x0, x1, f0, f1);
                double fnew = f(xnew);
// 		cout << "xnew = " << xnew << "    fnew = " << fnew << endl;
                ++iterations_used;
                if (iterations_used > max_iter) {
                    return ErrorPolicy::handleTooManyIterations(x0, x1, max_iter);
                }
                if (fabs(fnew) < epsF) {
                    return xnew;
                }
                // Now we must check which point we must replace.
                if ((fnew > 0.0) == (f0 > 0.0)) {
                    // We must replace x0.
                    x0 = x1;
                    f0 = f1;
                } else {
                    // We must replace x1, this is the case where
                    // the modification to regula falsi kicks in,
                    // by modifying f0.
                    // 1. The classic Illinois method
//      const double gamma = 0.5;
                    // @afr: The next two methods do not work??!!?
                    // 2. The method called 'Method 3' in the paper.
// 		    const double phi0 = f1/f0;
// 		    const double phi1 = fnew/f1;
// 		    const double gamma = 1.0 - phi1/(1.0 - phi0);
                    // 3. The method called 'Method 4' in the paper.
// 		    const double phi0 = f1/f0;
// 		    const double phi1 = fnew/f1;
// 		    const double gamma = 1.0 - phi0 - phi1;
// 		    cout << "phi0 = " << phi0 <<" phi1 = " << phi1 <<
// 		    " gamma = " << gamma << endl;
                    // 4. The 'Pegasus' method
                    const double gamma = f1/(f1 + fnew);
                    f0 *= gamma;
                }
                x1 = xnew;
                f1 = fnew;
            }
            return 0.5*(x0 + x1);
        }


    private:
        inline static double regulaFalsiStep(const double a,
                                             const double b,
                                             const double fa,
                                             const double fb)
        {
            assert(fa*fb < 0.0);
            return (b*fa - a*fb)/(fa - fb);
        }


    };



    template <class ErrorPolicy = ThrowOnError>
    class RegulaFalsiBisection
    {
    public:
        inline static std::string name()
        {
            return "RegulaFalsiBisection";
        }

        /// Implements a modified regula falsi method as described in
        /// "Improved algorithms of Illinois-type for the numerical
        /// solution of nonlinear equations"
        /// by J. A. Ford. Current variant is the 'Pegasus' method.
        /// Combines this method with the bisection method, inspired by
        /// http://phillipmfeldman.org/Python/roots/find_roots.html
        template <class Functor>
        inline static double solve(const Functor& f,
                                   const double a,
                                   const double b,
                                   const int max_iter,
                                   const double tolerance,
                                   int& iterations_used)
        {
            using namespace std;
            const double sqrt_half = std::sqrt(0.5);
            const double macheps = numeric_limits<double>::epsilon();
            const double eps = tolerance + macheps * max(max(fabs(a), fabs(b)), 1.0);

            double x0 = a;
            double x1 = b;
            double f0 = f(x0);
            const double epsF = tolerance + macheps * max(fabs(f0), 1.0);
            if (fabs(f0) < epsF) {
                return x0;
            }
            double f1 = f(x1);
            if (fabs(f1) < epsF) {
                return x1;
            }
            if (f0 * f1 > 0.0) {
                return ErrorPolicy::handleBracketingFailure(a, b, f0, f1);
            }
            iterations_used = 0;
            // In every iteraton, x1 is the last point computed,
            // and x0 is the last point computed that makes it a bracket.
            double width = fabs(x1 - x0);
            double contraction = 1.0;
            while (width >= eps) {
                // If we are contracting sufficiently to at least halve
                // the interval width in two iterations we use regula
                // falsi. Otherwise, we take a bisection step to avoid
                // slow convergence.
                const double xnew = (contraction < sqrt_half)
                    ? regulaFalsiStep(x0, x1, f0, f1)
                    : bisectionStep(x0, x1, f0, f1);
                const double fnew = f(xnew);
                ++iterations_used;
                if (iterations_used > max_iter) {
                    return ErrorPolicy::handleTooManyIterations(x0, x1, max_iter);
                }
                if (fabs(fnew) < epsF) {
                    return xnew;
                }
                // Now we must check which point we must replace.
                if ((fnew > 0.0) == (f0 > 0.0)) {
                    // We must replace x0.
                    x0 = x1;
                    f0 = f1;
                } else {
                    // We must replace x1, this is the case where
                    // the modification to regula falsi kicks in,
                    // by modifying f0.
                    // 1. The classic Illinois method
                    // const double gamma = 0.5;
                    // @afr: The next two methods do not work??!!?
                    // 2. The method called 'Method 3' in the paper.
                    // const double phi0 = f1/f0;
                    // const double phi1 = fnew/f1;
                    // const double gamma = 1.0 - phi1/(1.0 - phi0);
                    // 3. The method called 'Method 4' in the paper.
                    // const double phi0 = f1/f0;
                    // const double phi1 = fnew/f1;
                    // const double gamma = 1.0 - phi0 - phi1;
                    //                  cout << "phi0 = " << phi0 <<" phi1 = " << phi1 <<
                    //                  " gamma = " << gamma << endl;
                    // 4. The 'Pegasus' method
                    const double gamma = f1 / (f1 + fnew);
                    // 5. Straight unmodified Regula Falsi
                    // const double gamma = 1.0;
                    f0 *= gamma;
                }
                x1 = xnew;
                f1 = fnew;
                const double widthnew = fabs(x1 - x0);
                contraction = widthnew/width;
                width = widthnew;
            }
            return 0.5 * (x0 + x1);
        }


    private:
        inline static double regulaFalsiStep(const double a, const double b, const double fa, const double fb)
        {
            assert(fa * fb < 0.0);
            return (b * fa - a * fb) / (fa - fb);
        }
        inline static double bisectionStep(const double a, const double b, const double fa, const double fb)
        {
            static_cast<void>(fa);
            static_cast<void>(fb);
            assert(fa * fb < 0.0);
            return 0.5*(a + b);
        }
    };





    /// Attempts to find an interval bracketing a zero by successive
    /// enlargement of search interval.
    template <class Functor>
    inline void bracketZero(const Functor& f,
                            const double x0,
                            const double dx,
                            double& a,
                            double& b)
    {
        const int max_iters = 100;
        double f0 = f(x0);
        double cur_dx = dx;
        int i = 0;
        for (; i < max_iters; ++i) {
            double x = x0 + cur_dx;
            double f_new = f(x);
            if (f0*f_new <= 0.0) {
                break;
            }
            cur_dx = -2.0*cur_dx;
        }
        if (i == max_iters) {
            OPM_THROW(std::runtime_error, "Could not bracket zero in " << max_iters << "iterations.");
        }
        if (cur_dx < 0.0) {
            a = x0 + cur_dx;
            b = i < 2 ? x0 : x0 + 0.25*cur_dx;
        } else {
            a = i < 2 ? x0 : x0 + 0.25*cur_dx;
            b = x0 + cur_dx;
        }
    }





} // namespace Opm




#endif // OPM_ROOTFINDERS_HEADER

/*
  Copyright 2013 Andreas Lauser
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

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
#ifndef OPM_ERRORMACROS_HPP
#define OPM_ERRORMACROS_HPP

#include <opm/common/OpmLog/OpmLog.hpp>

#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <cassert>

// macros for reporting to stderr
#ifdef OPM_VERBOSE // Verbose mode
# include <iostream>
# define OPM_REPORT do { std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] " } while (false)
# define OPM_MESSAGE(x) do { OPM_REPORT; std::cerr << x << "\n"; } while (false)
# define OPM_MESSAGE_IF(cond, m) do {if(cond) OPM_MESSAGE(m);} while (false)
#else // non-verbose mode (default)
# define OPM_REPORT do {} while (false)
# define OPM_MESSAGE(x) do {} while (false)
# define OPM_MESSAGE_IF(cond, m) do {} while (false)
#endif

// Macro to throw an exception. NOTE: For this macro to work, the
// exception class must exhibit a constructor with the signature
// (const std::string &message). Since this condition is not fulfilled
// for the std::exception, you should use this macro with some
// exception class derived from either std::logic_error or
// std::runtime_error.
//
// Usage: OPM_THROW(ExceptionClass, "Error message " << value);
#define OPM_THROW(Exception, message)                                   \
    do {                                                                \
        std::ostringstream oss__;                                       \
        oss__ << "[" << __FILE__ << ":" << __LINE__ << "] " << message; \
        ::Opm::OpmLog::error(oss__.str());                                \
        throw Exception(oss__.str());                                   \
    } while (false)

// Same as OPM_THROW, except for not making an OpmLog::error() call.
//
// Usage: OPM_THROW_NOLOG(ExceptionClass, "Error message " << value);
#define OPM_THROW_NOLOG(Exception, message)                             \
    do {                                                                \
        std::ostringstream oss__;                                       \
        oss__ << "[" << __FILE__ << ":" << __LINE__ << "] " << message; \
        throw Exception(oss__.str());                                   \
    } while (false)

// throw an exception if a condition is true
#define OPM_ERROR_IF(condition, message) do {if(condition){ OPM_THROW(std::logic_error, message);}} while(false)

#endif // OPM_ERRORMACROS_HPP

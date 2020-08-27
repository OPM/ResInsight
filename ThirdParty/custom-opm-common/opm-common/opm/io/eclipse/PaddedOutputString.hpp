/*
  Copyright (c) 2018 Statoil ASA

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

#ifndef OPM_PADDEDOUTPUTSTRING_HEADER_HPP
#define OPM_PADDEDOUTPUTSTRING_HEADER_HPP

#include <algorithm>
#include <array>
#include <cstring>
#include <cstddef>
#include <string>

namespace Opm { namespace EclIO {

    /// Null-terminated, left adjusted, space padded array of N characters.
    ///
    /// Simple container of character data.  Exists solely for purpose of
    /// outputting std::string (or types convertible to std::string) as
    /// Fortran-style \code character (len=N) \endcode values.
    ///
    /// \tparam N Number of characters.
    template <std::size_t N>
    class PaddedOutputString
    {
    public:
        PaddedOutputString()
        {
            this->clear();
        }

        explicit PaddedOutputString(const std::string& s)
            : PaddedOutputString()
        {
            this->copy_in(s.c_str(), s.size());
        }

        ~PaddedOutputString() = default;

        PaddedOutputString(const PaddedOutputString& rhs) = default;
        PaddedOutputString(PaddedOutputString&& rhs) = default;

        PaddedOutputString& operator=(const PaddedOutputString& rhs) = default;
        PaddedOutputString& operator=(PaddedOutputString&& rhs) = default;

        /// Assign from \code std::string \endcode.
        PaddedOutputString& operator=(const std::string& s)
        {
            this->clear();
            this->copy_in(s.data(), s.size());

            return *this;
        }

        const char* c_str() const
        {
            return this->s_.data();
        }

    private:
        enum : typename std::array<char, N + 1>::size_type { NChar = N };

        std::array<char, NChar + 1> s_;

        /// Clear contents of internal array (fill with ' ').
        void clear()
        {
            this->s_.fill(' ');
            this->s_[NChar] = '\0';
        }

        /// Assign new value to internal array (left adjusted, space padded
        /// and null-terminated).
        void copy_in(const char*                                           s,
                     const typename std::array<char, NChar + 1>::size_type len)
        {
            const auto ncpy = std::min(len, static_cast<decltype(len)>(NChar));

            // Note: Not strcpy() or strncpy() here.  The former has no bounds
            // checking, the latter writes a null-terminator at position 'ncpy'
            // (s_[ncpy]) which violates the post condition if ncpy < NChar.
            std::memcpy(this->s_.data(), s,
                        ncpy * sizeof *this->s_.data());
        }
    };

}} // Opm::EclIO
#endif // OPM_PADDEDOUTPUTSTRING_HEADER_HPP

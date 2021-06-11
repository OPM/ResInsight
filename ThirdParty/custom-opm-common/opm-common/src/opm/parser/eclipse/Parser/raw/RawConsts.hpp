/*
  Copyright 2013 Statoil ASA.

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

#ifndef RAWCONSTS_HPP
#define	RAWCONSTS_HPP

#include <string>

namespace Opm {

    /// Consts used in the non semantic, raw parsing of the eclipse file
    namespace RawConsts {
        const char slash = '/';
        const char quote = '\'';
        const std::string include = "INCLUDE";
        const std::string end = "END";
        const std::string endinclude = "ENDINC";
        const std::string paths = "PATHS";
        const std::string pyinput = "PYINPUT";
        const unsigned int maxKeywordLength = 8;

        /* The lookup uses some bit-tricks to achieve branchless lookup in the
         * table. It has a robustness weakness because all input characters
         * that are non-ascii will be interpreted only by their 7 smallest
         * bits. However:
         * * only ascii input is supported, so a non-ascii input deck
         *   is fundamentally broken
         * * the underlying storage is still char which when signed is
         *   +-127
         * * it's reasonable to assume non-ascii input only shows up in
         * comments, which most of the time will be skipped when
         * looking for separators
         */

        constexpr bool sep_table[ 128 ] = {
            0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        };

        struct is_separator {
            /*
             * ch is SOH (ASCII 1), space, comma, \r, \n, \t, \v or \f => true
             * else false
             */
            constexpr bool operator()( int ch ) const {
                return sep_table[ ch & 0x7f ];
            }
        };

        constexpr bool q_table[ 128 ] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        };

        struct is_quote {
            /*
             * ch is ' or " => true
             * else false
             */
            constexpr bool operator()( int ch ) const {
                return q_table[ ch & 0x7f ];
            }
        };
    }
}


#endif	/* RAWCONSTS_HPP */


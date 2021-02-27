/*
  Copyright 2019 Equinor ASA.

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

#ifndef OPM_IO_ECLIODATA_HPP
#define OPM_IO_ECLIODATA_HPP

#include <tuple>

namespace Opm { namespace EclIO {

    // type MESS have no assisiated data
    enum eclArrType {
        INTE, REAL, DOUB, CHAR, LOGI, MESS, C0NN
    };

    // named constants related to binary file format
    const unsigned int true_value_ecl = 0xffffffff;
    const unsigned int true_value_ix = 0x1000000;
    const unsigned int false_value = 0x00000000;


    const int sizeOfInte =  4;    // number of bytes pr integer (inte) element
    const int sizeOfReal =  4;    // number of bytes pr float (real) element
    const int sizeOfDoub =  8;    // number of bytes pr double (doub) element
    const int sizeOfLogi =  4;    // number of bytes pr bool (logi) element
    const int sizeOfChar =  8;    // number of bytes pr string (char) element

    const int MaxBlockSizeInte = 4000;    // Maximum block size for INTE arrays in binary files
    const int MaxBlockSizeReal = 4000;    // Maximum block size for REAL arrays in binary files
    const int MaxBlockSizeDoub = 8000;    // Maximum block size for DOUB arrays in binary files
    const int MaxBlockSizeLogi = 4000;    // Maximum block size for LOGI arrays in binary files
    const int MaxBlockSizeChar =  840;    // Maximum block size for CHAR arrays in binary files

    // named constants related to formatted file file format
    const int MaxNumBlockInte = 1000;    // maximum number of Inte values in block => hard line shift
    const int MaxNumBlockReal = 1000;    // maximum number of Real values in block => hard line shift
    const int MaxNumBlockDoub = 1000;    // maximum number of Doub values in block => hard line shift
    const int MaxNumBlockLogi = 1000;    // maximum number of Logi values in block => hard line shift
    const int MaxNumBlockChar =  105;    // maximum number of Char values in block => hard line shift

    const int numColumnsInte = 6;        // number of columns for Inte values
    const int numColumnsReal = 4;        // number of columns for Real values
    const int numColumnsDoub = 3;        // number of columns for Doub values
    const int numColumnsLogi = 25;       // number of columns for Logi values
    const int numColumnsChar = 7;        // number of columns for Char values

    const int columnWidthInte = 12;      // number of characters fore each Inte Element
    const int columnWidthReal = 17;      // number of characters fore each Inte Element
    const int columnWidthDoub = 23;      // number of characters fore each Inte Element
    const int columnWidthLogi = 3;       // number of characters fore each Inte Element
    const int columnWidthChar = 11;      // number of characters fore each Inte Element

}} // namespace Opm::EclIO

#endif // OPM_IO_ECLIODATA_HPP

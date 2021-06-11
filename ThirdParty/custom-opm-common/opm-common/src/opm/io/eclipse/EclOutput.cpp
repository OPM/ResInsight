/*
   Copyright 2019 Statoil ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/EclUtil.hpp>

#include <opm/common/ErrorMacros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <iomanip>
#include <iostream>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace Opm { namespace EclIO {

EclOutput::EclOutput(const std::string&            filename,
                     const bool                    formatted,
                     const std::ios_base::openmode mode)
    : isFormatted{formatted}
{
    const auto binmode = mode | std::ios_base::binary;
    ix_standard = false;

    this->ofileH.open(filename, this->isFormatted ? mode : binmode);
}


template<>
void EclOutput::write<std::string>(const std::string& name,
                                   const std::vector<std::string>& data)
{
    // array type will be assumed CHAR if maximum string length is 8 or less
    // If maximum string length is > 8, C0nn will be used with element size equal to
    // maximum string length

    int maximum_length = 8;

    if (data.size() > 0) {
        auto it = std::max_element(data.begin(), data.end(), []
                                   (const std::string& str1, const std::string& str2)
        {
            return str2.size() > str1.size();
        });

        maximum_length = it->size();
    }


    if (isFormatted)
    {
        if (maximum_length > sizeOfChar){
            writeFormattedHeader(name, data.size(), C0NN, maximum_length);
            writeFormattedCharArray(data, maximum_length);
        } else {
            writeFormattedHeader(name, data.size(), CHAR, sizeOfChar);
            writeFormattedCharArray(data, sizeOfChar);
        }
    }
    else
    {
        if (maximum_length > sizeOfChar){
            writeBinaryHeader(name, data.size(), C0NN, maximum_length);
            writeBinaryCharArray(data, maximum_length);
        } else {
            writeBinaryHeader(name, data.size(), CHAR, sizeOfChar);
            writeBinaryCharArray(data, sizeOfChar);
       }
    }
}

void EclOutput::write(const std::string& name, const std::vector<std::string>& data, int element_size)
{
    // array type will be assumed C0NN (not CHAR). Also in cases where element size is 8 or less

    if (data.size() > 0) {
        auto it = std::max_element(data.begin(), data.end(), []
                                   (const std::string& str1, const std::string& str2)
        {
            return str2.size() > str1.size();
        });

        if (it->size() > static_cast<size_t>(element_size))
            OPM_THROW(std::runtime_error, "specified element size for type C0NN less than maximum string length in ouput data");
    }

    if (isFormatted)
    {
        if (element_size > sizeOfChar){
            writeFormattedHeader(name, data.size(), C0NN, element_size);
            writeFormattedCharArray(data, element_size);
        } else {
            writeFormattedHeader(name, data.size(), C0NN, sizeOfChar);
            writeFormattedCharArray(data, sizeOfChar);
        }
    }
    else
    {
        if (element_size > sizeOfChar){
            writeBinaryHeader(name, data.size(), C0NN, element_size);
            writeBinaryCharArray(data, element_size);
        } else {
            writeBinaryHeader(name, data.size(), C0NN, sizeOfChar);
            writeBinaryCharArray(data, sizeOfChar);
        }
    }
}

template <>
void EclOutput::write<PaddedOutputString<8>>
    (const std::string&                        name,
     const std::vector<PaddedOutputString<8>>& data)
{
    if (this->isFormatted) {
        writeFormattedHeader(name, data.size(), CHAR, sizeOfChar);
        writeFormattedCharArray(data);
    }
    else {
        writeBinaryHeader(name, data.size(), CHAR, sizeOfChar);
        writeBinaryCharArray(data);
    }
}

void EclOutput::message(const std::string& msg)
{
    // Generate message, i.e., output vector of type eclArrType::MESS,
    // by passing an empty std::vector of element type 'char'.  Entire
    // contents of message contained in the 'msg' string.

    this->write(msg, std::vector<char>{});
}

void EclOutput::flushStream()
{
    this->ofileH.flush();
}

void EclOutput::writeBinaryHeader(const std::string&arrName, int64_t size, eclArrType arrType, int element_size)
{
    int bhead = flipEndianInt(16);
    std::string name = arrName + std::string(8 - arrName.size(),' ');

    // write X231 header if size larger that limits for 4 byte integers
    if (size > std::numeric_limits<int>::max()) {
        int64_t val231 = std::pow(2,31);
        int64_t x231 = size / val231;

        int flippedx231 = flipEndianInt(static_cast<int>( (-1)*x231 ));

        ofileH.write(reinterpret_cast<char*>(&bhead), sizeof(bhead));
        ofileH.write(name.c_str(), 8);
        ofileH.write(reinterpret_cast<char*>(&flippedx231), sizeof(flippedx231));
        ofileH.write("X231", 4);
        ofileH.write(reinterpret_cast<char*>(&bhead), sizeof(bhead));

        size = size - (x231 * val231);
    }

    int flippedSize = flipEndianInt(size);

    ofileH.write(reinterpret_cast<char*>(&bhead), sizeof(bhead));

    ofileH.write(name.c_str(), 8);
    ofileH.write(reinterpret_cast<char*>(&flippedSize), sizeof(flippedSize));

    std::string c0nn_str;

    if (arrType == C0NN){
        std::ostringstream ss;
        ss << "C" << std::setw(3) << std::setfill('0') << element_size;
        c0nn_str = ss.str();
    }

    switch(arrType) {
    case INTE:
        ofileH.write("INTE", 4);
        break;
    case REAL:
        ofileH.write("REAL", 4);
        break;
    case DOUB:
        ofileH.write("DOUB", 4);
        break;
    case LOGI:
        ofileH.write("LOGI", 4);
        break;
    case CHAR:
        ofileH.write("CHAR", 4);
        break;
    case C0NN:
        ofileH.write(c0nn_str.c_str(), 4);
        break;
    case MESS:
        ofileH.write("MESS", 4);
        break;
    }

    ofileH.write(reinterpret_cast<char *>(&bhead), sizeof(bhead));
}

template <typename T>
void EclOutput::writeBinaryArray(const std::vector<T>& data)
{
    int num, rval;
    int64_t rest;
    int dhead;
    float value_f;
    double value_d;
    int intVal;

    int64_t n = 0;
    int64_t size = data.size();

    eclArrType arrType = MESS;

    if (typeid(std::vector<T>) == typeid(std::vector<int>)) {
        arrType = INTE;
    } else if (typeid(std::vector<T>) == typeid(std::vector<float>)) {
        arrType = REAL;
    } else if (typeid(std::vector<T>) == typeid(std::vector<double>)) {
        arrType = DOUB;
    } else if (typeid(std::vector<T>) == typeid(std::vector<bool>)) {
        arrType = LOGI;
    }

    auto sizeData = block_size_data_binary(arrType);

    int sizeOfElement = std::get<0>(sizeData);
    int maxBlockSize = std::get<1>(sizeData);
    int maxNumberOfElements = maxBlockSize / sizeOfElement;

    if (!ofileH.is_open()) {
        OPM_THROW(std::runtime_error, "fstream fileH not open for writing");
    }

    int logi_true_val = ix_standard ? true_value_ix : true_value_ecl;

    rest = size * static_cast<int64_t>(sizeOfElement);
    while (rest > 0) {
        if (rest > maxBlockSize) {
            rest -= maxBlockSize;
            num = maxNumberOfElements;
        } else {
            num = static_cast<int>(rest) / sizeOfElement;
            rest = 0;
        }

        dhead = flipEndianInt(num * sizeOfElement);

        ofileH.write(reinterpret_cast<char*>(&dhead), sizeof(dhead));

        for (int i = 0; i < num; i++) {
            if (arrType == INTE) {
                rval = flipEndianInt(data[n]);
                ofileH.write(reinterpret_cast<char*>(&rval), sizeof(rval));
            } else if (arrType == REAL) {
                value_f = flipEndianFloat(data[n]);
                ofileH.write(reinterpret_cast<char*>(&value_f), sizeof(value_f));
            } else if (arrType == DOUB) {
                value_d = flipEndianDouble(data[n]);
                ofileH.write(reinterpret_cast<char*>(&value_d), sizeof(value_d));
            } else if (arrType == LOGI) {
                intVal = data[n] ? logi_true_val : false_value;
                ofileH.write(reinterpret_cast<char*>(&intVal), sizeOfElement);
            } else {
                std::cerr << "type not supported in write binaryarray\n";
                std::exit(EXIT_FAILURE);
            }

            n++;
        }

        ofileH.write(reinterpret_cast<char*>(&dhead), sizeof(dhead));
    }
}


template void EclOutput::writeBinaryArray<int>(const std::vector<int>& data);
template void EclOutput::writeBinaryArray<float>(const std::vector<float>& data);
template void EclOutput::writeBinaryArray<double>(const std::vector<double>& data);
template void EclOutput::writeBinaryArray<bool>(const std::vector<bool>& data);
template void EclOutput::writeBinaryArray<char>(const std::vector<char>& data);


void EclOutput::writeBinaryCharArray(const std::vector<std::string>& data, int element_size)
{
    int num,dhead;

    int n = 0;
    int size = data.size();

    auto sizeData = block_size_data_binary(CHAR);

    if (element_size > sizeOfChar){
        std::get<1>(sizeData)= std::get<1>(sizeData) / std::get<0>(sizeData) * element_size;
        std::get<0>(sizeData) = element_size;
    }

    int sizeOfElement = std::get<0>(sizeData);
    int maxBlockSize = std::get<1>(sizeData);
    int maxNumberOfElements = maxBlockSize / sizeOfElement;

    int rest = size * sizeOfElement;

    if (!ofileH.is_open()) {
        OPM_THROW(std::runtime_error,"fstream fileH not open for writing");
    }

    while (rest > 0) {
        if (rest > maxBlockSize) {
            rest -= maxBlockSize;
            num = maxNumberOfElements;
        } else {
            num = rest / sizeOfElement;
            rest = 0;
        }

        dhead = flipEndianInt(num * sizeOfElement);

        ofileH.write(reinterpret_cast<char*>(&dhead), sizeof(dhead));

        for (int i = 0; i < num; i++) {
            std::string tmpStr = data[n] + std::string(sizeOfElement - data[n].size(),' ');
            ofileH.write(tmpStr.c_str(), sizeOfElement);
            n++;
        }

        ofileH.write(reinterpret_cast<char*>(&dhead), sizeof(dhead));
    }
}

void EclOutput::writeBinaryCharArray(const std::vector<PaddedOutputString<8>>& data)
{
    const auto size = data.size();

    const auto sizeData = block_size_data_binary(CHAR);

    const int sizeOfElement       = std::get<0>(sizeData);
    const int maxBlockSize        = std::get<1>(sizeData);
    const int maxNumberOfElements = maxBlockSize / sizeOfElement;

    int rest = size * sizeOfElement;

    if (!ofileH.is_open()) {
        OPM_THROW(std::runtime_error,"fstream fileH not open for writing");
    }

    auto elm = data.begin();
    while (rest > 0) {
        const auto numElm = (rest > maxBlockSize)
            ? maxNumberOfElements
            : rest / sizeOfElement;

        rest = (rest > maxBlockSize) ? rest - maxBlockSize : 0;

        auto dhead = flipEndianInt(numElm * sizeOfElement);

        ofileH.write(reinterpret_cast<char*>(&dhead), sizeof(dhead));

        for (auto i = 0*numElm; i < numElm; ++i, ++elm) {
            ofileH.write(elm->c_str(), sizeOfElement);
        }

        ofileH.write(reinterpret_cast<char*>(&dhead), sizeof(dhead));
    }
}

void EclOutput::writeFormattedHeader(const std::string& arrName, int size, eclArrType arrType, int element_size)
{
    std::string name = arrName + std::string(8 - arrName.size(),' ');

    ofileH << " '" << name << "' " << std::setw(11) << size;

    std::string c0nn_str;

    if (arrType == C0NN){
        std::ostringstream ss;
        ss << "C" << std::setw(3) << std::setfill('0') << element_size;
        c0nn_str = ss.str();
    }


    switch (arrType) {
    case INTE:
        ofileH << " 'INTE'" <<  std::endl;
        break;
    case REAL:
        ofileH << " 'REAL'" <<  std::endl;
        break;
    case DOUB:
        ofileH << " 'DOUB'" <<  std::endl;
        break;
    case LOGI:
        ofileH << " 'LOGI'" <<  std::endl;
        break;
    case CHAR:
        ofileH << " 'CHAR'" <<  std::endl;
        break;
    case C0NN:
        ofileH << " '" << c0nn_str << "'" <<  std::endl;
        break;
    case MESS:
        ofileH << " 'MESS'" <<  std::endl;
        break;
    }
}


std::string EclOutput::make_real_string_ecl(float value) const
{
    char buffer [15];
    std::sprintf (buffer, "%10.7E", value);

    if (value == 0.0) {
        return "0.00000000E+00";
    } else {
        if (std::isnan(value))
            return "NAN";

        if (std::isinf(value)) {
            if (value > 0)
                return "INF";
            else
                return "-INF";
        }

        std::string tmpstr(buffer);

        int exp =  value < 0.0 ? std::stoi(tmpstr.substr(11, 3)) :  std::stoi(tmpstr.substr(10, 3));

        if (value < 0.0) {
            tmpstr = "-0." + tmpstr.substr(1, 1) + tmpstr.substr(3, 7) + "E";
        } else {
            tmpstr = "0." + tmpstr.substr(0, 1) + tmpstr.substr(2, 7) +"E";
        }

        std::sprintf (buffer, "%+03i", exp+1);
        tmpstr = tmpstr+buffer;

        return tmpstr;
    }
}

std::string EclOutput::make_real_string_ix(float value) const
{
    char buffer [15];
    std::sprintf (buffer, "%10.7E", value);

    if (value == 0.0) {
        return " 0.0000000E+00";
    } else {
        if (std::isnan(value))
            return "NAN";

        if (std::isinf(value)) {
            if (value > 0)
                return "INF";
            else
                return "-INF";
        }

        std::string tmpstr(buffer);

        return tmpstr;
    }
}


std::string EclOutput::make_doub_string_ecl(double value) const
{
    char buffer [21 + 1];
    std::snprintf (buffer, sizeof buffer, "%19.13E", value);

    if (value == 0.0) {
        return "0.00000000000000D+00";
    } else {
        if (std::isnan(value))
            return "NAN";

        if (std::isinf(value)) {
            if (value > 0)
                return "INF";
            else
                return "-INF";
        }

        std::string tmpstr(buffer);
        int exp = value < 0.0 ? std::stoi(tmpstr.substr(17, 4)) : std::stoi(tmpstr.substr(16, 4));
        const bool use_exp_char = (exp >= -100) && (exp < 99);

        if (value < 0.0) {
            if (use_exp_char) {
                tmpstr = "-0." + tmpstr.substr(1, 1) + tmpstr.substr(3, 13) + "D";
            } else {
                tmpstr = "-0." + tmpstr.substr(1, 1) + tmpstr.substr(3, 13);
            }
        } else {
            if (use_exp_char) {
                tmpstr = "0." + tmpstr.substr(0, 1) + tmpstr.substr(2, 13) + "D";
            } else {
                tmpstr = "0." + tmpstr.substr(0, 1) + tmpstr.substr(2, 13);
            }
        }

        std::snprintf(buffer, sizeof buffer, "%+03i", exp + 1);
        tmpstr = tmpstr + buffer;
        return tmpstr;
    }
}

std::string EclOutput::make_doub_string_ix(double value) const
{
    char buffer [21];
    std::sprintf (buffer, "%19.13E", value);

    if (value == 0.0) {
        return " 0.0000000000000E+00";
    } else {
        if (std::isnan(value))
            return "NAN";

        if (std::isinf(value)) {
            if (value > 0)
                return "INF";
            else
                return "-INF";
        }

        std::string tmpstr(buffer);

        return tmpstr;
    }
}


template <typename T>
void EclOutput::writeFormattedArray(const std::vector<T>& data)
{
    int size = data.size();
    int n = 0;

    eclArrType arrType = MESS;
    if (typeid(T) == typeid(int)) {
        arrType = INTE;
    } else if (typeid(T) == typeid(float)) {
        arrType = REAL;
    } else if (typeid(T) == typeid(double)) {
        arrType = DOUB;
    } else if (typeid(T) == typeid(bool)) {
        arrType = LOGI;
    }


    auto sizeData = block_size_data_formatted(arrType);

    int maxBlockSize = std::get<0>(sizeData);
    int nColumns = std::get<1>(sizeData);
    int columnWidth = std::get<2>(sizeData);

    for (int i = 0; i < size; i++) {
        n++;

        switch (arrType) {
        case INTE:
            ofileH << std::setw(columnWidth) << data[i];
            break;
        case REAL:
            if (ix_standard)
                ofileH << std::setw(columnWidth) << make_real_string_ix(data[i]);
            else
                ofileH << std::setw(columnWidth) << make_real_string_ecl(data[i]);
            break;
        case DOUB:
            if (ix_standard)
                ofileH << std::setw(columnWidth) << make_doub_string_ix(data[i]);
            else
                ofileH << std::setw(columnWidth) << make_doub_string_ecl(data[i]);
            break;
        case LOGI:
            if (data[i]) {
                ofileH << "  T";
            } else {
                ofileH << "  F";
            }
            break;
        default:
            break;
        }

        if ((n % nColumns) == 0 || (n % maxBlockSize) == 0) {
            ofileH << std::endl;
        }

        if ((n % maxBlockSize) == 0) {
            n=0;
        }
    }

    if ((n % nColumns) != 0 && (n % maxBlockSize) != 0) {
        ofileH << std::endl;
    }
}


template void EclOutput::writeFormattedArray<int>(const std::vector<int>& data);
template void EclOutput::writeFormattedArray<float>(const std::vector<float>& data);
template void EclOutput::writeFormattedArray<double>(const std::vector<double>& data);
template void EclOutput::writeFormattedArray<bool>(const std::vector<bool>& data);
template void EclOutput::writeFormattedArray<char>(const std::vector<char>& data);


void EclOutput::writeFormattedCharArray(const std::vector<std::string>& data, int element_size)
{
    auto sizeData = block_size_data_formatted(CHAR);
    int maxBlockSize = std::get<0>(sizeData);

    int nColumns;

    if (element_size < 9)
    {
        element_size = 8;
        nColumns = std::get<1>(sizeData);
    } else
        nColumns = 80 / (element_size + 3);

    int rest = data.size();
    int n = 0;

    while (rest > 0) {
        int size = rest;

        if (size > maxBlockSize)
            size = maxBlockSize;

        for (int i = 0; i < size; i++) {
            std::string str1(element_size,' ');
            str1 = data[n] + std::string(element_size - data[n].size(),' ');

            n++;
            ofileH << " '" << str1 << "'";

            if ((i+1) % nColumns == 0) {
                ofileH  << std::endl;
            }
        }

        if ((size % nColumns) != 0) {
            ofileH  << std::endl;
        }

        rest = (rest > maxBlockSize) ? rest - maxBlockSize : 0;
    }
}


void EclOutput::writeFormattedCharArray(const std::vector<PaddedOutputString<8>>& data)
{
    const auto sizeData = block_size_data_formatted(CHAR);

    const int nColumns = std::get<1>(sizeData);

    const auto size = data.size();

    for (auto i = 0*size; i < size; ++i) {
        ofileH << " '" << data[i].c_str() << '\'';

        if ((i+1) % nColumns == 0) {
            ofileH << '\n';
        }
    }

    if ((size % nColumns) != 0) {
        ofileH << '\n';
    }
}

}} // namespace Opm::EclIO

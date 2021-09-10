/*
   Copyright 2019 Equinor ASA.

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

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclUtil.hpp>
#include <opm/common/ErrorMacros.hpp>

#include <fmt/format.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <numeric>
#include <cmath>

#include <iostream>


namespace Opm { namespace EclIO {

void EclFile::load(bool preload) {
    std::fstream fileH;

    if (formatted) {
        fileH.open(this->inputFilename, std::ios::in);
    } else {
        fileH.open(this->inputFilename, std::ios::in |  std::ios::binary);
    }

    if (!fileH)
        throw std::runtime_error(fmt::format("Can not open EclFile: {}", this->inputFilename));

    int n = 0;
    while (!isEOF(&fileH)) {
        std::string arrName(8,' ');
        eclArrType arrType;
        int64_t num;
        int sizeOfElement;

        if (formatted) {
            readFormattedHeader(fileH,arrName,num,arrType, sizeOfElement);
        } else {
            readBinaryHeader(fileH,arrName,num, arrType, sizeOfElement);
        }

        array_size.push_back(num);
        array_type.push_back(arrType);
        array_name.push_back(trimr(arrName));
        array_element_size.push_back(sizeOfElement);

        array_index[array_name[n]] = n;

        uint64_t pos = fileH.tellg();
        ifStreamPos.push_back(pos);

        arrayLoaded.push_back(false);

        if (num > 0){
            if (formatted) {
                uint64_t sizeOfNextArray = sizeOnDiskFormatted(num, arrType, sizeOfElement);
                fileH.seekg(static_cast<std::streamoff>(sizeOfNextArray), std::ios_base::cur);
            } else {
                uint64_t sizeOfNextArray = sizeOnDiskBinary(num, arrType, sizeOfElement);
                fileH.seekg(static_cast<std::streamoff>(sizeOfNextArray), std::ios_base::cur);
            }
        }

        n++;
    };

    fileH.seekg(0, std::ios_base::end);
    this->ifStreamPos.push_back(static_cast<uint64_t>(fileH.tellg()));
    fileH.close();

    if (preload)
        this->loadData();
}


EclFile::EclFile(const std::string& filename, EclFile::Formatted fmt, bool preload) :
    formatted(fmt.value),
    inputFilename(filename)
{
    this->load(preload);
}


EclFile::EclFile(const std::string& filename, bool preload) :
    inputFilename(filename)
{
    if (!fileExists(filename))
        throw std::runtime_error(fmt::format("Can not open EclFile: {}", filename));

    formatted = isFormatted(filename);
    this->load(preload);
}


void EclFile::loadBinaryArray(std::fstream& fileH, std::size_t arrIndex)
{
    fileH.seekg (ifStreamPos[arrIndex], fileH.beg);

    switch (array_type[arrIndex]) {
    case INTE:
        inte_array[arrIndex] = readBinaryInteArray(fileH, array_size[arrIndex]);
        break;
    case REAL:
        real_array[arrIndex] = readBinaryRealArray(fileH, array_size[arrIndex]);
        break;
    case DOUB:
        doub_array[arrIndex] = readBinaryDoubArray(fileH, array_size[arrIndex]);
        break;
    case LOGI:
        logi_array[arrIndex] = readBinaryLogiArray(fileH, array_size[arrIndex]);
        break;
    case CHAR:
        char_array[arrIndex] = readBinaryCharArray(fileH, array_size[arrIndex]);
        break;
    case C0NN:
        char_array[arrIndex] = readBinaryC0nnArray(fileH, array_size[arrIndex], array_element_size[arrIndex]);
        break;
    case MESS:
        break;
    default:
        OPM_THROW(std::runtime_error, "Asked to read unexpected array type");
        break;
    }

    arrayLoaded[arrIndex] = true;
}

void EclFile::loadFormattedArray(const std::string& fileStr, std::size_t arrIndex, int64_t fromPos)
{

    switch (array_type[arrIndex]) {
    case INTE:
        inte_array[arrIndex] = readFormattedInteArray(fileStr, array_size[arrIndex], fromPos);
        break;
    case REAL:
        real_array[arrIndex] = readFormattedRealArray(fileStr, array_size[arrIndex], fromPos);
        break;
    case DOUB:
        doub_array[arrIndex] = readFormattedDoubArray(fileStr, array_size[arrIndex], fromPos);
        break;
    case LOGI:
        logi_array[arrIndex] = readFormattedLogiArray(fileStr, array_size[arrIndex], fromPos);
        break;
    case CHAR:
        char_array[arrIndex] = readFormattedCharArray(fileStr, array_size[arrIndex], fromPos, sizeOfChar);
        break;
    case C0NN:
        char_array[arrIndex] = readFormattedCharArray(fileStr, array_size[arrIndex], fromPos, array_element_size[arrIndex]);
        break;
    case MESS:
        break;
    default:
        OPM_THROW(std::runtime_error, "Asked to read unexpected array type");
        break;
    }

    arrayLoaded[arrIndex] = true;
}


void EclFile::loadData()
{

    if (formatted) {

        std::vector<int> arrIndices(array_name.size());
        std::iota(arrIndices.begin(), arrIndices.end(), 0);

        this->loadData(arrIndices);

    } else {

        std::fstream fileH;
        fileH.open(inputFilename, std::ios::in |  std::ios::binary);

        if (!fileH) {
            std::string message="Could not open file: '" + inputFilename +"'";
            OPM_THROW(std::runtime_error, message);
        }

        for (size_t i = 0; i < array_name.size(); i++) {
            loadBinaryArray(fileH, i);
        }

        fileH.close();
    }
}


void EclFile::loadData(const std::string& name)
{

    if (formatted) {

        std::ifstream inFile(inputFilename);

        for (unsigned int arrIndex = 0; arrIndex < array_name.size(); arrIndex++) {

            if (array_name[arrIndex] == name) {

                inFile.seekg(ifStreamPos[arrIndex]);

                size_t size = sizeOnDiskFormatted(array_size[arrIndex], array_type[arrIndex], array_element_size[arrIndex])+1;
                std::vector<char> buffer(size);
                inFile.read (buffer.data(), size);

                std::string fileStr = std::string(buffer.data(), size);

                loadFormattedArray(fileStr, arrIndex, 0);
            }
        }

    } else {

        std::fstream fileH;
        fileH.open(inputFilename, std::ios::in |  std::ios::binary);

        if (!fileH) {
            std::string message="Could not open file: '" + inputFilename +"'";
            OPM_THROW(std::runtime_error, message);
        }

        for (size_t i = 0; i < array_name.size(); i++) {
            if (array_name[i] == name) {
                loadBinaryArray(fileH, i);
            }
        }

        fileH.close();
    }
}


void EclFile::loadData(const std::vector<int>& arrIndex)
{

    if (formatted) {

        std::ifstream inFile(inputFilename);

        for (int ind : arrIndex) {

            inFile.seekg(ifStreamPos[ind]);

            size_t size = sizeOnDiskFormatted(array_size[ind], array_type[ind], array_element_size[ind])+1;
            std::vector<char> buffer(size);
            inFile.read (buffer.data(), size);

            std::string fileStr = std::string(buffer.data(), size);

            loadFormattedArray(fileStr, ind, 0);
        }

    } else {
        std::fstream fileH;
        fileH.open(inputFilename, std::ios::in |  std::ios::binary);

        if (!fileH) {
            std::string message="Could not open file: '" + inputFilename +"'";
            OPM_THROW(std::runtime_error, message);
        }

        for (int ind : arrIndex) {
            loadBinaryArray(fileH, ind);
        }

        fileH.close();
    }
}


void EclFile::loadData(int arrIndex)
{
    if (formatted) {

        std::ifstream inFile(inputFilename);

            inFile.seekg(ifStreamPos[arrIndex]);

            size_t size = sizeOnDiskFormatted(array_size[arrIndex], array_type[arrIndex], array_element_size[arrIndex])+1;
            std::vector<char> buffer(size);
            inFile.read (buffer.data(), size);

            std::string fileStr = std::string(buffer.data(), size);

            loadFormattedArray(fileStr, arrIndex, 0);


    } else {
        std::fstream fileH;
        fileH.open(inputFilename, std::ios::in |  std::ios::binary);

        if (!fileH) {
            std::string message="Could not open file: '" + inputFilename +"'";
            OPM_THROW(std::runtime_error, message);
        }

        loadBinaryArray(fileH, arrIndex);

        fileH.close();
    }
}

bool EclFile::is_ix() const
{
    // assuming that array data type C0nn only are used in IX. This may change in future.

    // Formatted files,
    //   >> use real arrays. Example Ecl = '0.70000000E-01', IX = '7.0000000E-02'
    // Binary files,
    //   >> if logi array exists in file, look for IX spes binary representation of true value

    if (formatted) {
        for (size_t n=0; n < array_type.size(); n++) {
            if (array_type[n] == Opm::EclIO::C0NN) {
                return true;
            } else if (array_type[n] == Opm::EclIO::REAL) {
                auto realStr = get_fmt_real_raw_str_values(n);
                int p, first;

                for (auto val : realStr) {
                    double dtmpv = fabs(std::stod(val));

                    if (dtmpv > 0.0) {
                        p = val.find_first_of(".");
                        first = abs(std::stoi(val.substr(0, p)));

                        if (first > 0)
                            return true;
                        else
                            return false;
                    }
                }
            }
        }
    } else {
        for (size_t n=0; n < array_type.size(); n++) {
            if (array_type[n] == Opm::EclIO::C0NN) {
                return true;
            } else if (array_type[n] == Opm::EclIO::LOGI) {
                auto raw_logi_values = get_bin_logi_raw_values(n);
                for (unsigned int val : raw_logi_values) {
                    if (val == Opm::EclIO::true_value_ix)
                        return true;
                }
            }
        }

        return false;
    }

    return false;
}

std::vector<unsigned int> EclFile::get_bin_logi_raw_values(int arrIndex) const
{
    if (array_type[arrIndex] != Opm::EclIO::LOGI)
        OPM_THROW(std::runtime_error, "Error, selected array is not of type LOGI");

    std::fstream fileH;
    fileH.open(inputFilename, std::ios::in |  std::ios::binary);

    if (!fileH) {
        std::string message="Could not open file: '" + inputFilename +"'";
        OPM_THROW(std::runtime_error, message);
    }

    fileH.seekg (ifStreamPos[arrIndex], fileH.beg);

    std::vector<unsigned int> raw_logi = readBinaryRawLogiArray(fileH, array_size[arrIndex]);

    return raw_logi;
}

std::vector<std::string> EclFile::get_fmt_real_raw_str_values(int arrIndex) const
{
    std::vector<std::string> real_vect;

    if (array_type[arrIndex] != Opm::EclIO::REAL)
        OPM_THROW(std::runtime_error, "Error, selected array is not of type REAL");

    std::ifstream inFile(inputFilename);

    if (!inFile) {
        std::string message="Could not open file: '" + inputFilename +"'";
        OPM_THROW(std::runtime_error, message);
    }

    inFile.seekg(ifStreamPos[arrIndex]);

    size_t size = sizeOnDiskFormatted(array_size[arrIndex], array_type[arrIndex], array_element_size[arrIndex])+1;

    std::vector<char> buffer(size);
    inFile.read (buffer.data(), size);

    std::string fileStr = std::string(buffer.data(), size);

    std::vector<std::string> real_vect_str;
    real_vect_str = readFormattedRealRawStrings(fileStr, array_size[arrIndex], 0);

    return real_vect_str;
}


std::vector<EclFile::EclEntry> EclFile::getList() const
{
    std::vector<EclEntry> list;
    list.reserve(this->array_name.size());

    for (size_t i = 0; i < array_name.size(); i++) {
        list.emplace_back(array_name[i], array_type[i], array_size[i]);
    }

    return list;
}


template<>
const std::vector<int>& EclFile::get<int>(int arrIndex)
{
    return getImpl(arrIndex, INTE, inte_array, "integer");
}

template<>
const std::vector<float>&
EclFile::get<float>(int arrIndex)
{
    return getImpl(arrIndex, REAL, real_array, "float");
}


template<>
const std::vector<double> &EclFile::get<double>(int arrIndex)
{
    return getImpl(arrIndex, DOUB, doub_array, "double");
}


template<>
const std::vector<bool>& EclFile::get<bool>(int arrIndex)
{
    return getImpl(arrIndex, LOGI, logi_array, "bool");
}


template<>
const std::vector<std::string>& EclFile::get<std::string>(int arrIndex)
{
    if ((array_type[arrIndex] != Opm::EclIO::C0NN) && (array_type[arrIndex] != Opm::EclIO::CHAR)){
        std::string message = "Array with index " + std::to_string(arrIndex) + " is not of type " + "std::string";
        OPM_THROW(std::runtime_error, message);
    }

    return getImpl(arrIndex, array_type[arrIndex], char_array, "string");
}


bool EclFile::hasKey(const std::string &name) const
{
    auto search = array_index.find(name);
    return search != array_index.end();
}

size_t EclFile::count(const std::string &name) const
{
    return std::count (array_name.begin(), array_name.end(), name);
}


std::streampos
EclFile::seekPosition(const std::vector<std::string>::size_type arrIndex) const
{
    if (arrIndex >= this->array_name.size()) {
        return { static_cast<std::streamoff>(this->ifStreamPos.back()) };
    }

    // StreamPos is file position of start of data vector's control
    // character (unformatted) or data items (formatted).  We need
    // file position of start of header, because that's where we're
    // supposed to start writing new data.  Subtract header size.
    //
    //   (*) formatted header size = 30 characters =
    //             1 space character
    //          +  1 quote character
    //          +  8 character header/vector name
    //          +  1 quote character
    //          +  1 space character
    //          + 11 characters for number of elments
    //          +  1 space character
    //          +  1 quote character
    //          +  4 characters for element type
    //          +  1 quote character
    //
    //
    //   (*) unformatted header size = 24 bytes =
    //            4 byte => size of header in byte
    //          + 8 byte header/vector name
    //          + 4 byte for number of elements
    //          + 4 byte for element type
    //          + 4 byte size of header in byte
    //
    //      16 byte header (8+4+4)
    //
    //       +------+------------+------+------+------+
    //       | Ctrl | Keyword    | #elm | type | Ctrl |  (item)
    //       |  4   |  8         |  4   |  4   |  4   |  (#bytes)
    //       +------+------------+------+------+------+
    //
    //      20 byte header (8+8+4)
    //
    //       +------+------------+------+------+------+
    //       | Ctrl | Keyword    | #elm | type | Ctrl |  (item)
    //       |  4   |  8         |  8   |  4   |  4   |  (#bytes)
    //       +------+------------+------+------+------+
    //

    const auto headerSize = this->formatted ? 30ul : 24ul;
    const auto datapos    = this->ifStreamPos[arrIndex];
    const auto seekpos    = (datapos <= headerSize)
        ? 0ul : datapos - headerSize;

    return { static_cast<std::streamoff>(seekpos) };
}

template<>
const std::vector<int>& EclFile::get<int>(const std::string& name)
{
    auto search = array_index.find(name);

    if (search == array_index.end()) {
        std::string message="key '"+name + "' not found";
        OPM_THROW(std::invalid_argument, message);
    }

    return getImpl(search->second, INTE, inte_array, "integer");
}

template<>
const std::vector<float>& EclFile::get<float>(const std::string& name)
{
    auto search = array_index.find(name);

    if (search == array_index.end()) {
        std::string message="key '"+name + "' not found";
        OPM_THROW(std::invalid_argument, message);
    }

    return getImpl(search->second, REAL, real_array, "float");
}


template<>
const std::vector<double>& EclFile::get<double>(const std::string &name)
{
    auto search = array_index.find(name);

    if (search == array_index.end()) {
        std::string message="key '"+name + "' not found";
        OPM_THROW(std::invalid_argument, message);
    }

    return getImpl(search->second, DOUB, doub_array, "double");
}


template<>
const std::vector<bool>& EclFile::get<bool>(const std::string &name)
{
    auto search = array_index.find(name);

    if (search == array_index.end()) {
        std::string message="key '"+name + "' not found";
        OPM_THROW(std::invalid_argument, message);
    }

    return getImpl(search->second, LOGI, logi_array, "bool");
}


template<>
const std::vector<std::string>& EclFile::get<std::string>(const std::string &name)
{
    auto search = array_index.find(name);

    if (search == array_index.end()) {
        std::string message="key '"+name + "' not found";
        OPM_THROW(std::invalid_argument, message);
    }

    if ((array_type[search->second] != Opm::EclIO::C0NN) && (array_type[search->second] != Opm::EclIO::CHAR)){
        std::string message = "Array with index " + std::to_string(search->second) + " is not of type " + "std::string";
        OPM_THROW(std::runtime_error, message);
    }

    return getImpl(search->second, array_type[search->second], char_array, "string");
}


template<class T>
const std::vector<T>& EclFile::getImpl(int arrIndex, eclArrType type,
                                       const std::unordered_map<int, std::vector<T>>& array,
                                       const std::string& typeStr)
{
    if (array_type[arrIndex] != type) {
        std::string message = "Array with index " + std::to_string(arrIndex) + " is not of type " + typeStr;
        OPM_THROW(std::runtime_error, message);
    }

    if (!arrayLoaded[arrIndex]) {
        loadData(arrIndex);
    }

    return array.at(arrIndex);
}


std::size_t EclFile::size() const {
    return this->array_name.size();
}


}} // namespace Opm::ecl

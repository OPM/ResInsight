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

// anonymous namespace for EclFile

namespace {

bool fileExists(const std::string& filename){

    std::ifstream fileH(filename.c_str());
    return fileH.good();
}

bool isFormatted(const std::string& filename)
{
    const auto p = filename.find_last_of(".");
    if (p == std::string::npos)
      OPM_THROW(std::invalid_argument,
                "Purported ECLIPSE Filename'" + filename + "'does not contain extension");
    return std::strchr("ABCFGH", static_cast<int>(filename[p+1])) != nullptr;
}


template<typename T, typename T2>
std::vector<T> readBinaryArray(std::fstream& fileH, const int64_t size, Opm::EclIO::eclArrType type,
                               std::function<T(T2)>& flip)
{
    std::vector<T> arr;

    auto sizeData = block_size_data_binary(type);
    int sizeOfElement = std::get<0>(sizeData);
    int maxBlockSize = std::get<1>(sizeData);
    int maxNumberOfElements = maxBlockSize / sizeOfElement;

    arr.reserve(size);

    int64_t rest = size;
    while (rest > 0) {
        int dhead;
        fileH.read(reinterpret_cast<char*>(&dhead), sizeof(dhead));
        dhead = Opm::EclIO::flipEndianInt(dhead);

        int num = dhead / sizeOfElement;

        if ((num > maxNumberOfElements) || (num < 0)) {
            OPM_THROW(std::runtime_error, "Error reading binary data, inconsistent header data or incorrect number of elements");
        }

        for (int i = 0; i < num; i++) {
            T2 value;
            fileH.read(reinterpret_cast<char*>(&value), sizeOfElement);
            arr.push_back(flip(value));
        }

        rest -= num;

        if (( num < maxNumberOfElements && rest != 0) ||
            (num == maxNumberOfElements && rest < 0)) {
            std::string message = "Error reading binary data, incorrect number of elements";
            OPM_THROW(std::runtime_error, message);
        }

        int dtail;
        fileH.read(reinterpret_cast<char*>(&dtail), sizeof(dtail));
        dtail = Opm::EclIO::flipEndianInt(dtail);

        if (dhead != dtail) {
            OPM_THROW(std::runtime_error, "Error reading binary data, tail not matching header.");
        }
    }

    return arr;
}


std::vector<int> readBinaryInteArray(std::fstream &fileH, const int64_t size)
{
    std::function<int(int)> f = Opm::EclIO::flipEndianInt;
    return readBinaryArray<int,int>(fileH, size, Opm::EclIO::INTE, f);
}


std::vector<float> readBinaryRealArray(std::fstream& fileH, const int64_t size)
{
    std::function<float(float)> f = Opm::EclIO::flipEndianFloat;
    return readBinaryArray<float,float>(fileH, size, Opm::EclIO::REAL, f);
}


std::vector<double> readBinaryDoubArray(std::fstream& fileH, const int64_t size)
{
    std::function<double(double)> f = Opm::EclIO::flipEndianDouble;
    return readBinaryArray<double,double>(fileH, size, Opm::EclIO::DOUB, f);
}

std::vector<bool> readBinaryLogiArray(std::fstream &fileH, const int64_t size)
{
    std::function<bool(unsigned int)> f = [](unsigned int intVal)
                                          {
                                              bool value;
                                              if (intVal == Opm::EclIO::true_value) {
                                                  value = true;
                                              } else if (intVal == Opm::EclIO::false_value) {
                                                  value = false;
                                              } else {
                                                  OPM_THROW(std::runtime_error, "Error reading logi value");
                                              }

                                              return value;
                                          };
    return readBinaryArray<bool,unsigned int>(fileH, size, Opm::EclIO::LOGI, f);
}


std::vector<std::string> readBinaryCharArray(std::fstream& fileH, const int64_t size)
{
    using Char8 = std::array<char, 8>;
    std::function<std::string(Char8)> f = [](const Char8& val)
                                          {
                                              std::string res(val.begin(), val.end());
                                              return Opm::EclIO::trimr(res);
                                          };
    return readBinaryArray<std::string,Char8>(fileH, size, Opm::EclIO::CHAR, f);
}


template<typename T>
std::vector<T> readFormattedArray(const std::string& file_str, const int size, int64_t fromPos,
                                 std::function<T(const std::string&)>& process)
{
    std::vector<T> arr;

    arr.reserve(size);

    int64_t p1=fromPos;

    for (int i=0; i< size; i++) {
        p1 = file_str.find_first_not_of(' ',p1);
        int64_t p2 = file_str.find_first_of(' ', p1);

        arr.push_back(process(file_str.substr(p1, p2-p1)));

        p1 = file_str.find_first_not_of(' ',p2);
    }

    return arr;

}


std::vector<int> readFormattedInteArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{

    std::function<int(const std::string&)> f = [](const std::string& val)
                                               {
                                                   return std::stoi(val);
                                               };

    return readFormattedArray(file_str, size, fromPos, f);
}


std::vector<std::string> readFormattedCharArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{
    std::vector<std::string> arr;
    arr.reserve(size);

    int64_t p1=fromPos;

    for (int i=0; i< size; i++) {
        p1 = file_str.find_first_of('\'',p1);
        std::string value = file_str.substr(p1 + 1, 8);

        if (value == "        ") {
            arr.push_back("");
        } else {
            arr.push_back(Opm::EclIO::trimr(value));
        }

        p1 = p1+10;
    }

    return arr;
}


std::vector<float> readFormattedRealArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{

    std::function<float(const std::string&)> f = [](const std::string& val)
                                                 {
                                                     // tskille: temporary fix, need to be discussed. OPM flow writes numbers
                                                     // that are outside valid range for float, and function stof will fail
                                                     double dtmpv = std::stod(val);
                                                     return dtmpv;
                                                 };

    return readFormattedArray<float>(file_str, size, fromPos, f);
}


std::vector<bool> readFormattedLogiArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{

    std::function<bool(const std::string&)> f = [](const std::string& val)
                                                {
                                                    if (val[0] == 'T') {
                                                        return true;
                                                    } else if (val[0] == 'F') {
                                                        return false;
                                                    } else {
                                                        std::string message="Could not convert '" + val + "' to a bool value ";
                                                        OPM_THROW(std::invalid_argument, message);
                                                    }
                                                };

    return readFormattedArray<bool>(file_str, size, fromPos, f);
}

std::vector<double> readFormattedDoubArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{

    std::function<double(const std::string&)> f = [](std::string val)
                                                  {
                                                      auto p1 = val.find_first_of("D");

                                                      if (p1 == std::string::npos) {
                                                          auto p2 = val.find_first_of("-+", 1);
                                                          if (p2 != std::string::npos) {
                                                              val = val.insert(p2,"E");
                                                          }
                                                      } else {
                                                          val.replace(p1,1,"E");
                                                      }

                                                      return std::stod(val);
                                                  };

    return readFormattedArray<double>(file_str, size, fromPos, f);
}

} // anonymous namespace

// ==========================================================================

namespace Opm { namespace EclIO {

EclFile::EclFile(const std::string& filename, bool preload) : inputFilename(filename)
{
    if (!fileExists(filename)){
        std::string message="Could not open EclFile: " + filename;
        OPM_THROW(std::invalid_argument, message);
    }

    std::fstream fileH;

    formatted = isFormatted(filename);

    if (formatted) {
        fileH.open(filename, std::ios::in);
    } else {
        fileH.open(filename, std::ios::in |  std::ios::binary);
    }

    if (!fileH) {
        std::string message="Could not open file: " + filename;
        OPM_THROW(std::runtime_error, message);
    }

    int n = 0;
    while (!isEOF(&fileH)) {
        std::string arrName(8,' ');
        eclArrType arrType;
        int64_t num;
        
        if (formatted) {
            readFormattedHeader(fileH,arrName,num,arrType);
        } else {
            readBinaryHeader(fileH,arrName,num,arrType);
        }

        array_size.push_back(num);
        array_type.push_back(arrType);

        array_name.push_back(trimr(arrName));
        array_index[array_name[n]] = n;

        uint64_t pos = fileH.tellg();
        ifStreamPos.push_back(pos);

        arrayLoaded.push_back(false);

        if (num > 0){ 
            if (formatted) {
                uint64_t sizeOfNextArray = sizeOnDiskFormatted(num, arrType);
                fileH.seekg(static_cast<std::streamoff>(sizeOfNextArray), std::ios_base::cur);
            } else {
                uint64_t sizeOfNextArray = sizeOnDiskBinary(num, arrType);
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
        char_array[arrIndex] = readFormattedCharArray(fileStr, array_size[arrIndex], fromPos);
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

                char* buffer;
                size_t size = sizeOnDiskFormatted(array_size[arrIndex], array_type[arrIndex])+1;
                buffer = new char [size];
                inFile.read (buffer, size);

                std::string fileStr = std::string(buffer, size);

                loadFormattedArray(fileStr, arrIndex, 0);

                delete[] buffer;
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

            char* buffer;
            size_t size = sizeOnDiskFormatted(array_size[ind], array_type[ind])+1;
            buffer = new char [size];
            inFile.read (buffer, size);

            std::string fileStr = std::string(buffer, size);

            loadFormattedArray(fileStr, ind, 0);

            delete[] buffer;
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

            char* buffer;
            size_t size = sizeOnDiskFormatted(array_size[arrIndex], array_type[arrIndex])+1;
            buffer = new char [size];
            inFile.read (buffer, size);

            std::string fileStr = std::string(buffer, size);

            loadFormattedArray(fileStr, arrIndex, 0);

            delete[] buffer;


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
    return getImpl(arrIndex, CHAR, char_array, "string");
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

    return getImpl(search->second, CHAR, char_array, "string");
}


std::size_t EclFile::size() const {
    return this->array_name.size();
}


}} // namespace Opm::ecl

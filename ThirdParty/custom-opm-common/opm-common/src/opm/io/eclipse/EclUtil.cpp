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

#include <opm/io/eclipse/EclUtil.hpp>

#include <opm/common/ErrorMacros.hpp>

#include <cross-platform/PortableEndian.hpp>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <cstring>

//temporary
#include <iostream>

int Opm::EclIO::flipEndianInt(int num)
{
    unsigned int tmp = __builtin_bswap32(num);
    return static_cast<int>(tmp);
}

int64_t Opm::EclIO::flipEndianLongInt(int64_t num)
{
    uint64_t tmp = __builtin_bswap64(num);
    return static_cast<int64_t>(tmp);
}

float Opm::EclIO::flipEndianFloat(float num)
{
    float value = num;

    char* floatToConvert = reinterpret_cast<char*>(&value);
    std::reverse(floatToConvert, floatToConvert+4);

    return value;
}


double Opm::EclIO::flipEndianDouble(double num)
{
    double value = num;

    char* doubleToConvert = reinterpret_cast<char*>(&value);
    std::reverse(doubleToConvert, doubleToConvert+8);

    return value;
}

bool Opm::EclIO::fileExists(const std::string& filename){

    std::ifstream fileH(filename.c_str());
    return fileH.good();
}


bool Opm::EclIO::is_number(const std::string& numstr)
{
    for (char const &c : numstr)
        if (std::isdigit(c) == 0)
            return false;

    return true;
}


bool Opm::EclIO::isFormatted(const std::string& filename)
{
    const auto p = filename.find_last_of(".");
    if (p == std::string::npos)
      OPM_THROW(std::invalid_argument,
                "Purported ECLIPSE Filename'" + filename + "'does not contain extension");
    return std::strchr("ABCFGH", static_cast<int>(filename[p+1])) != nullptr;
}

bool Opm::EclIO::isEOF(std::fstream* fileH)
{
    int num;
    int64_t pos = fileH->tellg();
    fileH->read(reinterpret_cast<char*>(&num), sizeof(num));

    if (fileH->eof()) {
        return true;
    } else {
        fileH->seekg (pos);
        return false;
    }
}


std::tuple<int, int> Opm::EclIO::block_size_data_binary(eclArrType arrType)
{
    using BlockSizeTuple = std::tuple<int, int>;

    switch (arrType) {
    case INTE:
        return BlockSizeTuple{sizeOfInte, MaxBlockSizeInte};
        break;
    case REAL:
        return BlockSizeTuple{sizeOfReal, MaxBlockSizeReal};
        break;
    case DOUB:
        return BlockSizeTuple{sizeOfDoub, MaxBlockSizeDoub};
        break;
    case LOGI:
        return BlockSizeTuple{sizeOfLogi, MaxBlockSizeLogi};
        break;
    case CHAR:
        return BlockSizeTuple{sizeOfChar, MaxBlockSizeChar};
        break;
    case C0NN:
        return BlockSizeTuple{sizeOfChar, MaxBlockSizeChar};
        break;
    case MESS:
        OPM_THROW(std::invalid_argument, "Type 'MESS' have no associated data");
        break;
    default:
        OPM_THROW(std::invalid_argument, "Unknown field type");
        break;
    }
}


std::tuple<int, int, int> Opm::EclIO::block_size_data_formatted(eclArrType arrType)
{
    using BlockSizeTuple = std::tuple<int, int, int>;

    switch (arrType) {
    case INTE:
        return BlockSizeTuple{MaxNumBlockInte, numColumnsInte, columnWidthInte};
        break;
    case REAL:
        return BlockSizeTuple{MaxNumBlockReal,numColumnsReal, columnWidthReal};
        break;
    case DOUB:
        return BlockSizeTuple{MaxNumBlockDoub,numColumnsDoub, columnWidthDoub};
        break;
    case LOGI:
        return BlockSizeTuple{MaxNumBlockLogi,numColumnsLogi, columnWidthLogi};
        break;
    case CHAR:
        return BlockSizeTuple{MaxNumBlockChar,numColumnsChar, columnWidthChar};
        break;
    case C0NN:
        return BlockSizeTuple{MaxNumBlockChar,numColumnsChar, columnWidthChar};
        break;
    case MESS:
        OPM_THROW(std::invalid_argument, "Type 'MESS' have no associated data") ;
        break;
    default:
        OPM_THROW(std::invalid_argument, "Unknown field type");
        break;
    }
}


std::string Opm::EclIO::trimr(const std::string &str1)
{
    if (str1 == "        ") {
        return "";
    } else {
        int p = str1.find_last_not_of(" ");

        return str1.substr(0,p+1);
    }
}

uint64_t Opm::EclIO::sizeOnDiskBinary(int64_t num, Opm::EclIO::eclArrType arrType, int elementSize)
{
    uint64_t size = 0;

    if (arrType == Opm::EclIO::MESS) {
        if (num > 0) {
            std::string message = "In routine calcSizeOfArray, type MESS can not have size > 0";
            OPM_THROW(std::invalid_argument, message);
        }
    } else {
        if (num > 0) {
            auto sizeData = Opm::EclIO::block_size_data_binary(arrType);

            if (arrType == Opm::EclIO::C0NN){
                std::get<1>(sizeData)= std::get<1>(sizeData) / std::get<0>(sizeData) * elementSize;
                std::get<0>(sizeData) = elementSize;
            }

            int sizeOfElement = std::get<0>(sizeData);
            int maxBlockSize = std::get<1>(sizeData);
            int maxNumberOfElements = maxBlockSize / sizeOfElement;

            auto numBlocks = static_cast<uint64_t>(num)/static_cast<uint64_t>(maxNumberOfElements);
            auto rest = static_cast<uint64_t>(num) - numBlocks*static_cast<uint64_t>(maxNumberOfElements);

            auto size2Inte = static_cast<uint64_t>(Opm::EclIO::sizeOfInte) * 2;
            auto sizeFullBlocks = numBlocks * (static_cast<uint64_t>(maxBlockSize) + size2Inte);

            uint64_t sizeLastBlock = 0;

            if (rest > 0)
                sizeLastBlock = rest * static_cast<uint64_t>(sizeOfElement) + size2Inte;

            size = sizeFullBlocks + sizeLastBlock;
        }
    }

    return size;
}

uint64_t Opm::EclIO::sizeOnDiskFormatted(const int64_t num, Opm::EclIO::eclArrType arrType, int elementSize)
{
    uint64_t size = 0;

    if (arrType == Opm::EclIO::MESS) {
        if (num > 0) {
            OPM_THROW(std::invalid_argument, "In routine calcSizeOfArray, type MESS can not have size > 0");
        }
    } else {
        auto sizeData = block_size_data_formatted(arrType);

        if (arrType == Opm::EclIO::C0NN){
            std::get<2>(sizeData) = elementSize + 3;
            std::get<1>(sizeData) = 80 / std::get<2>(sizeData);
        }

        int maxBlockSize = std::get<0>(sizeData);
        int nColumns = std::get<1>(sizeData);
        int columnWidth = std::get<2>(sizeData);

        int nBlocks = num /maxBlockSize;
        int sizeOfLastBlock = num %  maxBlockSize;

        size = 0;

        if (nBlocks > 0) {
            int nLinesBlock = maxBlockSize / nColumns;
            int rest = maxBlockSize % nColumns;

            if (rest > 0) {
                nLinesBlock++;
            }

            int64_t blockSize = maxBlockSize * columnWidth + nLinesBlock;
            size = nBlocks * blockSize;
        }

        int nLines = sizeOfLastBlock / nColumns;
        int rest = sizeOfLastBlock % nColumns;

        size = size + sizeOfLastBlock * columnWidth + nLines;

        if (rest > 0) {
            size++;
        }
    }

    return size;
}

void Opm::EclIO::readBinaryHeader(std::fstream& fileH, std::string& tmpStrName,
                      int& tmpSize, std::string& tmpStrType)
{
    int bhead;

    fileH.read(reinterpret_cast<char*>(&bhead), sizeof(bhead));
    bhead = Opm::EclIO::flipEndianInt(bhead);

    if (bhead != 16){
        std::string message="Error reading binary header. Expected 16 bytes of header data, found " + std::to_string(bhead);
        OPM_THROW(std::runtime_error, message);
    }

    fileH.read(&tmpStrName[0], 8);

    fileH.read(reinterpret_cast<char*>(&tmpSize), sizeof(tmpSize));
    tmpSize = Opm::EclIO::flipEndianInt(tmpSize);

    fileH.read(&tmpStrType[0], 4);

    fileH.read(reinterpret_cast<char*>(&bhead), sizeof(bhead));
    bhead = Opm::EclIO::flipEndianInt(bhead);

    if (bhead != 16){
        std::string message="Error reading binary header. Expected 16 bytes of header data, found " + std::to_string(bhead);
        OPM_THROW(std::runtime_error, message);
    }
}

void Opm::EclIO::readBinaryHeader(std::fstream& fileH, std::string& arrName,
                      int64_t& size, Opm::EclIO::eclArrType &arrType, int& elementSize)
{
    std::string tmpStrName(8,' ');
    std::string tmpStrType(4,' ');
    int tmpSize;

    readBinaryHeader(fileH, tmpStrName, tmpSize, tmpStrType);

    if (tmpStrType == "X231"){
        std::string x231ArrayName = tmpStrName;
        int x231exp = tmpSize * (-1);

        readBinaryHeader(fileH, tmpStrName, tmpSize, tmpStrType);

        if (x231ArrayName != tmpStrName)
            OPM_THROW(std::runtime_error, "Invalied X231 header, name should be same in both headers'");

        if (x231exp < 0)
            OPM_THROW(std::runtime_error, "Invalied X231 header, size of array should be negative'");

        size = static_cast<int64_t>(tmpSize) + static_cast<int64_t>(x231exp) * pow(2,31);
    } else {
        size = static_cast<int64_t>(tmpSize);
    }

    elementSize = 4;

    arrName = tmpStrName;
    if (tmpStrType == "INTE")
        arrType = Opm::EclIO::INTE;
    else if (tmpStrType == "REAL")
        arrType = Opm::EclIO::REAL;
    else if (tmpStrType == "DOUB"){
        arrType = Opm::EclIO::DOUB;
        elementSize = 8;
    }
    else if (tmpStrType == "CHAR"){
        arrType = Opm::EclIO::CHAR;
        elementSize = 8;
    }
    else if (tmpStrType.substr(0,1)=="C"){
        arrType = Opm::EclIO::C0NN;
        elementSize = std::stoi(tmpStrType.substr(1,3));
    }
    else if (tmpStrType =="LOGI")
        arrType = Opm::EclIO::LOGI;
    else if (tmpStrType == "MESS")
        arrType = Opm::EclIO::MESS;
    else
        OPM_THROW(std::runtime_error, "Error, unknown array type '" + tmpStrType +"'");
}


void Opm::EclIO::readFormattedHeader(std::fstream& fileH, std::string& arrName,
                         int64_t &num, Opm::EclIO::eclArrType &arrType, int& elementSize)
{
    std::string line;
    std::getline(fileH,line);

    int p1 = line.find_first_of("'");
    int p2 = line.find_first_of("'",p1+1);
    int p3 = line.find_first_of("'",p2+1);
    int p4 = line.find_first_of("'",p3+1);

    if (p1 == -1 || p2 == -1 || p3 == -1 || p4 == -1) {
        OPM_THROW(std::runtime_error, "Header name and type should be enclosed with '");
    }

    arrName = line.substr(p1 + 1, p2 - p1 - 1);
    std::string antStr = line.substr(p2 + 1, p3 - p2 - 1);
    std::string arrTypeStr = line.substr(p3 + 1, p4 - p3 - 1);

    num = std::stol(antStr);

    elementSize = 4;

    if (arrTypeStr == "INTE")
        arrType = Opm::EclIO::INTE;
    else if (arrTypeStr == "REAL")
        arrType = Opm::EclIO::REAL;
    else if (arrTypeStr == "DOUB"){
        arrType = Opm::EclIO::DOUB;
        elementSize = 8;
    }
    else if (arrTypeStr == "CHAR"){
        arrType = Opm::EclIO::CHAR;
        elementSize = 8;
    }
    else if (arrTypeStr.substr(0,1)=="C"){
        arrType = Opm::EclIO::C0NN;
        elementSize = std::stoi(arrTypeStr.substr(1,3));
    }
    else if (arrTypeStr == "LOGI")
        arrType = Opm::EclIO::LOGI;
    else if (arrTypeStr == "MESS")
        arrType = Opm::EclIO::MESS;
    else
        OPM_THROW(std::runtime_error, "Error, unknown array type '" + arrTypeStr +"'");

    if (arrName.size() != 8) {
        OPM_THROW(std::runtime_error, "Header name should be 8 characters");
    }
}

template<typename T, typename T2>
std::vector<T> Opm::EclIO::readBinaryArray(std::fstream& fileH, const int64_t size, Opm::EclIO::eclArrType type,
                               std::function<T(T2)>& flip, int elementSize)
{
    std::vector<T> arr;

    auto sizeData = block_size_data_binary(type);

    if (type == Opm::EclIO::C0NN){
        std::get<1>(sizeData)= std::get<1>(sizeData) / std::get<0>(sizeData) * elementSize;
        std::get<0>(sizeData) = elementSize;
    }

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

            if constexpr (std::is_same_v<T2, std::string>) {
                value.resize(sizeOfElement) ;
                fileH.read(&value[0], sizeOfElement);
            } else
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


std::vector<int> Opm::EclIO::readBinaryInteArray(std::fstream &fileH, const int64_t size)
{
    std::function<int(int)> f = Opm::EclIO::flipEndianInt;
    return readBinaryArray<int,int>(fileH, size, Opm::EclIO::INTE, f, sizeOfInte);
}


std::vector<float> Opm::EclIO::readBinaryRealArray(std::fstream& fileH, const int64_t size)
{
    std::function<float(float)> f = Opm::EclIO::flipEndianFloat;
    return readBinaryArray<float,float>(fileH, size, Opm::EclIO::REAL, f, sizeOfReal);
}


std::vector<double> Opm::EclIO::readBinaryDoubArray(std::fstream& fileH, const int64_t size)
{
    std::function<double(double)> f = Opm::EclIO::flipEndianDouble;
    return readBinaryArray<double,double>(fileH, size, Opm::EclIO::DOUB, f, sizeOfDoub);
}

std::vector<bool> Opm::EclIO::readBinaryLogiArray(std::fstream &fileH, const int64_t size)
{
    std::function<bool(unsigned int)> f = [](unsigned int intVal)
                                          {
                                              bool value;
                                              if (intVal == Opm::EclIO::true_value_ecl) {
                                                  value = true;
                                              } else if (intVal == Opm::EclIO::false_value) {
                                                  value = false;
                                              } else if (intVal == Opm::EclIO::true_value_ix) {
                                                  value = true;
                                              } else {
                                                  OPM_THROW(std::runtime_error, "Error reading logi value");
                                              }

                                              return value;
                                          };
    return readBinaryArray<bool,unsigned int>(fileH, size, Opm::EclIO::LOGI, f, sizeOfLogi);
}

std::vector<unsigned int> Opm::EclIO::readBinaryRawLogiArray(std::fstream &fileH, const int64_t size)
{
    std::function<unsigned int(unsigned int)> f = [](unsigned int intVal)
                                          {
                                              return intVal;
                                          };
    return readBinaryArray<unsigned int, unsigned int>(fileH, size, Opm::EclIO::LOGI, f, sizeOfLogi);
}


std::vector<std::string> Opm::EclIO::readBinaryCharArray(std::fstream& fileH, const int64_t size)
{
    using Char8 = std::array<char, 8>;
    std::function<std::string(Char8)> f = [](const Char8& val)
                                          {
                                              std::string res(val.begin(), val.end());
                                              return Opm::EclIO::trimr(res);
                                          };
    return readBinaryArray<std::string,Char8>(fileH, size, Opm::EclIO::CHAR, f, sizeOfChar);
}


std::vector<std::string> Opm::EclIO::readBinaryC0nnArray(std::fstream& fileH, const int64_t size, int elementSize)
{
    std::function<std::string(std::string)> f = [](const std::string& val)
                                          {
                                              return Opm::EclIO::trimr(val);
                                          };

    return readBinaryArray<std::string,std::string>(fileH, size, Opm::EclIO::C0NN, f, elementSize);
}


template<typename T>
std::vector<T> Opm::EclIO::readFormattedArray(const std::string& file_str, const int size, int64_t fromPos,
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


std::vector<int> Opm::EclIO::readFormattedInteArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{

    std::function<int(const std::string&)> f = [](const std::string& val)
                                               {
                                                   return std::stoi(val);
                                               };

    return readFormattedArray(file_str, size, fromPos, f);
}


std::vector<std::string> Opm::EclIO::readFormattedCharArray(const std::string& file_str, const int64_t size,
                                                            int64_t fromPos, int elementSize)
{
    std::vector<std::string> arr;
    arr.reserve(size);

    int64_t p1=fromPos;

    for (int i=0; i< size; i++) {
        p1 = file_str.find_first_of('\'',p1);
        std::string value = file_str.substr(p1 + 1, elementSize);

        if (value == "        ") {
            arr.push_back("");
        } else {
            arr.push_back(Opm::EclIO::trimr(value));
        }

        p1 = p1 + elementSize + 2;
    }

    return arr;
}


std::vector<float> Opm::EclIO::readFormattedRealArray(const std::string& file_str, const int64_t size, int64_t fromPos)
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

std::vector<std::string> Opm::EclIO::readFormattedRealRawStrings(const std::string& file_str, const int64_t size, int64_t fromPos)
{


    std::function<std::string(const std::string&)> f = [](const std::string& val)
                                                 {
                                                     return val;
                                                 };

    return readFormattedArray<std::string>(file_str, size, fromPos, f);
}


std::vector<bool> Opm::EclIO::readFormattedLogiArray(const std::string& file_str, const int64_t size, int64_t fromPos)
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

std::vector<double> Opm::EclIO::readFormattedDoubArray(const std::string& file_str, const int64_t size, int64_t fromPos)
{

    std::function<double(const std::string&)> f = [](std::string val)
                                                  {
                                                      auto p1 = val.find_first_of("D");

                                                      if (p1 != std::string::npos) {
                                                          val.replace(p1,1,"E");
                                                      }

                                                      p1 = val.find_first_of("E");

                                                      if (p1 == std::string::npos) {
                                                          auto p2 = val.find_first_of("-+", 1);

                                                          if (p2 != std::string::npos)
                                                              val = val.insert(p2,"E");
                                                      }
                                                      return std::stod(val);
                                                  };

    return readFormattedArray<double>(file_str, size, fromPos, f);
}


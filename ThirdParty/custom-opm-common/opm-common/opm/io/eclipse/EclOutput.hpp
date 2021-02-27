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
#ifndef OPM_IO_ECLOUTPUT_HPP
#define OPM_IO_ECLOUTPUT_HPP

#include <fstream>
#include <ios>
#include <string>
#include <typeinfo>
#include <vector>

#include <opm/io/eclipse/EclIOdata.hpp>
#include <opm/io/eclipse/PaddedOutputString.hpp>
#include <iostream>

namespace Opm { namespace EclIO { namespace OutputStream {
    class Restart;
    class SummarySpecification;
}}}

namespace Opm { namespace EclIO {

class EclOutput
{
public:
    EclOutput(const std::string&            filename,
              const bool                    formatted,
              const std::ios_base::openmode mode = std::ios::out);

    template<typename T>
    void write(const std::string& name,
               const std::vector<T>& data)
    {
        eclArrType arrType = MESS;
        int element_size = 4;

        if (typeid(T) == typeid(int))
            arrType = INTE;
        else if (typeid(T) == typeid(float))
            arrType = REAL;
        else if (typeid(T) == typeid(double)){
            arrType = DOUB;
            element_size = 8;
        } else if (typeid(T) == typeid(bool))
            arrType = LOGI;
        else if (typeid(T) == typeid(char))
            arrType = MESS;

        if (isFormatted)
        {
            writeFormattedHeader(name, data.size(), arrType, element_size);
            if (arrType != MESS)
                writeFormattedArray(data);
        }
        else
        {
            writeBinaryHeader(name, data.size(), arrType, element_size);
            if (arrType != MESS)
                writeBinaryArray(data);
        }
    }

    // when this function is used array type will be assumed C0NN (not CHAR).
    // Also in cases where element size is 8 or less, element size will be 8.

    void write(const std::string& name, const std::vector<std::string>& data, int element_size);

    void message(const std::string& msg);
    void flushStream();

    void set_ix() { ix_standard = true; }

    friend class OutputStream::Restart;
    friend class OutputStream::SummarySpecification;

private:
    void writeBinaryHeader(const std::string& arrName, int64_t size, eclArrType arrType, int element_size);

    template <typename T>
    void writeBinaryArray(const std::vector<T>& data);

    void writeBinaryCharArray(const std::vector<std::string>& data, int element_size);
    void writeBinaryCharArray(const std::vector<PaddedOutputString<8>>& data);

    void writeFormattedHeader(const std::string& arrName, int size, eclArrType arrType, int element_size);

    template <typename T>
    void writeFormattedArray(const std::vector<T>& data);

    void writeFormattedCharArray(const std::vector<std::string>& data, int element_size);
    void writeFormattedCharArray(const std::vector<PaddedOutputString<8>>& data);

    void writeArrayType(const eclArrType arrType);
    std::string make_real_string_ecl(float value) const;
    std::string make_real_string_ix(float value) const;
    std::string make_doub_string_ecl(double value) const;
    std::string make_doub_string_ix(double value) const;

    bool isFormatted, ix_standard;
    std::ofstream ofileH;
};


template<>
void EclOutput::write<std::string>(const std::string& name,
                                   const std::vector<std::string>& data);

template <>
void EclOutput::write<PaddedOutputString<8>>
    (const std::string&                        name,
     const std::vector<PaddedOutputString<8>>& data);

}} // namespace Opm::EclIO

#endif // OPM_IO_ECLOUTPUT_HPP

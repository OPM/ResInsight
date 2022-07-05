/*
  Copyright (c) 2020 Equinor ASA

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

#include <sstream>

class MessageBuffer
{
private:
    std::stringstream str_{};

public:
    template <class T>
    void read(T& value)
    {
        this->str_.read(reinterpret_cast<char *>(&value), sizeof value);
    }

    template <class T>
    void write(T const& value)
    {
        this->str_.write(reinterpret_cast<const char *>(&value), sizeof value);
    }

    void write(const std::string& str)
    {
        const int size = str.size();
        this->write(size);
        for (int k = 0; k < size; ++k) {
            this->write(str[k]);
        }
    }

    void read(std::string& str)
    {
        int size = 0;
        this->read(size);
        str.resize(size);
        for (int k = 0; k < size; ++k) {
            this->read(str[k]);
        }
    }
};



/*
  Copyright 2020 Equinor ASA.

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

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef OPM_SERIALIZER_HPP
#define OPM_SERIALIZER_HPP

namespace Opm {
/*
  This is a very basic serialization class used to support serialization of
  small state objects from opm common. The main serialization code used in
  opm/flow is initiated and controlled from the restart code, and therefor
  slightly cumbersome to use for objects which should be serialized not as part
  of the restart code.
*/


class Serializer {
public:
    Serializer() = default;
    explicit Serializer(const std::vector<char>& buffer_arg) :
        buffer(buffer_arg)
    {}


    template <typename T>
    void put(const T& value) {
        this->pack(std::addressof(value), sizeof(T));
    }

    template <typename T>
    void put(const T* ) {
        throw std::logic_error("Serializer can not pack pointers");
    }

    template <typename T>
    T get() {
        T value;
        std::memcpy(&value, &this->buffer[this->read_pos], sizeof(T));
        this->read_pos += sizeof(T);
        return value;
    }

    template<typename T>
    void put_vector(const std::vector<T>& values) {
        this->put(values.size());
        this->pack(values.data(), values.size() * sizeof(T));
    }



    template<typename T>
    std::vector<T> get_vector() {
        std::size_t size = this->get<std::size_t>();
        std::vector<T> values(size);
        for (std::size_t index=0; index < size; index++)
            values[index] = this->get<T>();

        return values;
    }

    template<typename K, typename T>
    void put_map(const std::unordered_map<K,T>& values) {
        this->put(values.size());
        for (const auto& value_pair : values) {
            this->put(value_pair.first);
            this->put(value_pair.second);
        }
    }

    template<typename K, typename T>
    std::unordered_map<K,T> get_map() {
        std::unordered_map<K,T> values;
        auto size = this->get<std::size_t>();
        for (std::size_t index = 0; index < size; index++) {
            auto key = this->get<K>();
            auto value = this->get<T>();
            values.insert( std::make_pair(key,value) );
        }
        return values;
    }


    std::vector<char> buffer;
private:
    void pack(const void * ptr, std::size_t value_size) {
        std::size_t write_pos = this->buffer.size();
        std::size_t new_size = write_pos + value_size;
        this->buffer.resize( new_size );
        std::memcpy(&this->buffer[write_pos], ptr, value_size);
    }

    std::size_t read_pos = 0;
};

template <>
void inline Serializer::put(const std::string& value) {
    this->put(value.size());
    if (value.empty())
        return;

    this->pack(value.c_str(), value.size());
}

template<>
std::string inline Serializer::get<std::string>() {
    std::string::size_type length = this->get<std::string::size_type>();
    if (length == 0)
        return std::string{};

    this->read_pos += length;
    return {std::addressof(this->buffer[this->read_pos - length]), length};
}

}
#endif

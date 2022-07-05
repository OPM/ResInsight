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

#ifndef JSON_OBJECT_HPP
#define JSON_OBJECT_HPP

#include <filesystem>
#include <string>

#include <opm/common/utility/FileSystem.hpp>

struct cJSON;

namespace Json {

    class JsonObject {
    public:
        JsonObject();

        explicit JsonObject(const std::filesystem::path& jsonFile );
        explicit JsonObject(const std::string& inline_json);
        explicit JsonObject(const char * inline_json);
        explicit JsonObject(cJSON * root);
        ~JsonObject();

        void add(double value);
        void add(int value);
        void add(const std::string& value);
        JsonObject add_array();
        JsonObject add_object();
        void add_item(const std::string& key, double value);
        void add_item(const std::string& key, int value);
        void add_item(const std::string& key, const std::string& value);
        JsonObject add_array(const std::string& key);
        JsonObject add_object(const std::string& key);
        std::string dump() const;

        bool has_item(const std::string& key) const;
        JsonObject get_array_item( size_t index ) const;
        JsonObject get_item(const std::string& key) const;

        std::string to_string() const;
        std::string get_string(const std::string& key) const;
        std::string as_string() const;
        bool is_string( ) const;

        bool is_number( ) const;
        int get_int(const std::string& key) const;
        int as_int() const;
        double get_double(const std::string& key) const;
        double as_double() const;

        bool is_array( ) const;
        bool is_object( ) const;

        size_t size() const;
    private:
        JsonObject get_scalar_object(const std::string& key) const;
        void  initialize(const std::string& inline_json);
        cJSON * root;
        bool    owner;
    };
}



#endif



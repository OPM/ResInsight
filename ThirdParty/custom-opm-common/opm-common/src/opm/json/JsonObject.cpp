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


#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <opm/common/utility/FileSystem.hpp>

#include <opm/json/JsonObject.hpp>
#include <cJSON.h>

namespace Json {

    void JsonObject::initialize(const std::string& inline_json) {
        this->root = cJSON_Parse( inline_json.c_str() );
        if (!this->root)
            throw std::invalid_argument("Parsing json input failed");
        owner = true;
    }


    JsonObject::JsonObject(const std::string& inline_json) {
        initialize( inline_json );
    }

    JsonObject::JsonObject(const char * inline_json) {
        initialize( inline_json );
    }


    JsonObject::JsonObject() {
        this->owner = true;
        this->root = cJSON_CreateObject();
    }


    void JsonObject::add_item(const std::string& key, const std::string& value) {
        cJSON_AddStringToObject(this->root, key.c_str(), value.c_str());
    }

    void JsonObject::add_item(const std::string& key, int value) {
        cJSON_AddNumberToObject(this->root, key.c_str(), value);
    }

    void JsonObject::add_item(const std::string& key, double value) {
        cJSON_AddNumberToObject(this->root, key.c_str(), value);
    }

    JsonObject JsonObject::add_array(const std::string& key) {
        return JsonObject( cJSON_AddArrayToObject(this->root, key.c_str()) );
    }

    JsonObject JsonObject::add_object(const std::string& key) {
        return JsonObject( cJSON_AddObjectToObject(this->root, key.c_str()) );
    }

    void JsonObject::add(const std::string& value) {
        if (this->is_array())
            cJSON_AddItemToArray(this->root, cJSON_CreateString( value.c_str() ));
        else
            throw std::invalid_argument("Item is not an array");
    }

    void JsonObject::add(int value) {
        if (this->is_array())
            cJSON_AddItemToArray(this->root, cJSON_CreateNumber( value ));
        else
            throw std::invalid_argument("Item is not an array");
    }

    void JsonObject::add(double value) {
        if (this->is_array())
            cJSON_AddItemToArray(this->root, cJSON_CreateNumber( value ));
        else
            throw std::invalid_argument("Item is not an array");
    }

    JsonObject JsonObject::add_array() {
        if (this->is_array()) {
            cJSON_AddItemToArray(this->root, cJSON_CreateArray());
            return JsonObject( this->get_array_item( this->size() - 1));
        }
        else
            throw std::invalid_argument("Item is not an array");
    }

    JsonObject JsonObject::add_object() {
        if (this->is_array()) {
            cJSON_AddItemToArray(this->root, cJSON_CreateObject( ));
            return JsonObject( this->get_array_item( this->size() - 1));
        } else
            throw std::invalid_argument("Item is not an array");
    }

    std::string JsonObject::dump() const {
        char * c_str = cJSON_Print( this->root );
        std::string str(c_str);
        free( c_str );
        return str;
    }



    JsonObject::JsonObject(const std::filesystem::path& jsonFile ) {
        std::ifstream stream(jsonFile.string().c_str());
        if (stream) {
            std::string content_from_file( (std::istreambuf_iterator<char>(stream)),
                                 (std::istreambuf_iterator<char>()));

            this->root = cJSON_Parse( content_from_file.c_str() );
            if (!this->root)
                throw std::invalid_argument("Parsing json file: " + jsonFile.string() + " failed ");

            this->owner = true;
        } else
            throw std::invalid_argument("Loading json from file: " + jsonFile.string() + " failed.");
    }




    JsonObject::JsonObject( cJSON * object ) {
        this->root = object;
        owner = false;
    }


    JsonObject::~JsonObject() {
        if (owner && this->root)
            cJSON_Delete(this->root);
    }



    bool JsonObject::has_item( const std::string& key) const {
        cJSON * object = cJSON_GetObjectItem( this->root , key.c_str() );
        if (object)
            return true;
        else
            return false;
    }


    bool JsonObject::is_array( ) const {
        if (this->root->type == cJSON_Array)
            return true;
        else
            return false;
    }

    bool JsonObject::is_number( ) const {
        if (this->root->type == cJSON_Number)
            return true;
        else
            return false;
    }


    bool JsonObject::is_string( ) const {
        if (this->root->type == cJSON_String)
            return true;
        else
            return false;
    }

    bool JsonObject::is_object( ) const {
        if (this->root->type == cJSON_Object)
            return true;
        else
            return false;
    }


    size_t JsonObject::size() const {
        int int_size = cJSON_GetArraySize( this->root );
        return (size_t) int_size;
    }


    JsonObject JsonObject::get_array_item( size_t index ) const {
        if (is_array()) {
            cJSON * new_c_ptr = cJSON_GetArrayItem( this->root , index );
            if (new_c_ptr)
                return JsonObject( new_c_ptr );
            else
                throw std::invalid_argument("Index is out ouf range.");
        } else
            throw std::invalid_argument("Object is not an array.");
    }


    JsonObject JsonObject::get_item(const std::string& key) const {
        cJSON * c_ptr = cJSON_GetObjectItem( this->root , key.c_str() );
        if (c_ptr)
            return JsonObject( c_ptr );
        else
            throw std::invalid_argument("Key: " + key + " does not exist in json object");
    }


    std::string JsonObject::get_string(const std::string& key) const {
        JsonObject child = get_scalar_object( key );
        return child.as_string();
    }


    std::string JsonObject::as_string() const {
        if (is_string())
            return this->root->valuestring;
        else
            throw std::invalid_argument("Object is not a string object");
    }


    int JsonObject::get_int(const std::string& key) const {
        JsonObject child = get_scalar_object( key );
        return child.as_int( );
    }


    int JsonObject::as_int() const {
        if (this->root->type == cJSON_Number)
            return this->root->valueint;
        else
            throw std::invalid_argument("Object is not a number object.");
    }


    double JsonObject::get_double(const std::string& key) const {
        JsonObject child = get_scalar_object( key );
        return child.as_double( );
    }


    double JsonObject::as_double() const {
        if (this->root->type == cJSON_Number)
            return this->root->valuedouble;
        else
            throw std::invalid_argument("Object is not a number object.");
    }


    JsonObject JsonObject::get_scalar_object(const std::string& key) const{
        JsonObject child = get_item( key );
        if (child.size())
            throw std::invalid_argument("Key: " + key + " is not a scalar object");
        else
            return child;
    }



    std::string JsonObject::to_string() const {
        char * c_str = cJSON_Print( this->root );
        std::string s(c_str);
        free( c_str );

        return s;
    }


}


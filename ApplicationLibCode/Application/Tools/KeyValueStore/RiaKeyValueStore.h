/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// Element type of the values a key holds when they are stored as bytes
enum class RiaKeyValueStoreElementType
{
    FLOAT32,
    FLOAT64,
    INT32,
    INT64,
    UINT32,
    UINT64
};

//==================================================================================================
///
//==================================================================================================
template <typename T>
class RiaKeyValueStore
{
public:
    RiaKeyValueStore() = default;

    // Store a complete array for a key (overwrites existing data)
    bool set( const std::string& key, const std::vector<T>& values, RiaKeyValueStoreElementType elementType = RiaKeyValueStoreElementType::FLOAT32 )
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        m_store[key]        = values;
        m_elementTypes[key] = elementType;
        return true;
    }

    // Get the entire array for a key
    std::optional<std::vector<T>> get( const std::string& key ) const
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        if ( auto it = m_store.find( key ); it != m_store.end() )
        {
            return it->second;
        }
        return std::nullopt;
    }

    // Get the entire array for a key, only if the key holds values of the given element type
    std::optional<std::vector<T>> get( const std::string& key, RiaKeyValueStoreElementType elementType ) const
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        if ( auto it = m_store.find( key ); it != m_store.end() && m_elementTypes.at( key ) == elementType )
        {
            return it->second;
        }
        return std::nullopt;
    }

    // Get the element type of the values for a key
    std::optional<RiaKeyValueStoreElementType> elementType( const std::string& key ) const
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        if ( auto it = m_elementTypes.find( key ); it != m_elementTypes.end() )
        {
            return it->second;
        }
        return std::nullopt;
    }

    // Remove an entire key-value pair
    bool remove( const std::string& key )
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        m_elementTypes.erase( key );
        return m_store.erase( key ) > 0;
    }

    // Check if a key exists
    bool exists( const std::string& key ) const
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        return m_store.find( key ) != m_store.end();
    }

    // Get all keys
    std::vector<std::string> keys() const
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        std::vector<std::string>     result;
        result.reserve( m_store.size() );
        for ( const auto& [key, _] : m_store )
        {
            result.push_back( key );
        }
        return result;
    }

    // Clear all key-value pairs
    void clear()
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        m_store.clear();
        m_elementTypes.clear();
    }

private:
    std::unordered_map<std::string, std::vector<T>>              m_store;
    std::unordered_map<std::string, RiaKeyValueStoreElementType> m_elementTypes;
    mutable std::mutex                                           m_mutex;
};

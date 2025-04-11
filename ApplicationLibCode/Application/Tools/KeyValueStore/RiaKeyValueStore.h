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

//==================================================================================================
///
//==================================================================================================
template <typename T>
class RiaKeyValueStore
{
public:
    RiaKeyValueStore() = default;

    // Store a complete array for a key (overwrites existing data)
    bool set( const std::string& key, const std::vector<T>& values )
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
        m_store[key] = values;
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

    // Remove an entire key-value pair
    bool remove( const std::string& key )
    {
        std::scoped_lock<std::mutex> lock( m_mutex );
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
    }

private:
    std::unordered_map<std::string, std::vector<T>> m_store;
    mutable std::mutex                              m_mutex;
};

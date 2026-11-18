#include "RiaKeyValueStoreUtil.h"

#include <cstdint>
#include <cstring>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> RiaKeyValueStoreUtil::convertToVector( const std::optional<std::vector<char>>& inputVector )
{
    if ( !inputVector || inputVector->empty() ) return {};

    // Ensure the byte vector size is a multiple of sizeof(T)
    if ( inputVector->size() % sizeof( T ) != 0 ) return {};

    // Calculate how many values we'll have
    size_t numValues = inputVector->size() / sizeof( T );

    // Create a vector of values with the appropriate size
    std::vector<T> values( numValues );

    // Copy the binary data from the byte vector to the value vector
    std::memcpy( values.data(), inputVector->data(), inputVector->size() );

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<char> RiaKeyValueStoreUtil::convertToByteVector( const std::vector<T>& values )
{
    if ( values.empty() ) return {};

    // Calculate the total size needed for the byte array
    size_t sizeInBytes = values.size() * sizeof( T );

    // Create a vector of bytes with the appropriate size
    std::vector<char> byteVec( sizeInBytes );

    // Copy the binary data from the value vector to the byte vector
    std::memcpy( byteVec.data(), values.data(), sizeInBytes );

    return byteVec;
}

template std::vector<float>    RiaKeyValueStoreUtil::convertToVector<float>( const std::optional<std::vector<char>>& );
template std::vector<double>   RiaKeyValueStoreUtil::convertToVector<double>( const std::optional<std::vector<char>>& );
template std::vector<int32_t>  RiaKeyValueStoreUtil::convertToVector<int32_t>( const std::optional<std::vector<char>>& );
template std::vector<int64_t>  RiaKeyValueStoreUtil::convertToVector<int64_t>( const std::optional<std::vector<char>>& );
template std::vector<uint32_t> RiaKeyValueStoreUtil::convertToVector<uint32_t>( const std::optional<std::vector<char>>& );
template std::vector<uint64_t> RiaKeyValueStoreUtil::convertToVector<uint64_t>( const std::optional<std::vector<char>>& );

template std::vector<char> RiaKeyValueStoreUtil::convertToByteVector<float>( const std::vector<float>& );
template std::vector<char> RiaKeyValueStoreUtil::convertToByteVector<double>( const std::vector<double>& );
template std::vector<char> RiaKeyValueStoreUtil::convertToByteVector<int32_t>( const std::vector<int32_t>& );
template std::vector<char> RiaKeyValueStoreUtil::convertToByteVector<int64_t>( const std::vector<int64_t>& );
template std::vector<char> RiaKeyValueStoreUtil::convertToByteVector<uint32_t>( const std::vector<uint32_t>& );
template std::vector<char> RiaKeyValueStoreUtil::convertToByteVector<uint64_t>( const std::vector<uint64_t>& );

#include "RiaKeyValueStoreUtil.h"

#include <cstring>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RiaKeyValueStoreUtil::convertToFloatVector( const std::optional<std::vector<char>>& inputVector )
{
    if ( !inputVector || inputVector->empty() ) return {};

    // Ensure the byte vector size is a multiple of sizeof(float)
    if ( inputVector->size() % sizeof( float ) != 0 ) return {};

    // Calculate how many floats we'll have
    size_t numFloats = inputVector->size() / sizeof( float );

    // Create a vector of floats with the appropriate size
    std::vector<float> floatVector( numFloats );

    // Copy the binary data from the byte vector to the float vector
    std::memcpy( floatVector.data(), inputVector->data(), inputVector->size() );

    return floatVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<char> RiaKeyValueStoreUtil::convertToByteVector( const std::vector<float>& floatVec )
{
    if ( floatVec.empty() ) return {};

    // Calculate the total size needed for the byte array
    size_t sizeInBytes = floatVec.size() * sizeof( float );

    // Create a vector of bytes with the appropriate size
    std::vector<char> byteVec( sizeInBytes );

    // Copy the binary data from the float vector to the byte vector
    std::memcpy( byteVec.data(), floatVec.data(), sizeInBytes );

    return byteVec;
}

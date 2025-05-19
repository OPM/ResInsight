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

#include "RiaKeyValueStoreUtil.h"

std::vector<float> convertToFloatVector( const std::optional<std::vector<char>>& input )
{
    if ( !input || input->empty() ) return {};

    // Ensure the byte vector size is a multiple of sizeof(float)
    if ( input->size() % sizeof( float ) != 0 ) return {};

    // Calculate how many floats we'll have
    size_t float_count = input->size() / sizeof( float );

    // Create a vector of floats with the appropriate size
    std::vector<float> float_vec( float_count );

    // Copy the binary data from the byte vector to the float vector
    std::memcpy( float_vec.data(), input->data(), input->size() );

    return float_vec;
}

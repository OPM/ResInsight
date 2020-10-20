#pragma once

// Brings in size_t and definition of NULL
#include <cstddef>

// Macro to disable the copy constructor and assignment operator
// Should be used in the private section of a class
#define PDM_DISABLE_COPY_AND_ASSIGN( CLASS_NAME ) \
    CLASS_NAME( const CLASS_NAME& );              \
    void operator=( const CLASS_NAME& )

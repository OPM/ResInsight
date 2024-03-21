/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RiaOpenMPTools.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaOpenMPTools::availableThreadCount()
{
    int numberOfThreads = 1;
#ifdef USE_OPENMP
    numberOfThreads = omp_get_max_threads();
#endif

    return numberOfThreads;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaOpenMPTools::currentThreadIndex()
{
    int myThread = 0;

#ifdef USE_OPENMP
    myThread = omp_get_thread_num();
#endif

    return myThread;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaOpenMPTools::numberOfThreads()
{
    int numberOfThreads = 1;

#ifdef USE_OPENMP

#pragma omp parallel
    {
        numberOfThreads = omp_get_num_threads();
    }

#endif

    return numberOfThreads;
}

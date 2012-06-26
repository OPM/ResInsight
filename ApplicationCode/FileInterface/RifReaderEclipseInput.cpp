/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfBase.h"

#include "RigMainGrid.h"
#include "RigReservoir.h"
#include "RigReservoirCellResults.h"

#include "RifReaderEclipseInput.h"
#include "RifReaderEclipseOutput.h"

#include "RifEclipseInputFileTools.h"

#include <iostream>
#include <cmath>

#ifdef USE_ECL_LIB
#include "ecl_grid.h"
#include "well_state.h"
#endif //USE_ECL_LIB
#include "util.h"

//==================================================================================================
//
// Class RifReaderEclipseInput
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseInput::RifReaderEclipseInput()
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseInput::~RifReaderEclipseInput()
{
}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseInput::open(const QString& fileName, RigReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    // Make sure everything's closed
    close();

    // Should we handle gridless properties ?
    //    If so, they must match dimentions, and a grid afterwards must match dimension

    // Add file:
    //   Open file
    //   If we do not have any grid data, 
    //      Search for grid keywords
    //      If grid data found
    //         Read grid keywords,
    //         produce ecl_grid,
    //         Transfer data to RigReservoir
    //      if not
    //          find include
    //  else
    //      Search through file for property keywords
    //         If found, 
    //              read them, 
    //              create InputProperty object

    bool isOk = false;
    if (reservoir->mainGrid()->gridPointDimensions() == cvf::Vec3st(0,0,0))
    {
        isOk = RifEclipseInputFileTools::openGridFile(fileName,  reservoir);
    }
    
    return isOk;
}

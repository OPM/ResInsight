/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RimRftWellCompletionTrack.h"

#include "RimModeledWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"

#include "RifReaderOpmRft.h"
#include "WellLogCommands/RicAppendWellPathFromRftDataFeature.h"

CAF_PDM_SOURCE_INIT( RimRftWellCompletionTrack, "RimRftWellCompletionTrack" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftWellCompletionTrack::RimRftWellCompletionTrack()
{
    CAF_PDM_InitObject( "Rft Track", ":/WellLogTrack16x16.png" );

    setShowWellPathAttributes( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::configureForWellPath( const QString& wellPathName, RifReaderOpmRft* rftReader )
{
    QString wellName = "[RFT Dummy] - " + wellPathName;

    auto wellPath = RicAppendWellPathFromRftDataFeature::findOrCreateWellAttributeWellPath( wellName );

    // Update well path attributes, packers and casing based on RFT data
    /*
        if ( rftReader )
        {
            rftReader->set
        }
    */

    wellPath->attributeCollection()->deleteAllAttributes();
    auto attribute = new RimWellPathAttribute;
    attribute->setDepthsFromWellPath( wellPath );
    wellPath->attributeCollection()->insertAttribute( nullptr, attribute );

    setWellPathAttributesSource( wellPath );
}

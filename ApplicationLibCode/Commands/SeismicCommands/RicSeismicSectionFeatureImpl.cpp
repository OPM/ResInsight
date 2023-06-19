/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RicSeismicSectionFeatureImpl.h"

#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSeismicSectionFeatureImpl::createSeismicSection( RiaDefines::SeismicSectionType sectionType )
{
    // Find the selected seismic section collection
    std::vector<RimSeismicSectionCollection*> colls = caf::selectedObjectsByTypeStrict<RimSeismicSectionCollection*>();
    if ( colls.empty() ) return;
    RimSeismicSectionCollection* seisColl = colls[0];

    RimSeismicSection* newSection = seisColl->addNewSection();
    if ( newSection )
    {
        newSection->setSectionType( sectionType );
        Riu3DMainWindowTools::selectAsCurrentItem( newSection );
        if ( sectionType == RiaDefines::SeismicSectionType::SS_POLYLINE ) newSection->enablePicking( true );
    }
}

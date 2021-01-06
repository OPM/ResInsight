/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotCellFilter.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"

#include "cvfArray.h"

class RimEclipseResultDefinition;

//==================================================================================================
///
//==================================================================================================
class RimPlotCellPropertyFilter : public RimPlotCellFilter
{
    CAF_PDM_HEADER_INIT;

public:
    RimPlotCellPropertyFilter();

    // Currently supported result definition is RimEclipseResultDefinition, but the interface is designed to also
    // support RimGeoMechResultDefinition
    void setResultDefinition( caf::PdmObject* resultDefinition );

    void setValueRange( double lowerBound, double upperBound );

    void updatePointerAfterCopy( RimPlotCellPropertyFilter* other );

protected:
    void updateCellVisibilityFromFilter( size_t timeStepIndex, cvf::UByteArray* visibleCells ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    RimEclipseResultDefinition* eclipseResultDefinition();
    void                        findOrComputeMinMaxResultValues( double& minimumValue, double& maximumValue );
    void                        updateName();

private:
    caf::PdmChildField<caf::PdmObject*> m_resultDefinition;
    caf::PdmField<double>               m_lowerBound;
    caf::PdmField<double>               m_upperBound;
};

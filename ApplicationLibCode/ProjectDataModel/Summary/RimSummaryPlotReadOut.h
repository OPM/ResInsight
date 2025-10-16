/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaPlotDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiItem.h"

class RimAnnotationLineAppearance;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryPlotReadOut : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotReadOut();

    bool                         enableCurvePointTracking() const;
    bool                         enableHorizontalLine() const;
    bool                         enableVerticalLine() const;
    RimAnnotationLineAppearance* lineAppearance() const;
    RiaDefines::TextAlignment    verticalLineLabelAlignment() const;
    RiaDefines::TextAlignment    horizontalLineLabelAlignment() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<caf::AppEnum<RiaDefines::ReadOutType>> m_readOutType;

    caf::PdmChildField<RimAnnotationLineAppearance*> m_lineAppearance;

    caf::PdmField<caf::AppEnum<RiaDefines::TextAlignment>> m_verticalLineLabelAlignment;
    caf::PdmField<caf::AppEnum<RiaDefines::TextAlignment>> m_horizontalLineLabelAlignment;
};

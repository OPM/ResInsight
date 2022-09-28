/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimWellLogCurve.h"

#include "cafPdmPtrField.h"

#include "RiaRftDefines.h"
#include <QDateTime>

class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimRftTopologyCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimRftTopologyCurve();

    void setDataSource( RimSummaryCase*           summaryCase,
                        const QDateTime&          timeStep,
                        const QString&            wellName,
                        int                       segmentBranchIndex,
                        RiaDefines::RftBranchType branchType );

    QString wellName() const override;
    QString wellLogChannelUiName() const override;
    QString wellLogChannelUnits() const override;

protected:
    QString createCurveAutoName() override;

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onLoadDataAndUpdate( bool updateParentPlot ) override;

private:
    caf::PdmPtrField<RimSummaryCase*>                      m_summaryCase;
    caf::PdmField<QDateTime>                               m_timeStep;
    caf::PdmField<QString>                                 m_wellName;
    caf::PdmField<int>                                     m_segmentBranchIndex;
    caf::PdmField<caf::AppEnum<RiaDefines::RftBranchType>> m_segmentBranchType;
};

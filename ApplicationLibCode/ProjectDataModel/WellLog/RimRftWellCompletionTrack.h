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

#pragma once

#include "RimWellLogTrack.h"

#include <QDateTime>

class RifReaderOpmRft;
class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimRftWellCompletionTrack : public RimWellLogTrack
{
    CAF_PDM_HEADER_INIT;

public:
    RimRftWellCompletionTrack();

    void setDataSource( RimSummaryCase* summaryCase, const QDateTime& timeStep, const QString& wellName );

private:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void configureForWellPath( RimSummaryCase* summaryCase, const QDateTime& timeStep, const QString& wellName );

private:
    caf::PdmPtrField<RimSummaryCase*> m_summaryCase;
    caf::PdmField<QDateTime>          m_timeStep;
    caf::PdmField<QString>            m_wellName;
};

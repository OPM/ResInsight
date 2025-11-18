/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimWellPath;
class RimWellLogChannel;
class RimWellLogFile;
class RigWellLogIndexDepthOffset;
class RimWellLog;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogLasFileCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogLasFileCurve();
    ~RimWellLogLasFileCurve() override;

    void         setWellPath( RimWellPath* wellPath );
    RimWellPath* wellPath() const;
    void         setWellLogChannelName( const QString& name );
    void         setWellLog( RimWellLog* wellLog );
    void         setIndexDepthOffsets( std::shared_ptr<RigWellLogIndexDepthOffset> depthOffsets );

    // Overrides from RimWellLogPlotCurve
    QString wellName() const override;
    QString wellLogChannelUiName() const override;
    QString wellLogChannelUnits() const override;

    RimWellLog* wellLog() const;

protected:
    // Overrides from RimWellLogPlotCurve
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    // Pdm overrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;

    bool isRftPlotChild() const;

    std::pair<std::vector<double>, std::vector<double>> adjustByIndexDepthOffsets( const std::vector<double>& measuredDepthValues,
                                                                                   const std::vector<double>& values,
                                                                                   const std::vector<double>& kIndexValues ) const;

protected:
    caf::PdmPtrField<RimWellPath*> m_wellPath;
    caf::PdmPtrField<RimWellLog*>  m_wellLog;
    caf::PdmField<QString>         m_wellLogChannelName;
    caf::PdmField<QString>         m_wellLogChannnelUnit;

    std::shared_ptr<RigWellLogIndexDepthOffset> m_indexDepthOffsets;
};

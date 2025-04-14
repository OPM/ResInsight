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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimPlotWindow;
class RimSummaryPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimPathPatternFileSet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPathPatternFileSet();

    static std::pair<QString, QString> findPathPattern( const QStringList& filePaths, const QString& placeHolderText );
    static QStringList createPathsFromPattern( const std::pair<QString, QString>& pathPattern, const QString& placeHolderText );

private:
    /*
        void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
        QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    */

private:
    caf::PdmField<QString> m_templatePath;
    caf::PdmField<QString> m_variableDefinition;
};

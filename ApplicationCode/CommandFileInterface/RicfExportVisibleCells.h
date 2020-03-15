/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RiaDefines.h"

#include "RicfCommandObject.h"

#include "cafPdmField.h"

class RimEclipseView;
class RicSaveEclipseInputVisibleCellsUi;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfExportVisibleCells : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

    enum ExportKeyword
    {
        FLUXNUM,
        MULTNUM
    };

public:
    RicfExportVisibleCells();

    caf::PdmScriptResponse execute() override;

private:
    void buildExportSettings( const QString& exportFolder, RicSaveEclipseInputVisibleCellsUi* exportSettings );

    caf::PdmField<int>                         m_caseId;
    caf::PdmField<int>                         m_viewId;
    caf::PdmField<QString>                     m_viewName;
    caf::PdmField<caf::AppEnum<ExportKeyword>> m_exportKeyword;
    caf::PdmField<int>                         m_visibleActiveCellsValue;
    caf::PdmField<int>                         m_hiddenActiveCellsValue;
    caf::PdmField<int>                         m_inactiveCellsValue;
};

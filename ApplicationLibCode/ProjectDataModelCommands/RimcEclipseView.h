/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "ExportCommands/RicSaveEclipseInputVisibleCellsUi.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"

#include "cvfObject.h"

#include <QString>

class RimEclipseView;
class RigResultAccessor;
class RimFaultInView;

//==================================================================================================
///
//==================================================================================================
class RimcEclipseView_addFaultDistance : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseView_addFaultDistance( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>                 m_resultName;
    caf::PdmPtrArrayField<RimFaultInView*> m_faults;
};

//==================================================================================================
/// Export the cell result currently shown in the view (at the current time step) to a GRDECL style text file.
//==================================================================================================
class RimEclipseView_exportCurrentProperty : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseView_exportCurrentProperty( caf::PdmObjectHandle* self );

    void setExportFile( const QString& exportFile );
    void setUndefinedValue( double undefinedValue );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

    /// Default file base name: <case>-<view>-T<time step>-<property>
    static QString defaultFileBaseName( const RimEclipseView* view );

    /// True if the cell result currently shown in the view is available at the current time step
    static bool hasCurrentProperty( const RimEclipseView* view );

private:
    static cvf::ref<RigResultAccessor> currentPropertyAccessor( const RimEclipseView* view );

    caf::PdmField<QString> m_exportFile;
    caf::PdmField<double>  m_undefinedValue;
};

//==================================================================================================
/// Export a GRDECL keyword (FLUXNUM, MULTNUM or ACTNUM) with one value per cell based on the cell
/// visibility in the view at the current time step.
//==================================================================================================
class RimEclipseView_exportVisibleCells : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseView_exportVisibleCells( caf::PdmObjectHandle* self );

    void setExportFile( const QString& exportFile );
    void setExportKeyword( RicSaveEclipseInputVisibleCellsUi::ExportKeyword exportKeyword );
    void setVisibleActiveCellsValue( int value );
    void setHiddenActiveCellsValue( int value );
    void setInactiveCellsValue( int value );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString>                                                        m_exportFile;
    caf::PdmField<caf::AppEnum<RicSaveEclipseInputVisibleCellsUi::ExportKeyword>> m_exportKeyword;
    caf::PdmField<int>                                                            m_visibleActiveCellsValue;
    caf::PdmField<int>                                                            m_hiddenActiveCellsValue;
    caf::PdmField<int>                                                            m_inactiveCellsValue;
};

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

#include "RigCompletionData.h"

#include "RicLgrSplitType.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfVector3.h"

#include <QStringList>

#include <set>

class RimEclipseCase;
class RicCellRangeUi;

//==================================================================================================
///
//==================================================================================================
class RicExportLgrUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicExportLgrUi();

    void setCase( RimEclipseCase* rimCase );
    void setTimeStep( int timeStep );

    cvf::Vec3st                                 refinement() const;
    QString                                     exportFolder() const;
    RimEclipseCase*                             caseToApply() const;
    int                                         timeStep() const;
    std::set<RigCompletionData::CompletionType> completionTypes() const;
    Lgr::SplitType                              splitType() const;

    void hideExportFolderField( bool hide );
    void setExportFolder( const QString& folder );

private:
    void setDefaultValuesFromCase();

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<QString>            m_exportFolder;
    caf::PdmPtrField<RimEclipseCase*> m_caseToApply;
    caf::PdmField<int>                m_timeStep;
    caf::PdmField<bool>               m_includePerforations;
    caf::PdmField<bool>               m_includeFractures;
    caf::PdmField<bool>               m_includeFishbones;

    caf::PdmField<int> m_refinementI;
    caf::PdmField<int> m_refinementJ;
    caf::PdmField<int> m_refinementK;

    caf::PdmField<Lgr::SplitTypeEnum> m_splitType;
};

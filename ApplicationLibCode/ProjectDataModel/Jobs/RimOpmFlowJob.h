/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimGenericJob.h"

#include "cafPdmPtrField.h"

class RimEclipseCase;

//==================================================================================================
///
///
//==================================================================================================
class RimOpmFlowJob : public RimGenericJob
{
    CAF_PDM_HEADER_INIT;

public:
    RimOpmFlowJob();
    ~RimOpmFlowJob() override;

    void setWorkingDirectory( QString workDir );
    void setEclipseCase( RimEclipseCase* eCase );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QString     title() override;
    QStringList command() override;
    QString     workingDirectory() override;
    bool        onPrepare() override;
    void        onCompleted( bool success ) override;

private:
    RimEclipseCase* findExistingCase( QString filename );
    QString         deckName();

private:
    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;
    caf::PdmField<caf::FilePath>      m_workDir;

    QString m_deckName;
};

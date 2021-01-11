/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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
#include "cvfObject.h"

class RigFormationNames;
class RimColorLegend;

//==================================================================================================
///
//==================================================================================================
class RimFormationNames : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFormationNames();
    ~RimFormationNames() override;

    void    setFileName( const QString& fileName );
    QString fileName();
    QString fileNameWoPath();

    RigFormationNames* formationNamesData() { return m_formationNamesData.p(); }
    RimColorLegend*    colorLegend() const;
    void               updateConnectedViews();

    void readFormationNamesFile( QString* errorMessage );
    void updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath );

    static QString layerZoneTableFileName();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    virtual void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;

private:
    void updateUiTreeName();

private:
    caf::PdmField<caf::FilePath>      m_formationNamesFileName;
    caf::PdmPtrField<RimColorLegend*> m_colorLegend;
    cvf::ref<RigFormationNames>       m_formationNamesData;
};

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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cvfObject.h"

class RigFormationNames;

//==================================================================================================
///  
//==================================================================================================
class RimFormationNames : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFormationNames();
    ~RimFormationNames() override;

    void                        setFileName(const QString& fileName);
    const QString&              fileName();
    QString                     fileNameWoPath();

    RigFormationNames*          formationNamesData() { return m_formationNamesData.p();}
    void                        updateConnectedViews();

    void                        readFormationNamesFile(QString * errorMessage);
    void                        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

protected:
    void                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                initAfterRead() override;

private:
    void                        updateUiTreeName();

    caf::PdmField<QString>      m_formationNamesFileName;

    cvf::ref<RigFormationNames> m_formationNamesData;
};

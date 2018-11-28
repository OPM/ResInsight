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
#include "cafPdmChildArrayField.h"

class RimFormationNames;

//==================================================================================================
///  
//==================================================================================================
class RimFormationNamesCollection: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFormationNamesCollection();
    ~RimFormationNamesCollection() override;

    const caf::PdmChildArrayField<RimFormationNames*>& formationNamesList() const { return m_formationNamesList;}

    void readAllFormationNames();
    
    RimFormationNames* importFiles(const QStringList& fileNames);
    void updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

private:
    caf::PdmChildArrayField<RimFormationNames*> m_formationNamesList;
};

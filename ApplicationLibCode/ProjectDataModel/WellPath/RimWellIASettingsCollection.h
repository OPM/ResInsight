/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <QString>

class RimWellIASettings;
class RimWellPath;
class RimGeoMechCase;

class RimWellIASettingsCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellIASettingsCollection();
    ~RimWellIASettingsCollection() override;

    std::vector<RimWellIASettings*> settings() const;

    bool isEnabled() const;
    bool hasSettings() const;

    RimWellIASettings* startWellIntegrationAnalysis( QString         baseDir,
                                                     RimWellPath*    wellPath,
                                                     double          measuredDepth,
                                                     RimGeoMechCase* theCase,
                                                     QString&        outErrmsg );

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    caf::PdmChildArrayField<RimWellIASettings*> m_wellIASettings;
};

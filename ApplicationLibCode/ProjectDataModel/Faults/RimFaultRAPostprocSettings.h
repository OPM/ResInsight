/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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

#include <list>

#include <QString>
#include <QStringList>

class RimFaultRASettings;
class RimParameterGroup;

class RimFaultRAPostprocSettings : public caf::PdmObject
{
public:
    RimFaultRAPostprocSettings();
    ~RimFaultRAPostprocSettings() override;

    void initFromSettings( RimFaultRASettings* settings );

    QStringList stepsToLoad();

    QString postprocParameterFilename( int faultID ) const;
    QString outputBaseDirectory() const;
    QString basicMacrisDatabase() const;
    QString advancedMacrisDatabase() const;

    bool geomechEnabled() const;

    RimParameterGroup* parameters() const;

    QStringList postprocCommandParameters( int faultID ) const;

protected:
    caf::PdmField<QString>               m_baseDir;
    caf::PdmField<int>                   m_startTimestepEclipse;
    caf::PdmField<int>                   m_endTimestepEclipse;
    caf::PdmField<bool>                  m_geomechEnabled;
    caf::PdmField<QString>               m_basicMacrisDatabase;
    caf::PdmField<QString>               m_advancedMacrisDatabase;
    caf::PdmPtrField<RimParameterGroup*> m_postprocParameters;
};

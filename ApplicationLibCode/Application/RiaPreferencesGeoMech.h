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
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesGeoMech : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaPreferencesGeoMech();

    static RiaPreferencesGeoMech* current();

    void appendItems( caf::PdmUiOrdering& uiOrdering ) const;

    bool validateWIASettings() const;
    bool validateFRMSettings() const;

    // geomech settings
    QString geomechWIADefaultXML() const;
    QString geomechWIACommand() const;

    QString geomechFRMDefaultXML() const;
    QString geomechFRMCommand() const;

    bool waitBeforeRun() const;
    bool keepTemporaryFiles() const;

private:
    bool filesExists( QStringList& filelist ) const;

    caf::PdmField<QString> m_geomechWIADefaultXML;
    caf::PdmField<QString> m_geomechWIACommand;

    caf::PdmField<QString> m_geomechFRMDefaultXML;
    caf::PdmField<QString> m_geomechFRMCommand;

    caf::PdmField<bool> m_waitForInputFileEdit;
    caf::PdmField<bool> m_keepTemporaryFiles;
};

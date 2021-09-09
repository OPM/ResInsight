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

#include "cafAppEnum.h"
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

    bool validateFRASettings() const;
    bool validateWIASettings() const;

    // geomech settings
    QString geomechFRAPreprocCommand() const;
    QString geomechFRAPostprocCommand() const;
    QString geomechFRAMacrisCommand() const;
    QString geomechFRADefaultBasicXML() const;
    QString geomechFRADefaultAdvXML() const;

    QString geomechWIADefaultXML() const;
    QString geomechWIACommand() const;

    bool keepTemporaryFiles() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    bool filesExists( QStringList& filelist ) const;

    caf::PdmField<QString> m_geomechFRAPreprocCommand;
    caf::PdmField<QString> m_geomechFRAPostprocCommand;
    caf::PdmField<QString> m_geomechFRAMacrisCommand;
    caf::PdmField<QString> m_geomechFRADefaultBasicXML;
    caf::PdmField<QString> m_geomechFRADefaultAdvXML;

    caf::PdmField<QString> m_geomechWIADefaultXML;
    caf::PdmField<QString> m_geomechWIACommand;

    caf::PdmField<bool> m_keepTemporaryFiles;
};

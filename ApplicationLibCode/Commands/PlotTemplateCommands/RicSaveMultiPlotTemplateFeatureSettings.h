////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
///
//==================================================================================================
class RicSaveMultiPlotTemplateFeatureSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicSaveMultiPlotTemplateFeatureSettings();

    void    setFilePath( const QString& filePath );
    QString filePath() const;

    void    setName( const QString& name );
    QString name() const;

    bool usePlacholderForWells() const;
    bool usePlacholderForGroups() const;
    bool usePlacholderForRegions() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<caf::FilePath> m_filePath;
    caf::PdmField<QString>       m_name;

    caf::PdmField<bool> m_persistObjectNameForWells;
    caf::PdmField<bool> m_persistObjectNameGroups;
    caf::PdmField<bool> m_persistObjectNameRegions;
};

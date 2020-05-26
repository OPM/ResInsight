/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigFaciesProperties.h"

#include <QString>

#include <tuple>

typedef std::tuple<QString, QString, QString> FaciesKey;

//==================================================================================================
///
//==================================================================================================
class RimFaciesProperties : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaciesProperties();
    ~RimFaciesProperties() override;

    QString filePath() const;
    void    setFilePath( const QString& filePath );

    void setPropertiesForFacies( FaciesKey& key, const RigFaciesProperties& properties );

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    QString generatePropertiesTable();

    caf::PdmField<caf::FilePath> m_filePath;
    caf::PdmField<QString>       m_propertiesTable;

    std::map<FaciesKey, RigFaciesProperties> m_properties;
};

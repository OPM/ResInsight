/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifWellPathFormationReader.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <expected>
#include <optional>
#include <vector>

//==================================================================================================
/// A single well formations/well picks file (e.g. an FMU formations.csv or a well pick export),
/// owned by RimWellFormationsCollection. Both RimEnsembleFileSet and RimWellPath can point to an
/// entry, so the file is only parsed once.
//==================================================================================================
class RimWellFormationsFile : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellFormationsFile();

    void    setFilePath( const QString& filePath );
    QString filePath() const;
    QString shortName() const;

    std::expected<void, QString> reload();

    QStringList                  wellNames() const;
    QStringList                  zoneNames( const QString& wellName ) const;
    const RigWellPathFormations* formationsForWell( const QString& wellName ) const;

    static QString normalizedWellName( const QString& wellName );
    static void    updateReferringObjects( const std::vector<caf::PdmObjectHandle*>& referringObjects );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void initAfterRead() override;

private:
    void updateUiTreeName();
    void updateContentTable();

private:
    caf::PdmField<caf::FilePath> m_filePath;
    caf::PdmField<QString>       m_contentTable;

    RifWellPathFormationReader::WellFormations m_wellFormations;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>
#include <QStringList>

class RimEclipseResultCase;

//==================================================================================================
///
//==================================================================================================
class RimcWellPathCollection_importWellPath : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPathCollection_importWellPath( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_fileName;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPathCollection_importWellPathFromPointsInternal : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPathCollection_importWellPathFromPointsInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_coordinateXKey;
    caf::PdmField<QString> m_coordinateYKey;
    caf::PdmField<QString> m_coordinateZKey;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPathCollection_setMswNameGrouping : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPathCollection_setMswNameGrouping( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_mswNameGrouping;
};

//==================================================================================================
/// Import well paths from a list of files and/or all well path files in a folder.
/// Returns the names of the imported well paths as a string container.
//==================================================================================================
class RimWellPathCollection_importWellPaths : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathCollection_importWellPaths( caf::PdmObjectHandle* self );

    void setWellPathFiles( const std::vector<QString>& wellPathFiles );
    void setWellPathFolder( const QString& wellPathFolder );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

    /// Warnings from the last execute(), e.g. files that could not be parsed
    QStringList warnings() const;

private:
    caf::PdmField<std::vector<QString>> m_wellPathFiles;
    caf::PdmField<QString>              m_wellPathFolder;

    QStringList m_warnings;
};

//==================================================================================================
/// Import well log files (LAS) from a list of files and/or all well log files in a folder, and attach
/// them to the well paths with matching names. Returns the names of the affected well paths.
//==================================================================================================
class RimWellPathCollection_importWellLogFiles : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathCollection_importWellLogFiles( caf::PdmObjectHandle* self );

    void setWellLogFiles( const std::vector<QString>& wellLogFiles );
    void setWellLogFolder( const QString& wellLogFolder );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

    /// Warnings from the last execute(), e.g. files that could not be parsed
    QStringList warnings() const;

private:
    caf::PdmField<std::vector<QString>> m_wellLogFiles;
    caf::PdmField<QString>              m_wellLogFolder;

    QStringList m_warnings;
};

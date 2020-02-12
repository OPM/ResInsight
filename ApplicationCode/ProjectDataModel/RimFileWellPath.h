/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RimWellPath.h"

class RimFileWellPath : public RimWellPath
{
    CAF_PDM_HEADER_INIT;

public:
    RimFileWellPath();
    ~RimFileWellPath() override;

    QString filePath() const;
    void    setFilepath( const QString& path );
    bool    readWellPathFile( QString* errorMessage, RifWellPathImporter* wellPathImporter );
    int     wellPathIndexInFile() const; // -1 means none.
    void    setWellPathIndexInFile( int index );
    void    updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath ) override;
    static QString getCacheDirectoryPath();

private:
    QString surveyType() { return m_surveyType; }
    void    setSurveyType( QString surveyType );
    bool    isStoredInCache() const;
    QString getCacheFileName();

    void setupBeforeSave() override;

    caf::PdmField<caf::FilePath> m_filePath;
    caf::PdmField<QString>       m_filePathInCache; // Used for SSIHUB imported well paths
    caf::PdmField<int>           m_wellPathIndexInFile; // -1 means none.

    caf::PdmField<QString> id;
    caf::PdmField<QString> sourceSystem;
    caf::PdmField<QString> utmZone;
    caf::PdmField<QString> updateDate;
    caf::PdmField<QString> updateUser;
    caf::PdmField<QString> m_surveyType;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};

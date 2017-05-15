/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigWellPath.h"

#include "cvfObject.h"

#include <map>
#include <vector>
#include <QString>
#include <QDateTime>

//==================================================================================================
///  
///  
//==================================================================================================
class RifWellPathImporter
{
public:
    struct WellData
    {
        QString                 m_name;
        cvf::ref<RigWellPath>   m_wellPathGeometry;
    };

    struct WellMetaData
    {
        QString                 m_name;
        QString                 m_id;
        QString                 m_sourceSystem;
        QString                 m_utmZone;
        QString                 m_updateUser;
        QString                 m_surveyType;
        QDateTime               m_updateDate;
    };

    WellData                    readWellData(const QString& filePath, size_t indexInFile);
    WellData                    readWellData(const QString& filePath);
    WellMetaData                readWellMetaData(const QString& filePath, size_t indexInFile);
    WellMetaData                readWellMetaData(const QString& filePath);
    size_t                      wellDataCount(const QString& filePath);

    void                        clear();
    void                        removeFilePath(const QString& filePath);

private:
    WellData                    readJsonWellData(const QString& filePath);
    WellMetaData                readJsonWellMetaData(const QString& filePath);
    WellData                    readAsciiWellData(const QString& filePath, size_t indexInFile);
    WellMetaData                readAsciiWellMetaData(const QString& filePath, size_t indexInFile);
    void                        readAllAsciiWellData(const QString& filePath);

    inline bool                 isJsonFile(const QString& filePath);

    std::map<QString, std::vector<RifWellPathImporter::WellData> > m_fileNameToWellDataGroupMap;
};

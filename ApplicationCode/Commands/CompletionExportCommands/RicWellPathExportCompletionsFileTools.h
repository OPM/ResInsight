/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include <QFile>
#include <QString>

#include <memory>

class RimWellPath;

class RicWellPathExportCompletionsFileTools
{
public:
    class OpenFileException
    {
    public:
        OpenFileException(const QString& message);
        QString message;
    };

    static std::shared_ptr<QFile> openFileForExport(const QString& folderName, const QString& fileName);
    static std::shared_ptr<QFile> openFileForExport(const QString& fullFileName);
    static const RimWellPath* findWellPathFromExportName(const QString& wellNameForExport);
  
};
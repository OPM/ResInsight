//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include <vector>

class QLineEdit;
class QString;
class QStringList;

namespace caf {

//==================================================================================================
//
// 
//
//==================================================================================================
class Utils
{
public:
    static double       editToDouble(QLineEdit* lineEdit, double defaultVal);

    static QString      absoluteFileName(const QString& fileName);
    static QStringList  getFilesInDirectory(const QString& dirPath, const QString& nameFilter, bool getAbsoluteFileNames);
    static QStringList  getFilesInDirectory(const QString& dirPath, const QStringList& nameFilters, bool getAbsoluteFileNames);
    static QString      constructFullFileName(const QString& folder, const QString& baseFileName, const QString& extension);
    static QString      makeValidFileBasename(const QString& fileBasenameCandidate);

    static QString      indentString(int numSpacesToIndent, const QString& str);
    
    static bool         getSaveDirectoryAndCheckOverwriteFiles(const QString& defaultDir, std::vector<QString> fileNames, QString* saveDir);

    static bool         fileExists(const QString& fileName);
    static QString      fileExtension(const QString& fileName);
    static bool         isFolderWritable(const QString& folderName);

    static bool         isStringMatch(const QString& filterString, const QString& value);
    static bool         removeDirectoryAndFilesRecursively(const QString& dirName);
};

}

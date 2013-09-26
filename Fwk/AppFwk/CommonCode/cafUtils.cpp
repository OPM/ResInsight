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


#include "cafUtils.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <QtGui/QLineEdit>

#include <QtOpenGL/QGLContext>

namespace caf {


    //--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Utils::editToDouble(QLineEdit* lineEdit, double defaultVal)
{
    if (!lineEdit) return false;
//    CVF_ASSERT(lineEdit);

    bool ok = false;
    double val = lineEdit->text().toDouble(&ok);
    if (ok)
    {
        return val;
    }
    else
    {
        return defaultVal;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Utils::absoluteFileName(const QString& fileName)
{
    QFileInfo fi(fileName);
    return QDir::toNativeSeparators(fi.absoluteFilePath());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList Utils::getFilesInDirectory(const QString& dirPath, const QString& filter, bool getAbsoluteFileNames)
{
    QDir::SortFlags sortFlags = QDir::SortFlags(QDir::Name | QDir::IgnoreCase);

    // Only get files
    QDir::Filters filters = QDir::Files;

    QDir dir(dirPath, filter, sortFlags, filters);

    QFileInfoList fileInfoList = dir.entryInfoList();
    
    QStringList retFileNames;

    QListIterator<QFileInfo> it(fileInfoList);
    while (it.hasNext())
    {
        QFileInfo fi = it.next();
        QString entry = getAbsoluteFileNames ? fi.absoluteFilePath() : fi.fileName();

        entry = QDir::toNativeSeparators(entry);
        retFileNames.push_back(entry);
    }

    return retFileNames;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Utils::constructFullFileName(const QString& folder, const QString& baseFileName, const QString& extension)
{
    QFileInfo fi(folder, baseFileName + extension);
    QString fullFileName = fi.filePath();
    fullFileName = QDir::toNativeSeparators(fullFileName);

    return fullFileName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Utils::indentString(int numSpacesToIndent, const QString& str)
{
	QString indentString;
	indentString.fill(' ', numSpacesToIndent);

	QStringList strList = str.split("\n");

	QString retStr = indentString + strList.join("\n" + indentString);
    return retStr;
}


} // namespace caf

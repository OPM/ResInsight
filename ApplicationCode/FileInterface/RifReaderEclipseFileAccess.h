/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfLibCore.h"

#include <QString>
#include <QStringList>
#include <QDateTime>

#ifdef USE_ECL_LIB
#include "ecl_file.h"
#endif //USE_ECL_LIB

//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifReaderEclipseFileAccess : public cvf::Object
{
public:
    RifReaderEclipseFileAccess();
    virtual ~RifReaderEclipseFileAccess();

    bool                open(const QString& fileName);
    void                close();

    size_t              numOccurrences(const QString& keyword);
    bool                keywordsOnFile(QStringList* keywords, size_t numDataItems = cvf::UNDEFINED_SIZE_T, size_t numSteps = cvf::UNDEFINED_SIZE_T);

    bool                timeStepsText(QStringList* timeSteps);
    bool                timeSteps(QList<QDateTime>* timeSteps);

    bool                keywordData(const QString& keyword, size_t index, std::vector<double>* values);

#ifdef USE_ECL_LIB
    ecl_file_type*      filePointer();
#endif //USE_ECL_LIB

    // Static methods
    static bool         fileSet(const QString& fileName, QStringList* fileSet);

#ifdef USE_ECL_LIB
    static QString      fileNameByType(const QStringList& fileSet, ecl_file_enum fileType);
    static QStringList  fileNamesByType(const QStringList& fileSet, ecl_file_enum fileType);
#endif //USE_ECL_LIB

protected:
#ifdef USE_ECL_LIB
    ecl_file_type*      m_file;
#endif //USE_ECL_LIB
};

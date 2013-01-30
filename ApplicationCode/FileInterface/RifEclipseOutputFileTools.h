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

#include "ecl_file.h"

#include "RifReaderInterface.h"

//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifEclipseOutputFileTools : public cvf::Object
{
public:
    RifEclipseOutputFileTools();
    virtual ~RifEclipseOutputFileTools();

    bool                open(const QString& fileName, const std::vector<size_t>& matrixActiveCellCounts, const std::vector<size_t>& fractureActiveCellCounts);
    void                close();

    int                 numOccurrences(const QString& keyword);
    
    bool                validKeywords(QStringList* keywords, RifReaderInterface::PorosityModelResultType matrixOrFracture);

    bool                timeStepsText(QStringList* timeSteps);
    bool                timeSteps(QList<QDateTime>* timeSteps);

    bool                keywordData(const QString& keyword, size_t fileKeywordOccurrence,
                                    RifReaderInterface::PorosityModelResultType matrixOrFracture,
                                    std::vector<double>* values);

    ecl_file_type*      filePointer();

    // Static methods
    static bool         fileSet(const QString& fileName, QStringList* fileSet);

    static QString      fileNameByType(const QStringList& fileSet, ecl_file_enum fileType);
    static QStringList  fileNamesByType(const QStringList& fileSet, ecl_file_enum fileType);

protected:
    ecl_file_type*      m_file;

private:
    std::vector<size_t> m_numMatrixActiveCellCounts;
    std::vector<size_t> m_numFractureActiveCellCount;

    size_t m_globalMatrixActiveCellCounts;
    size_t m_globalFractureActiveCellCounts;
};

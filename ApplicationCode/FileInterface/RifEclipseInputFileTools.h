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
#include <map>

class RigReservoir;
class QString;
class QFile;

//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifEclipseInputFileTools : public cvf::Object
{
public:
    RifEclipseInputFileTools();
    virtual ~RifEclipseInputFileTools();

    static bool openGridFile(const QString& fileName, RigReservoir* reservoir);
    
    // Returns map of assigned resultName and Eclipse Keyword.
    static std::map<QString, QString> readProperties(const QString& fileName, RigReservoir* reservoir);
    static bool                       readProperty  (const QString& fileName, RigReservoir* reservoir, const QString& eclipseKeyWord, const QString& resultName );
    static std::vector<QString> findKeywordsOnFile(const QString &fileName);

    static const std::vector<QString>& knownPropertyKeywords(); 

    static bool     writePropertyToTextFile(const QString& fileName, RigReservoir* reservoir, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord);
    static bool     writeBinaryResultToTextFile(const QString& fileName, RigReservoir* reservoir, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord, const double undefinedValue);

private:
    static void     writeDataToTextFile(QFile* file, const QString& eclipseKeyWord, const std::vector<double>& resultData);
};

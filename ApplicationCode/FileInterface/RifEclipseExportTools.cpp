/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RifEclipseExportTools.h"

#include "RimFracture.h"
#include "RimEclipseWellCollection.h"
#include "RimEclipseView.h"

#include "cafProgressInfo.h"

#include <QTextStream>
#include <QFile>



//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseExportTools::RifEclipseExportTools()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseExportTools::~RifEclipseExportTools()
{
  
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseExportTools::writeFracturesToTextFile(const QString& fileName, RimEclipseWellCollection* wellColl)
{

    std::vector<RimFracture*> fractures;
    wellColl->descendantsIncludingThisOfType(fractures);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    writeFractureDataToTextFile(&file, fractures);


    return true;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::writeFractureDataToTextFile(QFile* file, const std::vector<RimFracture*>& fractures)
{
    QTextStream out(file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n";
    out << "COMPDAT" << "\n" << right << qSetFieldWidth(16);

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(file->fileName()));
    size_t progressSteps = fractures.size() / 20;

    size_t i;
    for (i = 0; i < fractures.size(); i++)
    {
        out << fractures.at(i);
        out << "\n";

        pi.setProgress(i);
    }

    out << "\n" << "/" << "\n";
}


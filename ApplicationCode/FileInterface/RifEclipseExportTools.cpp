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
#include "RimFractureDefinition.h"
#include "RimEclipseWellCollection.h"
#include "RimEclipseView.h"

#include "cafProgressInfo.h"

#include <QTextStream>
#include <QFile>
#include "RimSimWellFracture.h"
#include "RimEclipseWell.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPath.h"



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
bool RifEclipseExportTools::writeSimWellFracturesToTextFile(const QString& fileName, RimEclipseWellCollection* wellColl)
{

    std::vector<RimSimWellFracture*> fractures;
    wellColl->descendantsIncludingThisOfType(fractures);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    writeSimWellFractureDataToTextFile(&file, fractures);


    return true;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseExportTools::writeWellPathFracturesToTextFile(const QString& fileName, RimWellPathCollection* wellColl)
{
    std::vector<RimWellPathFracture*> fractures;
    wellColl->descendantsIncludingThisOfType(fractures);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    writeWellPathFractureDataToTextFile(&file, fractures);


    return true;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::writeSimWellFractureDataToTextFile(QFile* file, const std::vector<RimSimWellFracture*>& fractures)
{
    QTextStream out(file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n\n";
    out << "COMPDAT" << "\n" << right << qSetFieldWidth(8);

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(file->fileName()));

    RimSimWellFracture* fracture;
    RimEclipseWell* simWell = nullptr;

    size_t i;
    std::vector<size_t> ijk;
    for (i = 0; i < fractures.size(); i++)
    {
        fracture = fractures.at(i);
        
        out << qSetFieldWidth(8);
        fracture->firstAncestorOrThisOfType(simWell);
        out << simWell->name;;  // 1. Well name 

        out << qSetFieldWidth(5);
        ijk = fracture->getIJK();
        out << ijk[0];          // 2. I location grid block
        out << ijk[1];          // 3. J location grid block
        out << ijk[2];          // 4. K location of upper connecting grid block
        out << ijk[2];          // 5. K location of lower connecting grid block

        out << "OPEN";          // 6. Open / Shut flag of connection
        out << "1* ";            // 7. Saturation table number for connection rel perm. Default value

        out << qSetFieldWidth(8);
        out << 123456789;         // 8. Transmissitivity //TODO: Calculate value

        out << qSetFieldWidth(4);
        out << "1* ";            // 9. Well bore diameter. Set to default

        out << qSetFieldWidth(8);
        out << fracture->fractureDefinition->effectiveKh(); // 10. Effective Kh (perm times width)
        out << qSetFieldWidth(4);
        out << fracture->fractureDefinition->skinFactor;    // 11. Skin factor

        out << "1*";            // 12. D-factor for handling non-Darcy flow of free gas. Default value. 
        out << "Z";             // 13. Direction well is penetrating the grid block. Z is default. 
        out << "1*";            // 14. Pressure equivalent radius, Default

        out << "/" << "\n";

        pi.setProgress(i);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::writeWellPathFractureDataToTextFile(QFile* file, const std::vector<RimWellPathFracture*>& fractures)
{
    QTextStream out(file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n\n";
    out << "COMPDAT" << "\n" << right << qSetFieldWidth(8);

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(file->fileName()));

    RimWellPathFracture* fracture;
    RimWellPath* wellPath = nullptr;

    size_t i;
    std::vector<size_t> ijk;
    for (i = 0; i < fractures.size(); i++)
    {
        fracture = fractures.at(i);

        out << qSetFieldWidth(8);
        fracture->firstAncestorOrThisOfType(wellPath);
        out << wellPath->name;  // 1. Well name

        out << qSetFieldWidth(5);
        //ijk = fracture->getIJK();
        out << "0";          // 2. I location grid block
        out << "0";          // 3. J location grid block
        out << "0";          // 4. K location of upper connecting grid block
        out << "0";          // 5. K location of lower connecting grid block

        out << "OPEN";          // 6. Open / Shut flag of connection
        out << "1* ";            // 7. Saturation table number for connection rel perm. Default value

        out << qSetFieldWidth(8);
        out << 123456789;         // 8. Transmissitivity //TODO: Calculate value

        out << qSetFieldWidth(4);
        out << "1* ";            // 9. Well bore diameter. Set to default

        out << qSetFieldWidth(8);
        out << fracture->fractureDefinition->effectiveKh(); // 10. Effective Kh (perm times width)
        out << qSetFieldWidth(4);
        out << fracture->fractureDefinition->skinFactor;    // 11. Skin factor

        out << "1*";            // 12. D-factor for handling non-Darcy flow of free gas. Default value. 
        out << "Z";             // 13. Direction well is penetrating the grid block. Z is default. 
        out << "1*";            // 14. Pressure equivalent radius, Default

        out << "/" << "\n";

        pi.setProgress(i);
    }
}


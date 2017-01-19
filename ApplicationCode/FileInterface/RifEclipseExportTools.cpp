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

#include "RiaApplication.h"
#include "RigFracture.h"
#include "RigFracture.h"
#include "RigMainGrid.h"

#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimFracture.h"
#include "RimEllipseFractureTemplate.h"
#include "RimWellPath.h"

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
bool RifEclipseExportTools::writeFracturesToTextFile(const QString& fileName,  const std::vector< RimFracture*>& fractures)
{

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream out(&file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n\n";
    out << "COMPDAT" << "\n" << right << qSetFieldWidth(8);

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(fileName));

    RimEclipseWell* simWell = nullptr;
    RimWellPath* wellPath = nullptr;

    size_t progress =0;
    std::vector<size_t> ijk;

    for (RimFracture* fracture : fractures)
    {
        fracture->computeTransmissibility();
        std::vector<RigFractureData> fracDataVector = fracture->attachedRigFracture()->fractureData();

        for (RigFractureData fracData : fracDataVector)
        {
            out << qSetFieldWidth(8);
            if (fracData.transmissibility == cvf::UNDEFINED_DOUBLE || !(fracture->attachedFractureDefinition())) out << "--"; //Commenting out line in output file

            wellPath, simWell = nullptr;
            fracture->firstAncestorOrThisOfType(simWell);
            if (simWell) out << simWell->name;    // 1. Well name 
            fracture->firstAncestorOrThisOfType(wellPath);
            if (wellPath) out << wellPath->name;  // 1. Well name 

            out << qSetFieldWidth(5);

            RiaApplication* app = RiaApplication::instance();
            RimView* activeView = RiaApplication::instance()->activeReservoirView();
            if (!activeView) return false;
            RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
            if (!activeRiv) return false;

            const RigMainGrid* mainGrid = activeRiv->mainGrid();
            if (!mainGrid) return false;

            size_t i, j, k;
            mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k); 
            out << i+1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
            out << j+1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
            out << k+1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition
            out << k+1;          // 5. K location of lower connecting grid block, adding 1 to go to eclipse 1-based grid definition

            out << "OPEN";          // 6. Open / Shut flag of connection
            out << "1* ";            // 7. Saturation table number for connection rel perm. Default value

            out << qSetFieldWidth(8);
            // 8. Transmissibility 
            if (fracData.transmissibility != cvf::UNDEFINED_DOUBLE) out << fracData.transmissibility;
            else out << "UNDEF";

            out << qSetFieldWidth(4);
            out << "1* ";            // 9. Well bore diameter. Set to default

            out << qSetFieldWidth(8);
            if (fracture->attachedFractureDefinition())
            {
                out << fracture->attachedFractureDefinition()->effectiveKh(); // 10. Effective Kh (perm times width)
                out << qSetFieldWidth(4);
                out << fracture->attachedFractureDefinition()->skinFactor;    // 11. Skin factor
            }
            else //If no attached fracture definition these parameters are set to UNDEF
            {
                out << "UNDEF";
                out << qSetFieldWidth(4);
                out << "UNDEF";  
            }

            out << "1*";            // 12. D-factor for handling non-Darcy flow of free gas. Default value. 
            out << "Z";             // 13. Direction well is penetrating the grid block. Z is default. 
            out << "1*";            // 14. Pressure equivalent radius, Default

            out << "/" << "\n";
        }
        
        progress++;
        pi.setProgress(progress);
    }

    return true;
}

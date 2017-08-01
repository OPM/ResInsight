/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "cafCmdFeature.h"

#include <QString>
#include <vector>
#include "RigFault.h"

class RigMainGrid;

typedef std::tuple<size_t, size_t, size_t, cvf::StructGridInterface::FaceType> FaultCellAndFace;

//==================================================================================================
/// 
//==================================================================================================
class RicExportFaultsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    virtual bool isCommandEnabled();
    virtual void onActionTriggered( bool isChecked );
    virtual void setupActionLook( QAction* actionToSetup );

private:
    static void saveFault(QString completeFilename, const RigMainGrid* mainGrid, const std::vector<RigFault::FaultFace>& faultFaces, QString faultName);
    static bool faultOrdering(FaultCellAndFace first, FaultCellAndFace second);
    static QString faceText(cvf::StructGridInterface::FaceType faceType);
    static void writeLine(QTextStream &stream, QString faultName, size_t i, size_t j, size_t startK, size_t endK, cvf::StructGridInterface::FaceType faceType);
};


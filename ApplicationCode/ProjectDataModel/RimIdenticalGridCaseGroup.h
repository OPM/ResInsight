/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RifReaderInterface.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBase.h"
#include "cvfObject.h"

class RigActiveCellInfo;
class RigMainGrid;

class RimCaseCollection;
class RimEclipseCase;
class RimEclipseStatisticsCase;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimIdenticalGridCaseGroup : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIdenticalGridCaseGroup();
    virtual ~RimIdenticalGridCaseGroup();

    caf::PdmField<QString>              name;
    caf::PdmField<int>                  groupId;
    caf::PdmChildField<RimCaseCollection*>   caseCollection;
    caf::PdmChildField<RimCaseCollection*>   statisticsCaseCollection;

    void                                addCase(RimEclipseCase* reservoir);
    void                                removeCase(RimEclipseCase* reservoir);

    bool                                contains(RimEclipseCase* reservoir) const;

    RimEclipseStatisticsCase*                  createAndAppendStatisticsCase();


    RimEclipseCase*                            mainCase();
    void                                loadMainCaseAndActiveCellInfo();

    RigMainGrid*                        mainGrid();

    RigActiveCellInfo*                  unionOfActiveCells(RiaDefines::PorosityModelType porosityType);
    void                                computeUnionOfActiveCells();

    static bool                         isStatisticsCaseCollection(RimCaseCollection* rimCaseCollection);

protected:
    virtual caf::PdmFieldHandle*        userDescriptionField();

private:
    void                                updateMainGridAndActiveCellsForStatisticsCases();
    void                                clearStatisticsResults();
    void                                clearActiveCellUnions();

private:
    RigMainGrid*                        m_mainGrid;
    
    cvf::ref<RigActiveCellInfo>         m_unionOfMatrixActiveCells;
    cvf::ref<RigActiveCellInfo>         m_unionOfFractureActiveCells;
};

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

#include "RiaEclipseUnitTools.h"

#include "cvfAssert.h"
#include "cvfArray.h"
#include "cvfObject.h"
#include "cvfCollection.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <vector>

class RigCaseCellResultsData;
class RigFormationNames;
class RigMainGrid;
class RigGridBase;
class RigCaseCellResultsData;
class RigActiveCellInfo;
class RigSingleWellResultsData;
class RigCell;

struct RigWellResultPoint;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigEclipseCaseData : public cvf::Object
{
public:
    RigEclipseCaseData();
    ~RigEclipseCaseData();

    RigMainGrid*                                mainGrid();
    const RigMainGrid*                          mainGrid() const;
    void                                        setMainGrid(RigMainGrid* mainGrid);

    void                                        allGrids(std::vector<RigGridBase*>* grids); // To be removed
    void                                        allGrids(std::vector<const RigGridBase*>* grids) const;// To be removed
    const RigGridBase*                          grid(size_t index) const;
    RigGridBase*                                grid(size_t index);
    size_t                                      gridCount() const;

    RigCaseCellResultsData*                     results(RifReaderInterface::PorosityModelResultType porosityModel);
    const RigCaseCellResultsData*               results(RifReaderInterface::PorosityModelResultType porosityModel) const;

    RigActiveCellInfo*                          activeCellInfo(RifReaderInterface::PorosityModelResultType porosityModel);
    const RigActiveCellInfo*                    activeCellInfo(RifReaderInterface::PorosityModelResultType porosityModel) const;
    void                                        setActiveCellInfo(RifReaderInterface::PorosityModelResultType porosityModel, RigActiveCellInfo* activeCellInfo);

    void                                        setActiveFormationNames(RigFormationNames* activeFormationNames);
    RigFormationNames*                          activeFormationNames(); 

    void                                        setWellResults(const cvf::Collection<RigSingleWellResultsData>& data);
    const cvf::Collection<RigSingleWellResultsData>&      wellResults() { return m_wellResults; }
    const RigSingleWellResultsData*             findWellResult(QString wellName) const;
    
    const cvf::UByteArray*                      wellCellsInGrid(size_t gridIndex);
    const cvf::UIntArray*                       gridCellToResultWellIndex(size_t gridIndex);

    const RigCell&                              cellFromWellResultCell(const RigWellResultPoint& wellResultCell) const;
    bool                                        findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace, const RigWellResultPoint& sourceWellCellResult, const RigWellResultPoint& otherWellCellResult) const;

    void                                        computeActiveCellBoundingBoxes();

    RiaEclipseUnitTools::UnitSystem             unitsType() const                   { return m_unitsType; }
    void                                        setUnitsType(RiaEclipseUnitTools::UnitSystem unitsType)   { m_unitsType = unitsType; }

private:
    void                                        computeActiveCellIJKBBox();
    void                                        computeWellCellsPrGrid();
    void                                        computeActiveCellsGeometryBoundingBox();

private:
    cvf::ref<RigMainGrid>                       m_mainGrid;

    cvf::ref<RigActiveCellInfo>                 m_activeCellInfo;
    cvf::ref<RigActiveCellInfo>                 m_fractureActiveCellInfo;

    cvf::ref<RigCaseCellResultsData>            m_matrixModelResults;
    cvf::ref<RigCaseCellResultsData>            m_fractureModelResults;

    cvf::ref<RigFormationNames>                 m_activeFormationNamesData;

    cvf::Collection<RigSingleWellResultsData>   m_wellResults;     //< A WellResults object for each well in the reservoir
    cvf::Collection<cvf::UByteArray>            m_wellCellsInGrid; //< A bool array pr grid with one bool pr cell telling wether the cell is a well cell or not
    cvf::Collection<cvf::UIntArray>             m_gridCellToResultWellIndex; //< Array pr grid with index to well pr cell telling which well a cell is in

    RiaEclipseUnitTools::UnitSystem             m_unitsType;
};

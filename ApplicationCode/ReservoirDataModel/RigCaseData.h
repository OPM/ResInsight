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

#include <vector>
#include "RigCell.h"
#include "cvfVector3.h"
#include "cvfAssert.h"
#include "cvfObject.h"
#include "RigMainGrid.h"
#include "RigSingleWellResultsData.h"
#include "RigActiveCellInfo.h"

class RigCaseCellResultsData;


class RigCaseData: public cvf::Object
{
public:
    RigCaseData();
    ~RigCaseData();

    RigMainGrid*                                mainGrid() { return m_mainGrid.p(); }
    const RigMainGrid*                          mainGrid() const { return m_mainGrid.p(); }
    void                                        setMainGrid(RigMainGrid* mainGrid);

    void                                        allGrids(std::vector<RigGridBase*>* grids); // To be removed
    void                                        allGrids(std::vector<const RigGridBase*>* grids) const;// To be removed
    const RigGridBase*                          grid(size_t index) const;
    RigGridBase*                                grid(size_t index);
    size_t                                      gridCount() const;

    RigCaseCellResultsData*		            results(RifReaderInterface::PorosityModelResultType porosityModel);
    const RigCaseCellResultsData*              results(RifReaderInterface::PorosityModelResultType porosityModel) const;

    RigActiveCellInfo*                          activeCellInfo(RifReaderInterface::PorosityModelResultType porosityModel);
    const RigActiveCellInfo*                    activeCellInfo(RifReaderInterface::PorosityModelResultType porosityModel) const;
    void                                        setActiveCellInfo(RifReaderInterface::PorosityModelResultType porosityModel, RigActiveCellInfo* activeCellInfo);
    

    cvf::ref<cvf::StructGridScalarDataAccess>   dataAccessObject(const RigGridBase* grid, 
                                                               RifReaderInterface::PorosityModelResultType porosityModel, 
                                                               size_t timeStepIndex, 
                                                               size_t scalarSetIndex);

    void                                        setWellResults(const cvf::Collection<RigSingleWellResultsData>& data);
    const cvf::Collection<RigSingleWellResultsData>&      wellResults() { return m_wellResults; }

    cvf::UByteArray*                            wellCellsInGrid(size_t gridIndex);

    RigCell&                                    cellFromWellResultCell(const RigWellResultCell& wellResultCell);
    bool                                        findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace, const RigWellResultCell& sourceWellCellResult, const RigWellResultCell& otherWellCellResult) const;

    void                                        computeActiveCellBoundingBoxes();

private:
    void                                        computeActiveCellIJKBBox();
    void                                        computeWellCellsPrGrid();
    void                                        computeActiveCellsGeometryBoundingBox();

private:
    cvf::ref<RigMainGrid>                       m_mainGrid;

    cvf::ref<RigActiveCellInfo>                 m_activeCellInfo;
    cvf::ref<RigActiveCellInfo>                 m_fractureActiveCellInfo;

    cvf::ref<RigCaseCellResultsData>           m_matrixModelResults;
    cvf::ref<RigCaseCellResultsData>           m_fractureModelResults;

    cvf::Collection<RigSingleWellResultsData>             m_wellResults;     //< A WellResults object for each well in the reservoir
    cvf::Collection<cvf::UByteArray>            m_wellCellsInGrid; //< A bool array pr grid with one bool pr cell telling wether the cell is a well cell or not
};

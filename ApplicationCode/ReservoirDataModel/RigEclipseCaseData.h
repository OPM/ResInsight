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
#include "cvfCollection.h"

#include <vector>
#include <map>
#include <set>

class RigCaseCellResultsData;
class RigFormationNames;
class RigMainGrid;
class RigGridBase;
class RigCaseCellResultsData;
class RigActiveCellInfo;
class RigSimWellData;
class RigCell;
class RigWellPath;
class RimEclipseCase;
class RigVirtualPerforationTransmissibilities;

struct RigWellResultPoint;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigEclipseCaseData : public cvf::Object
{
public:
    explicit RigEclipseCaseData(RimEclipseCase* ownerCase);
    ~RigEclipseCaseData();

    RimEclipseCase*                             ownerCase() const { return m_ownerCase; }

    RigMainGrid*                                mainGrid();
    const RigMainGrid*                          mainGrid() const;
    void                                        setMainGrid(RigMainGrid* mainGrid);

    void                                        allGrids(std::vector<RigGridBase*>* grids); // To be removed
    void                                        allGrids(std::vector<const RigGridBase*>* grids) const;// To be removed
    const RigGridBase*                          grid(size_t index) const;
    RigGridBase*                                grid(size_t index);
    size_t                                      gridCount() const;

    RigCaseCellResultsData*                     results(RiaDefines::PorosityModelType porosityModel);
    const RigCaseCellResultsData*               results(RiaDefines::PorosityModelType porosityModel) const;
    const std::vector<double>*                  resultValues(RiaDefines::PorosityModelType porosityModel, 
                                                             RiaDefines::ResultCatType type, 
                                                             const QString& resultName, 
                                                             size_t timeStepIndex);

    RigActiveCellInfo*                          activeCellInfo(RiaDefines::PorosityModelType porosityModel);
    const RigActiveCellInfo*                    activeCellInfo(RiaDefines::PorosityModelType porosityModel) const;
    void                                        setActiveCellInfo(RiaDefines::PorosityModelType porosityModel, RigActiveCellInfo* activeCellInfo);

    bool                                        hasFractureResults() const;

    void                                        setActiveFormationNames(RigFormationNames* activeFormationNames);
    RigFormationNames*                          activeFormationNames(); 

    void                                        setSimWellData(const cvf::Collection<RigSimWellData>& data);
    const cvf::Collection<RigSimWellData>&      wellResults() const { return m_simWellData; }
    std::set<QString>                           findSortedWellNames() const;
    const RigSimWellData*                       findSimWellData(QString wellName) const;
    
    const cvf::UByteArray*                      wellCellsInGrid(size_t gridIndex);
    const cvf::UIntArray*                       gridCellToResultWellIndex(size_t gridIndex);

    const RigCell&                              cellFromWellResultCell(const RigWellResultPoint& wellResultCell) const;
    bool                                        findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace, const RigWellResultPoint& sourceWellCellResult, const RigWellResultPoint& otherWellCellResult) const;

    void                                        computeActiveCellBoundingBoxes();

    RiaEclipseUnitTools::UnitSystem             unitsType() const                   { return m_unitsType; }
    void                                        setUnitsType(RiaEclipseUnitTools::UnitSystem unitsType)   { m_unitsType = unitsType; }

    std::vector<QString>                        simulationWellNames() const;
    bool                                        hasSimulationWell(const QString& simWellName) const;

    std::vector<const RigWellPath*>             simulationWellBranches(const QString& simWellName,
                                                                       bool includeAllCellCenters,
                                                                       bool useAutoDetectionOfBranches) const;

    void                                        setVirtualPerforationTransmissibilities(RigVirtualPerforationTransmissibilities* virtualPerforationTransmissibilities);
    const RigVirtualPerforationTransmissibilities*    virtualPerforationTransmissibilities() const;

private:
    void                                        computeActiveCellIJKBBox();
    void                                        computeWellCellsPrGrid();
    void                                        computeActiveCellsGeometryBoundingBox();

private:
    cvf::ref<RigMainGrid>                       m_mainGrid;
    RimEclipseCase*                             m_ownerCase;

    cvf::ref<RigActiveCellInfo>                 m_activeCellInfo;
    cvf::ref<RigActiveCellInfo>                 m_fractureActiveCellInfo;

    cvf::ref<RigCaseCellResultsData>            m_matrixModelResults;
    cvf::ref<RigCaseCellResultsData>            m_fractureModelResults;

    cvf::ref<RigFormationNames>                 m_activeFormationNamesData;

    RigVirtualPerforationTransmissibilities*    m_virtualPerforationTransmissibilities;

    cvf::Collection<RigSimWellData>             m_simWellData;     //< A WellResults object for each well in the reservoir
    cvf::Collection<cvf::UByteArray>            m_wellCellsInGrid; //< A bool array pr grid with one bool pr cell telling whether the cell is a well cell or not
    cvf::Collection<cvf::UIntArray>             m_gridCellToResultWellIndex; //< Array pr grid with index to well pr cell telling which well a cell is in

    RiaEclipseUnitTools::UnitSystem             m_unitsType;

    mutable std::map<std::tuple<QString, bool, bool>, cvf::Collection<RigWellPath>> m_simWellBranchCache;
};

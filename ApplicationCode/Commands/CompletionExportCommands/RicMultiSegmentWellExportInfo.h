/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RiaEclipseUnitTools.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <QString>

class RimWellPath;
class RimFishbonesMultipleSubs;

//==================================================================================================
/// 
//==================================================================================================
struct RicWellSegmentLateralIntersection
{
    RicWellSegmentLateralIntersection(size_t             globalCellIndex,
                                      const cvf::Vec3st& cellIJK,
                                      double             length,
                                      double             depth,
                                      const cvf::Vec3d&  lengthsInCell)
        : segmentNumber(std::numeric_limits<int>::infinity())
        , attachedSegmentNumber(std::numeric_limits<int>::infinity())
        , globalCellIndex(globalCellIndex)
        , cellIJK(cellIJK)
        , mdFromPreviousIntersection(length)
        , tvdChangeFromPreviousIntersection(depth)
        , lengthsInCell(lengthsInCell)
        , mainBoreCell(false)
    {
    }
    int         segmentNumber;
    int         attachedSegmentNumber;
    size_t      globalCellIndex;
    cvf::Vec3st cellIJK;
    bool        mainBoreCell;
    double      mdFromPreviousIntersection;
    double      tvdChangeFromPreviousIntersection;
    cvf::Vec3d  lengthsInCell;
};

//==================================================================================================
/// 
//==================================================================================================
struct RicWellSegmentLateral
{
    RicWellSegmentLateral(size_t lateralIndex)
        : lateralIndex(lateralIndex)
        , branchNumber(0)
    {
    }

    size_t                                      lateralIndex;
    int                                         branchNumber;
    std::vector<RicWellSegmentLateralIntersection> intersections;
};

//==================================================================================================
/// 
//==================================================================================================
struct RicWellSegmentLocation
{
    RicWellSegmentLocation(const QString& label,
                           double measuredDepth,
                           double trueVerticalDepth,
                           size_t subIndex,
                           int segmentNumber = -1)
        : label(label)
        , measuredDepth(measuredDepth)
        , trueVerticalDepth(trueVerticalDepth)
        , effectiveDiameter(0.15)
        , holeDiameter(std::numeric_limits<double>::infinity())
        , openHoleRoughnessFactor(5.0e-5)
        , skinFactor(std::numeric_limits<double>::infinity())
        , icdFlowCoefficient(std::numeric_limits<double>::infinity())
        , icdArea(std::numeric_limits<double>::infinity())
        , subIndex(subIndex)
        , segmentNumber(segmentNumber)
        , icdBranchNumber(-1)
        , icdSegmentNumber(-1)
    {
    }

    bool operator<(const RicWellSegmentLocation& rhs) const;
    
    QString                         label;
    double                          measuredDepth;
    double                          trueVerticalDepth;
    double                          effectiveDiameter;
    double                          holeDiameter;
    double                          linerDiameter;
    double                          openHoleRoughnessFactor;
    double                          skinFactor;
    double                          icdFlowCoefficient;
    double                          icdArea;

    size_t                          subIndex;
    int                             segmentNumber;
    int                             icdBranchNumber;
    int                             icdSegmentNumber;

    std::vector<RicWellSegmentLateral> laterals;
};

class RicMultiSegmentWellExportInfo
{
public:
    RicMultiSegmentWellExportInfo(const RimWellPath*              wellPath,
                                  RiaEclipseUnitTools::UnitSystem unitSystem,
                                  double                          initialMD,
                                  const QString&                  lengthAndDepthText,
                                  const QString&                  pressureDropText)
        : m_wellPath(wellPath)
        , m_initialMD(initialMD)
        , m_unitSystem(unitSystem)
        , m_topWellBoreVolume(std::numeric_limits<double>::infinity())
        , m_linerDiameter(0.15)
        , m_roughnessFactor(5.0e-5)
        , m_lengthAndDepthText(lengthAndDepthText)
        , m_pressureDropText(pressureDropText)
    {}

    void setTopWellBoreVolume(double topWellBoreVolume);
    void setLinerDiameter(double linerDiameter);
    void setRoughnessFactor(double roughnessFactor);
    void addWellSegmentLocation(const RicWellSegmentLocation& location);
    void sortLocations();

    const RimWellPath*              wellPath() const;
    RiaEclipseUnitTools::UnitSystem unitSystem() const;
    double                          initialMD() const;
    double                          initialTVD() const;
    double                          topWellBoreVolume() const;
    double                          linerDiameter() const;
    double                          roughnessFactor() const;
    QString                         lengthAndDepthText() const;
    QString                         pressureDropText() const;

    const std::vector<RicWellSegmentLocation>& wellSegmentLocations() const;
    std::vector<RicWellSegmentLocation>& wellSegmentLocations();

protected:
    const RimWellPath*                   m_wellPath;
    RiaEclipseUnitTools::UnitSystem      m_unitSystem;
    double                               m_initialMD;
    double                               m_topWellBoreVolume;
    double                               m_linerDiameter;
    double                               m_roughnessFactor;
    QString                              m_lengthAndDepthText;
    QString                              m_pressureDropText;

    std::vector<RicWellSegmentLocation> m_wellSegmentLocations;
};


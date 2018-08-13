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
class RicWellSegmentLateralIntersection
{
public:
    RicWellSegmentLateralIntersection(const QString& gridName, // Pass in empty string for main grid
                                      size_t gridLocalCellIndex,
                                      const cvf::Vec3st& gridLocalCellIJK,
                                      double startMD,
                                      double deltaMD,
                                      double startTVD,
                                      double deltaTVD,
                                      const cvf::Vec3d& lengthsInCell);

    const QString&    gridName() const;
    size_t            gridLocalCellIndex() const;
    cvf::Vec3st       gridLocalCellIJK() const;
    double            startMD() const;
    double            deltaMD() const;
    double            startTVD() const;
    double            deltaTVD() const;
    const cvf::Vec3d& lengthsInCell() const;

    int               segmentNumber() const;
    int               attachedSegmentNumber() const;
    bool              isMainBoreCell() const;

    void              setSegmentNumber(int segmentNumber);
    void              setAttachedSegmentNumber(int attachedSegmentNumber);
    void              setIsMainBoreCell(bool isMainBoreCell);

private:
    QString     m_gridName;
    size_t      m_gridLocalCellIndex;
    cvf::Vec3st m_gridLocalCellIJK;
    double      m_startMD;
    double      m_deltaMD;
    double      m_startTVD;
    double      m_deltaTVD;
    cvf::Vec3d  m_lengthsInCell;
    int         m_segmentNumber;
    int         m_attachedSegmentNumber;
    bool        m_mainBoreCell;
};

//==================================================================================================
/// 
//==================================================================================================
class RicWellSegmentLateral
{
public:
    RicWellSegmentLateral(size_t lateralIndex, int branchNumber = 0);

    size_t                                                lateralIndex() const;
    int                                                   branchNumber() const;
    void                                                  setBranchNumber(int branchNumber);

    void                                                  addIntersection(const RicWellSegmentLateralIntersection& intersection);
    std::vector<RicWellSegmentLateralIntersection>&       intersections();
    const std::vector<RicWellSegmentLateralIntersection>& intersections() const;
    
private:
    size_t                                         m_lateralIndex;
    int                                            m_branchNumber;
    std::vector<RicWellSegmentLateralIntersection> m_intersections;
};

//==================================================================================================
/// 
//==================================================================================================
class RicWellSegmentLocation
{
public:
    RicWellSegmentLocation(const QString& label,
                           double measuredDepth,
                           double trueVerticalDepth,
                           size_t subIndex,
                           int segmentNumber = -1);

    QString label() const;
    double  measuredDepth() const;
    double  trueVerticalDepth() const;
    double  effectiveDiameter() const;
    double  holeDiameter() const;
    double  openHoleRoughnessFactor() const;
    double  skinFactor() const;
    double  icdFlowCoefficient() const;
    double  icdArea() const;
    
    size_t  subIndex() const;
    int     segmentNumber() const;
    int     icdBranchNumber() const;
    int     icdSegmentNumber() const;

    const std::vector<RicWellSegmentLateral>& laterals() const;
    std::vector<RicWellSegmentLateral>&       laterals();

    void setEffectiveDiameter(double effectiveDiameter);
    void setHoleDiameter(double holeDiameter);
    void setOpenHoleRoughnessFactor(double roughnessFactor);
    void setSkinFactor(double skinFactor);
    void setIcdFlowCoefficient(double icdFlowCoefficient);
    void setIcdArea(double icdArea);
    void setSegmentNumber(int segmentNumber);
    void setIcdBranchNumber(int icdBranchNumber);
    void setIcdSegmentNumber(int icdSegmentNumber);    
    void addLateral(const RicWellSegmentLateral& lateral);

    bool operator<(const RicWellSegmentLocation& rhs) const;
    
private:
    QString                         m_label;
    double                          m_measuredDepth;
    double                          m_trueVerticalDepth;
    double                          m_effectiveDiameter;
    double                          m_holeDiameter;
    double                          m_linerDiameter;
    double                          m_openHoleRoughnessFactor;
    double                          m_skinFactor;
    double                          m_icdFlowCoefficient;
    double                          m_icdArea;

    size_t                          m_subIndex;
    int                             m_segmentNumber;
    int                             m_icdBranchNumber;
    int                             m_icdSegmentNumber;

    std::vector<RicWellSegmentLateral> m_laterals;
};

class RicMultiSegmentWellExportInfo
{
public:
    RicMultiSegmentWellExportInfo(const RimWellPath*              wellPath,
                                  RiaEclipseUnitTools::UnitSystem unitSystem,
                                  double                          initialMD,
                                  const QString&                  lengthAndDepthText,
                                  const QString&                  pressureDropText);
    
    void setTopWellBoreVolume(double topWellBoreVolume);
    void setLinerDiameter(double linerDiameter);
    void setRoughnessFactor(double roughnessFactor);
    void setHasSubGridIntersections(bool subGridIntersections);

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
    bool                            hasSubGridIntersections() const;
    static double                   defaultDoubleValue();

    const std::vector<RicWellSegmentLocation>& wellSegmentLocations() const;
    std::vector<RicWellSegmentLocation>&       wellSegmentLocations();

private:
    const RimWellPath*                   m_wellPath;
    RiaEclipseUnitTools::UnitSystem      m_unitSystem;
    double                               m_initialMD;
    double                               m_topWellBoreVolume;
    double                               m_linerDiameter;
    double                               m_roughnessFactor;
    QString                              m_lengthAndDepthText;
    QString                              m_pressureDropText;
    bool                                 m_hasSubGridIntersections;

    std::vector<RicWellSegmentLocation> m_wellSegmentLocations;
};


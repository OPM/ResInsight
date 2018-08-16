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
#include "RigCompletionData.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <QString>

class RimWellPath;
class RimFishbonesMultipleSubs;

//==================================================================================================
/// 
//==================================================================================================
class RicWellSegmentSubSegmentIntersection
{
public:
    RicWellSegmentSubSegmentIntersection(const QString& gridName, // Pass in empty string for main grid
                                         size_t globalCellIndex,
                                         const cvf::Vec3st& gridLocalCellIJK,
                                         const cvf::Vec3d& lengthsInCell);
    const QString&    gridName() const;
    size_t            globalCellIndex() const;
    cvf::Vec3st       gridLocalCellIJK() const;
    const cvf::Vec3d& lengthsInCell() const;
private:
    QString     m_gridName;
    size_t      m_globalCellIndex;
    cvf::Vec3st m_gridLocalCellIJK;
    cvf::Vec3d  m_lengthsInCell;
};

//==================================================================================================
/// 
//==================================================================================================
class RicWellSegmentSubSegment
{
public:
    RicWellSegmentSubSegment(double startMD,
                             double deltaMD,
                             double startTVD,
                             double deltaTVD);

    double            startMD() const;
    double            deltaMD() const;
    double            startTVD() const;
    double            deltaTVD() const;

    int               segmentNumber() const;
    int               attachedSegmentNumber() const;

    void              setSegmentNumber(int segmentNumber);
    void              setAttachedSegmentNumber(int attachedSegmentNumber);
    void              addIntersection(const RicWellSegmentSubSegmentIntersection& intersection);

    const std::vector<RicWellSegmentSubSegmentIntersection>& intersections() const;
    std::vector<RicWellSegmentSubSegmentIntersection>&       intersections();
    

private:
    double      m_startMD;
    double      m_deltaMD;
    double      m_startTVD;
    double      m_deltaTVD;
    int         m_segmentNumber;
    int         m_attachedSegmentNumber;

    std::vector<RicWellSegmentSubSegmentIntersection> m_intersections;
};

//==================================================================================================
/// 
//==================================================================================================
class RicWellSegmentCompletion
{
public:
    RicWellSegmentCompletion(RigCompletionData::CompletionType completionType, size_t index = cvf::UNDEFINED_SIZE_T, int branchNumber = cvf::UNDEFINED_INT);

    RigCompletionData::CompletionType              completionType() const;
    size_t                                         index() const;
    int                                            branchNumber() const;
    void                                           setBranchNumber(int branchNumber);

    void                                           addSubSegment(const RicWellSegmentSubSegment& subSegment);
    std::vector<RicWellSegmentSubSegment>&         subSegments();
    const std::vector<RicWellSegmentSubSegment>&   subSegments() const;
    
private:
    RigCompletionData::CompletionType      m_completionType;
    size_t                                 m_index;
    int                                    m_branchNumber;
    std::vector<RicWellSegmentSubSegment>  m_subSegments;
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
                           size_t subIndex = cvf::UNDEFINED_SIZE_T,
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

    const std::vector<RicWellSegmentCompletion>& completions() const;
    std::vector<RicWellSegmentCompletion>&       completions();

    void setEffectiveDiameter(double effectiveDiameter);
    void setHoleDiameter(double holeDiameter);
    void setOpenHoleRoughnessFactor(double roughnessFactor);
    void setSkinFactor(double skinFactor);
    void setIcdFlowCoefficient(double icdFlowCoefficient);
    void setIcdArea(double icdArea);
    void setSegmentNumber(int segmentNumber);
    void addCompletion(const RicWellSegmentCompletion& completion);

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

    std::vector<RicWellSegmentCompletion> m_completions;
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


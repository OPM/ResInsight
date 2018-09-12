/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

class RimWellPath;
class RivIntersectionPartMgr;
class RimSimWellInView;
class RimSimWellInViewCollection;
class Rim2dIntersectionView;

namespace caf 
{
    class PdmUiListEditorAttribute;
    class PdmUiPushButtonEditorAttribute;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RimIntersection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum CrossSectionEnum
    {
        CS_WELL_PATH,
        CS_SIMULATION_WELL,
        CS_POLYLINE,
        CS_AZIMUTHLINE
    };

    enum CrossSectionDirEnum
    {
        CS_VERTICAL,
        CS_HORIZONTAL,
        CS_TWO_POINTS,
    };

public:
    RimIntersection();
    ~RimIntersection();

    caf::PdmField<QString>                               name;
    caf::PdmField<bool>                                  isActive;

    caf::PdmField< caf::AppEnum< CrossSectionEnum > >    type;
    caf::PdmField< caf::AppEnum< CrossSectionDirEnum > > direction;
    caf::PdmField< bool >                                showInactiveCells;

    caf::PdmPtrField<RimWellPath*>                       wellPath;
    caf::PdmPtrField<RimSimWellInView*>                  simulationWell;

    caf::PdmField< bool >                                inputPolyLineFromViewerEnabled;
    caf::PdmField< bool >                                inputExtrusionPointsFromViewerEnabled;
    caf::PdmField< bool >                                inputTwoAzimuthPointsFromViewerEnabled;

    std::vector< std::vector <cvf::Vec3d> >              polyLines(cvf::Vec3d * flattenedPolylineStartPoint = nullptr) const;
    void                                                 appendPointToPolyLine(const cvf::Vec3d& point);

    Rim2dIntersectionView*                               correspondingIntersectionView();
    RivIntersectionPartMgr*                              intersectionPartMgr();

    std::vector <cvf::Vec3d>                             polyLinesForExtrusionDirection() const;
    void                                                 appendPointToExtrusionDirection(const cvf::Vec3d& point);

    void                                                 appendPointToAzimuthLine(const cvf::Vec3d& point);

    cvf::Vec3d                                           extrusionDirection() const;
    double                                               lengthUp() const;
    double                                               lengthDown() const;
    void                                                 setLengthUp(double heightUp);
    void                                                 setLengthDown(double heightDown);
    double                                               extentLength();
    void                                                 recomputeSimulationWellBranchData();
    bool                                                 hasDefiningPoints() const;

    int                                                  branchIndex() const;
    void                                                 rebuildGeometryAndScheduleCreateDisplayModel();

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField();
    virtual caf::PdmFieldHandle*            objectToggleField();
                                            
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);


    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);
                                            
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);

private:

private:                                    
    caf::PdmField<int>                      m_branchIndex;
    caf::PdmField<double>                   m_extentLength;
    caf::PdmField<double>                   m_azimuthAngle;
    caf::PdmField<double>                   m_dipAngle;
    caf::PdmField<double>                   m_lengthUp;
    caf::PdmField<double>                   m_lengthDown;

    caf::PdmField< std::vector< cvf::Vec3d> >  m_userPolyline;
    caf::PdmField< std::vector< cvf::Vec3d> >  m_customExtrusionPoints;
    caf::PdmField< std::vector< cvf::Vec3d> >  m_twoAzimuthPoints;

    static void                             setPushButtonText(bool buttonEnable, caf::PdmUiPushButtonEditorAttribute* attribute);
    static void                             setBaseColor(bool enable, caf::PdmUiListEditorAttribute* attribute);

    RimSimWellInViewCollection*             simulationWellCollection() const;
    void                                    updateAzimuthLine();
    void                                    updateSimulationWellCenterline() const;
    void                                    updateWellExtentDefaultValue();
    void                                    addExtents(std::vector<cvf::Vec3d> &polyLine) const;
    void                                    updateName();
    static double                           azimuthInRadians(cvf::Vec3d vec);
private:                                    
    cvf::ref<RivIntersectionPartMgr>        m_crossSectionPartMgr;
    
    mutable 
    std::vector< std::vector <cvf::Vec3d> > m_simulationWellBranchCenterlines;
};

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

#include "RimIntersection.h"

#include "cafPdmPtrArrayField.h"

class RimWellPath;
class RivExtrudedCurveIntersectionPartMgr;
class RimIntersectionResultDefinition;
class RimSimWellInView;
class RimSimWellInViewCollection;
class Rim2dIntersectionView;
class RimSurface;

namespace caf
{
class PdmUiListEditorAttribute;
class PdmUiPushButtonEditorAttribute;
} // namespace caf

//==================================================================================================
//
//
//
//==================================================================================================
class RimExtrudedCurveIntersection : public RimIntersection
{
    CAF_PDM_HEADER_INIT;

public:
    enum class CrossSectionEnum
    {
        CS_WELL_PATH,
        CS_SIMULATION_WELL,
        CS_POLYLINE,
        CS_AZIMUTHLINE
    };

    enum class CrossSectionDirEnum
    {
        CS_VERTICAL,
        CS_HORIZONTAL,
        CS_TWO_POINTS,
    };

public:
    RimExtrudedCurveIntersection();
    ~RimExtrudedCurveIntersection() override;

    QString name() const override;
    void    setName( const QString& newName );

    RimExtrudedCurveIntersection::CrossSectionEnum    type() const;
    RimExtrudedCurveIntersection::CrossSectionDirEnum direction() const;

    RimWellPath*      wellPath() const;
    RimSimWellInView* simulationWell() const;
    bool              inputPolyLineFromViewerEnabled() const;
    bool              inputExtrusionPointsFromViewerEnabled() const;
    bool              inputTwoAzimuthPointsFromViewerEnabled() const;

    void configureForSimulationWell( RimSimWellInView* simWell );
    void configureForWellPath( RimWellPath* wellPath );
    void configureForPolyLine();
    void configureForAzimuthLine();

    std::vector<std::vector<cvf::Vec3d>> polyLines( cvf::Vec3d* flattenedPolylineStartPoint = nullptr ) const;
    void                                 appendPointToPolyLine( const cvf::Vec3d& point );

    Rim2dIntersectionView*                    correspondingIntersectionView() const;
    RivExtrudedCurveIntersectionPartMgr*      intersectionPartMgr();
    void                                      rebuildGeometry();
    const RivIntersectionGeometryGeneratorIF* intersectionGeometryGenerator() const override;

    std::vector<cvf::Vec3d> polyLinesForExtrusionDirection() const;
    void                    appendPointToExtrusionDirection( const cvf::Vec3d& point );

    void appendPointToAzimuthLine( const cvf::Vec3d& point );

    cvf::Vec3d extrusionDirection() const;
    double     lengthUp() const;
    double     lengthDown() const;
    void       setLengthUp( double heightUp );
    void       setLengthDown( double heightDown );
    double     extentLength();
    void       recomputeSimulationWellBranchData();
    bool       hasDefiningPoints() const;

    std::vector<RimSurface*> annotatedSurfaces() const;

    int  branchIndex() const;
    void rebuildGeometryAndScheduleCreateDisplayModel();

protected:
    caf::PdmFieldHandle*          userDescriptionField() final;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    static void setPushButtonText( bool buttonEnable, caf::PdmUiPushButtonEditorAttribute* attribute );
    static void setBaseColor( bool enable, caf::PdmUiListEditorAttribute* attribute );

    RimSimWellInViewCollection* simulationWellCollection() const;
    void                        updateAzimuthLine();
    void                        updateSimulationWellCenterline() const;
    void                        addExtents( std::vector<cvf::Vec3d>& polyLine ) const;
    void                        updateName();
    static double               azimuthInRadians( cvf::Vec3d vec );

private:
    caf::PdmField<QString> m_name;

    caf::PdmField<caf::AppEnum<CrossSectionEnum>>    m_type;
    caf::PdmField<caf::AppEnum<CrossSectionDirEnum>> m_direction;

    caf::PdmPtrField<RimWellPath*>      m_wellPath;
    caf::PdmPtrField<RimSimWellInView*> m_simulationWell;

    caf::PdmField<bool> m_inputPolyLineFromViewerEnabled;
    caf::PdmField<bool> m_inputExtrusionPointsFromViewerEnabled;
    caf::PdmField<bool> m_inputTwoAzimuthPointsFromViewerEnabled;

    caf::PdmField<int>    m_branchIndex;
    caf::PdmField<double> m_extentLength;
    caf::PdmField<double> m_azimuthAngle;
    caf::PdmField<double> m_dipAngle;
    caf::PdmField<double> m_lengthUp;
    caf::PdmField<double> m_lengthDown;

    caf::PdmField<std::vector<cvf::Vec3d>> m_userPolyline;
    caf::PdmField<std::vector<cvf::Vec3d>> m_customExtrusionPoints;
    caf::PdmField<std::vector<cvf::Vec3d>> m_twoAzimuthPoints;

    // Surface intersection annotations
    caf::PdmPtrArrayField<RimSurface*> m_annotationSurfaces;

    cvf::ref<RivExtrudedCurveIntersectionPartMgr> m_crossSectionPartMgr;

    mutable std::vector<std::vector<cvf::Vec3d>> m_simulationWellBranchCenterlines;
};

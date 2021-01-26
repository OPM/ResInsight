/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2018 Statoil ASA
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaDefines.h"

#include "Rim3dPropertiesInterface.h"
#include "RimCheckableNamedObject.h"
#include "RimWellPathComponentInterface.h"

#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimEllipseFractureTemplate;
class RivWellFracturePartMgr;
class RimFractureTemplate;
class RigFracturedEclipseCellExportData;
class RigMainGrid;
class RigFractureGrid;

class NonDarcyData
{
public:
    NonDarcyData()
        : width( std::numeric_limits<double>::infinity() )
        , conductivity( std::numeric_limits<double>::infinity() )
        , effectivePermeability( std::numeric_limits<double>::infinity() )
        , dFactor( std::numeric_limits<double>::infinity() )
        , eqWellRadius( std::numeric_limits<double>::infinity() )
        , betaFactor( std::numeric_limits<double>::infinity() )
        , isDataDirty( true )
    {
    }

    bool isDirty() const { return isDataDirty; }

    double eqWellRadius;
    double width;
    double conductivity;
    double effectivePermeability;
    double dFactor;
    double betaFactor;
    bool   isDataDirty;
};

//==================================================================================================
///
///
//==================================================================================================
class RimFracture : public RimCheckableNamedObject, public Rim3dPropertiesInterface, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimFracture( void );
    ~RimFracture( void ) override;

    double perforationLength() const;
    double perforationEfficiency() const;

    void setStimPlanTimeIndexToPlot( int timeIndex );

    double                        wellRadius() const;
    cvf::Vec3d                    anchorPosition() const;
    void                          setAnchorPosition( const cvf::Vec3d& pos );
    RiaDefines::EclipseUnitSystem fractureUnit() const;
    void                          setFractureUnit( RiaDefines::EclipseUnitSystem unitSystem );

    bool isEclipseCellOpenForFlow( const RigMainGrid*      mainGrid,
                                   const std::set<size_t>& reservoirCellIndicesOpenForFlow,
                                   size_t                  globalCellIndex ) const;

    cvf::Mat4d transformMatrix() const;
    double     dip() const;
    double     tilt() const;

    void                 setFractureTemplateNoUpdate( RimFractureTemplate* fractureTemplate );
    void                 setFractureTemplate( RimFractureTemplate* fractureTemplate );
    RimFractureTemplate* fractureTemplate() const;

    RivWellFracturePartMgr* fracturePartManager();

    std::vector<size_t> getPotentiallyFracturedCells( const RigMainGrid* mainGrid ) const;

    void       fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    cvf::Vec3d fracturePosition() const;

    virtual void   updateAzimuthBasedOnWellAzimuthAngle() = 0;
    virtual double wellAzimuthAtFracturePosition() const  = 0;
    virtual double fractureMD() const                     = 0;

    virtual void                    loadDataAndUpdate()                       = 0;
    virtual std::vector<cvf::Vec3d> perforationLengthCenterLineCoords() const = 0;

    // Fracture properties
    const NonDarcyData& nonDarcyProperties() const;
    void                ensureValidNonDarcyProperties();
    void                clearCachedNonDarcyProperties();

    virtual void triangleGeometry( std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices ) const;

    void                   updateFractureGrid();
    const RigFractureGrid* fractureGrid() const;

    friend class RimFractureTemplate;

    // RimWellPathCompletionsInterface overrides.
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

private:
    cvf::Vec3d fracturePositionForUi() const;
    double     wellFractureAzimuthDiff() const;
    void       triangleGeometryTransformed( std::vector<cvf::uint>*  triangleIndices,
                                            std::vector<cvf::Vec3f>* vxCoords,
                                            bool                     transform ) const;

    QString wellFractureAzimuthDiffText() const;
    QString wellAzimuthAtFracturePositionText() const;

    cvf::BoundingBox boundingBoxInDomainCoords() const override;

protected:
    caf::PdmPtrField<RimFractureTemplate*>                     m_fractureTemplate;
    caf::PdmField<bool>                                        m_editFractureTemplate;
    caf::PdmField<bool>                                        m_createEllipseFractureTemplate;
    caf::PdmField<bool>                                        m_createStimPlanFractureTemplate;
    caf::PdmField<double>                                      m_wellPathDepthAtFracture;
    caf::PdmProxyValueField<cvf::Vec3d>                        m_uiAnchorPosition;
    caf::PdmField<caf::AppEnum<RiaDefines::EclipseUnitSystem>> m_fractureUnit;

    caf::PdmProxyValueField<QString> m_uiWellPathAzimuth;
    caf::PdmProxyValueField<QString> m_uiWellFractureAzimuthDiff;
    caf::PdmField<QString>           m_wellFractureAzimuthAngleWarning;

    caf::PdmField<double> m_dip;
    caf::PdmField<double> m_tilt;
    caf::PdmField<double> m_azimuth;
    caf::PdmField<double> m_perforationLength;
    caf::PdmField<double> m_perforationEfficiency;
    caf::PdmField<double> m_wellDiameter;
    caf::PdmField<int>    m_stimPlanTimeIndexToPlot;

private:
    caf::PdmField<cvf::Vec3d> m_anchorPosition;

    cvf::ref<RivWellFracturePartMgr> m_fracturePartMgr;
    cvf::cref<RigFractureGrid>       m_fractureGrid;

    NonDarcyData m_cachedFractureProperties;
};

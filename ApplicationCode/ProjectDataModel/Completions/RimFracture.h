/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "Rim3dPropertiesInterface.h"
#include "RimCheckableNamedObject.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

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


//==================================================================================================
///  
///  
//==================================================================================================
class RimFracture : public RimCheckableNamedObject, public Rim3dPropertiesInterface
{
     CAF_PDM_HEADER_INIT;

public:
    RimFracture(void);
    virtual ~RimFracture(void);

    double                          perforationLength() const;
    double                          perforationEfficiency() const;
    
    void                            setStimPlanTimeIndexToPlot(int timeIndex);

    double                          wellRadius() const;
    cvf::Vec3d                      anchorPosition() const ;
    void                            setAnchorPosition(const cvf::Vec3d& pos);
    RiaEclipseUnitTools::UnitSystem fractureUnit() const;
    void                            setFractureUnit(RiaEclipseUnitTools::UnitSystem unitSystem);

    bool                            isEclipseCellWithinContainment(const RigMainGrid* mainGrid, 
                                                                   const std::set<size_t>& containmentCells,
                                                                   size_t globalCellIndex) const;

    size_t                          findAnchorEclipseCell(const RigMainGrid* mainGrid) const;

    cvf::Mat4d                      transformMatrix() const;
    double                          dip() const;
    double                          tilt() const;

    void                            setFractureTemplateNoUpdate(RimFractureTemplate* fractureTemplate);
    void                            setFractureTemplate(RimFractureTemplate* fractureTemplate);
    RimFractureTemplate*            fractureTemplate() const;

    RivWellFracturePartMgr*         fracturePartManager();

    void                            triangleGeometry(std::vector<cvf::uint>* triangleIndices, std::vector<cvf::Vec3f>* vxCoords );

    std::vector<size_t>             getPotentiallyFracturedCells(const RigMainGrid* mainGrid);

    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    cvf::Vec3d                      fracturePosition() const;

    virtual void                    updateAzimuthBasedOnWellAzimuthAngle() = 0;
    virtual double                  wellAzimuthAtFracturePosition() const = 0;
    virtual double                  fractureMD() const = 0;
    
    virtual void                    loadDataAndUpdate() = 0;
    virtual std::vector<cvf::Vec3d> perforationLengthCenterLineCoords() const = 0;
    
    friend class RimFractureTemplate;

protected:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;

private:
    cvf::Vec3d                      fracturePositionForUi() const;
    double                          wellFractureAzimuthDiff() const; 
    
    QString                         wellFractureAzimuthDiffText() const;
    QString                         wellAzimuthAtFracturePositionText() const;

    virtual cvf::BoundingBox        boundingBoxInDomainCoords() override;

protected:
    caf::PdmPtrField<RimFractureTemplate*>          m_fractureTemplate;
    caf::PdmProxyValueField<cvf::Vec3d>             m_uiAnchorPosition;
    caf::PdmField< RiaEclipseUnitTools::UnitSystemType >  m_fractureUnit;

    caf::PdmProxyValueField<QString> m_uiWellPathAzimuth;
    caf::PdmProxyValueField<QString> m_uiWellFractureAzimuthDiff;
    caf::PdmField<QString>           m_wellFractureAzimuthAngleWarning;

    caf::PdmField<double>            m_dip;
    caf::PdmField<double>            m_tilt;
    caf::PdmField<double>            m_azimuth;
    caf::PdmField<double>            m_perforationLength;
    caf::PdmField<double>            m_perforationEfficiency;
    caf::PdmField<double>            m_wellDiameter;
    caf::PdmField<int>               m_stimPlanTimeIndexToPlot;

private:
    caf::PdmField<cvf::Vec3d>        m_anchorPosition;

    cvf::ref<RivWellFracturePartMgr> m_fracturePartMgr;
};

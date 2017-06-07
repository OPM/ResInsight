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

#include "RimCheckableNamedObject.h"
#include "RimDefines.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"


class RigFracture;
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
class RimFracture : public RimCheckableNamedObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimFracture(void);
    virtual ~RimFracture(void);

    caf::PdmField<double>           azimuth;
    caf::PdmField<double>           perforationLength;
    caf::PdmField<double>           perforationEfficiency;
    caf::PdmField<double>           wellRadius;

    caf::PdmField<double>           dip;
    caf::PdmField<double>           tilt;

    caf::PdmField<int>              stimPlanTimeIndexToPlot;
    caf::PdmField<bool>             showPolygonFractureOutline;

    caf::PdmField< caf::AppEnum< RimDefines::UnitSystem > >  fractureUnit;


    cvf::Vec3d                      anchorPosition() const ;
    void                            setAnchorPosition(const cvf::Vec3d& pos);

    cvf::Mat4f                      transformMatrix() const; 

    const RigFracture*              attachedRigFracture() const;

    void                            setFractureTemplate(RimFractureTemplate* fractureTemplate);
    RimFractureTemplate*            attachedFractureDefinition() const;

    RivWellFracturePartMgr*         fracturePartManager();

    bool                            hasValidGeometry() const;
    void                            computeGeometry();
    void                            setRecomputeGeometryFlag();
    
    const std::vector<cvf::uint>&   triangleIndices() const;
    const std::vector<cvf::Vec3f>&  nodeCoords() const;

    std::vector<size_t>             getPotentiallyFracturedCells(const RigMainGrid* mainGrid);

    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    cvf::Vec3d                      fracturePosition() const;

    virtual void                    updateAzimuthFromFractureDefinition() = 0;
    virtual double                  wellAzimuthAtFracturePosition() = 0;


protected:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;

private:
    void                            updateFieldVisibility();
    bool                            isRecomputeGeometryFlagSet();
    cvf::Vec3d                      fracturePositionForUi() const;
    
    QString                         createOneBasedIJK() const;


protected:
    caf::PdmPtrField<RimFractureTemplate*>          m_fractureTemplate;
    caf::PdmProxyValueField<cvf::Vec3d>             m_uiAnchorPosition;
    caf::PdmProxyValueField<QString>                m_displayIJK;

private:
    caf::PdmField<cvf::Vec3d>                       m_anchorPosition;
    cvf::ref<RigFracture>                           m_rigFracture;
    bool                                            m_recomputeGeometry;

    caf::PdmField<int>                              m_i;    // Zero based indexing
    caf::PdmField<int>                              m_j;    // Zero based indexing
    caf::PdmField<int>                              m_k;    // Zero based indexing

    cvf::ref<RivWellFracturePartMgr>                m_rivFracture;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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
#include "RimPolylinesDataInterface.h"

#include "cafPdmFieldCvfVec3d.h"

#include "cafPdmPtrField.h"

class RimPolygon;
class RivPolylinePartMgr;

namespace cvf
{
class ModelBasicList;
class BoundingBox;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
} // namespace caf

class RimPolygonInView : public RimCheckableNamedObject, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonInView();
    ~RimPolygonInView() override;

    RimPolygon* polygon() const;
    void        setPolygon( RimPolygon* polygon );

    void appendPartsToModel( cvf::ModelBasicList* model, caf::DisplayCoordTransform* scaleTransform, const cvf::BoundingBox& boundingBox );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    void                       updateNameField();
    cvf::ref<RigPolyLinesData> polyLinesData() const override;

private:
    caf::PdmPtrField<RimPolygon*> m_polygon;
    cvf::ref<RivPolylinePartMgr>  m_polylinePartMgr;
};

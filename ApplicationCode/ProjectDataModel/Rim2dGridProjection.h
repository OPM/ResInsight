/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "RimRegularLegendConfig.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfGeometryBuilderFaceList.h"
#include "cvfVector2.h"

class RigMainGrid;
class RimEclipseResultCase;

//==================================================================================================
///  
///  
//==================================================================================================
class Rim2dGridProjection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;
public:
    enum ResultAggregationEnum
    {
        RESULTS_MEAN_VALUE = 0,
        RESULTS_MIN_VALUE,
        RESULTS_MAX_VALUE
    };
    typedef caf::AppEnum<ResultAggregationEnum> ResultAggregation;

    Rim2dGridProjection();
    virtual ~Rim2dGridProjection();

    void                        extractGridData();
    void                        generateVertices(cvf::Vec3fArray* vertices, const caf::DisplayCoordTransform* displayCoordTransform);
    double                      maxValue() const;
    double                      minValue() const;
    double                      sampleSpacing() const;
    void                        updateDefaultSampleSpacingFromGrid();

    double                      value(uint i, uint j) const;
    bool                        hasResultAt(uint i, uint j) const;

    cvf::Vec2ui                 surfaceGridSize() const;
    uint                        vertexCount() const;
    RimRegularLegendConfig*     legendConfig() const;

    size_t                      gridIndex(uint i, uint j) const;
    cvf::Vec2ui                 ijFromGridIndex(size_t gridIndex) const;
protected:
    cvf::Vec2d                  globalPos2d(uint i, uint j) const;
    const std::vector<size_t>&  cellsAtPos2d(uint i, uint j) const;    

    const RimEclipseResultCase* eclipseCase() const;
    RigMainGrid*                mainGrid() const;
    
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

protected:
    caf::PdmField<double>                        m_sampleSpacing;
    caf::PdmField<ResultAggregation>             m_resultAggregation;
    caf::PdmChildField<RimRegularLegendConfig*>  m_legendConfig;

    std::vector<std::vector<size_t>>             m_projected3dGridIndices;


};

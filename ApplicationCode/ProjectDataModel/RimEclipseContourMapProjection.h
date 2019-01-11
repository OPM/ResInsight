/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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
#include "RimContourMapProjection.h"
#include "RimRegularLegendConfig.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfString.h"
#include "cvfVector2.h"

class RigMainGrid;
class RigResultAccessor;
class RimEclipseContourMapView;
class RimEclipseResultCase;
class RimEclipseResultDefinition;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseContourMapProjection : public RimContourMapProjection
{
    CAF_PDM_HEADER_INIT;
public:

    RimEclipseContourMapProjection();
    ~RimEclipseContourMapProjection() override;

    QString                      resultDescriptionText() const override;
    QString                      weightingParameter() const;

    void                         updatedWeightingResult();

    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void initAfterRead() override;

private:
    typedef RimContourMapProjection::CellIndexAndResult CellIndexAndResult;

    void                            generateGridMapping() override;
    void                            generateResults(int timeStep) override;

    bool                            gridMappingNeedsUpdating() const override;
    bool                            resultsNeedUpdating(int timeStep) const override;

    void                            clearResults() override;

    double                          calculateValueInCell(uint i, uint j) const;

    std::vector<CellIndexAndResult> visibleCellsAndOverlapVolumeFrom2dPoint(const cvf::Vec2d& globalPos2d, const std::vector<double>* weightingResultValues = nullptr) const;
    std::vector<CellIndexAndResult> visibleCellsAndLengthInCellFrom2dPoint(const cvf::Vec2d& globalPos2d, const std::vector<double>* weightingResultValues = nullptr) const;
    double                          findColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const;

    void                            updateGridInformation() override;

    RimEclipseResultCase*           eclipseCase() const;
    RimGridView*                    baseView() const override;
    RimEclipseContourMapView*       view() const;

protected:
    caf::PdmField<bool>                                 m_weightByParameter;
    caf::PdmChildField<RimEclipseResultDefinition*>     m_weightingResult;

    cvf::ref<cvf::UByteArray>                           m_cellGridIdxVisibility;

    cvf::ref<RigResultAccessor>                         m_resultAccessor;

    caf::PdmPointer<RimEclipseResultCase>               m_eclipseCase;
    cvf::ref<RigMainGrid>                               m_mainGrid;
    QString                                             m_currentResultName;
};

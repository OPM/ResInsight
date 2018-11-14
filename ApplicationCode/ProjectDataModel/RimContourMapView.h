/////////////////////////////////////////////////////////////////////////////////
//
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

#include "RimEclipseView.h"
#include "RimNameConfig.h"

class RimContourMapNameConfig;
class RivContourMapProjectionPartMgr;

class RimContourMapView : public RimEclipseView, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;
public:
    RimContourMapView();
    RimContourMapProjection*                     contourMapProjection() const;

    QString createAutoName() const override;
    bool    isTimeStepDependentDataVisible() const override;

protected:
    void initAfterRead() override;
    void createDisplayModel() override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void updateCurrentTimeStep() override;

    void appendContourMapProjectionToModel();

    void updateLegends() override;
    void updateViewWidgetAfterCreation() override;  
    void updateViewFollowingRangeFilterUpdates() override;
    void onLoadDataAndUpdate() override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;    

    caf::PdmFieldHandle* userDescriptionField() override;

    virtual std::set<RivCellSetEnum> allVisibleFaultGeometryTypes() const override;

private:
    cvf::ref<RivContourMapProjectionPartMgr>     m_contourMapProjectionPartMgr;
    caf::PdmChildField<RimContourMapProjection*> m_contourMapProjection;
    caf::PdmField<bool>                          m_showAxisLines;
    caf::PdmChildField<RimContourMapNameConfig*> m_nameConfig;

};


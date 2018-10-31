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

class Riv2dGridProjectionPartMgr;

class Rim2dEclipseView : public RimEclipseView
{
    CAF_PDM_HEADER_INIT;
public:
    Rim2dEclipseView();
    Rim2dGridProjection*                     grid2dProjection() const;

protected:
    void initAfterRead() override;
    void createDisplayModel() override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void updateCurrentTimeStep() override;
    void updateLegends() override;
    void updateViewWidgetAfterCreation() override;  
    void updateViewFollowingRangeFilterUpdates() override;
    void onLoadDataAndUpdate() override;

private:
    cvf::ref<Riv2dGridProjectionPartMgr>     m_grid2dProjectionPartMgr;
    caf::PdmChildField<Rim2dGridProjection*> m_2dGridProjection;

};


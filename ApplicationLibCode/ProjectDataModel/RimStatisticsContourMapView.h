/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimEclipseContourMapView.h"

class RimStatisticsContourMap;

class RimStatisticsContourMapView : public RimEclipseContourMapView
{
    CAF_PDM_HEADER_INIT;

public:
    RimStatisticsContourMapView();

    void                     setStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap );
    RimStatisticsContourMap* statisticsContourMap() const;

    QString createAutoName() const override;
    void    setDefaultCustomName();

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void    onClampCurrentTimestep() override;
    size_t  onTimeStepCountRequested() override;
    QString timeStepName( int frameIdx ) const override;

    // void createContourMapGeometry();
    void onUpdateLegends() override;

private:
    caf::PdmPtrField<RimStatisticsContourMap*> m_statisticsContourMap;
};

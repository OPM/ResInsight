/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimHistogramPlot.h"

#include "cafPdmObject.h"
#include "cafSignal.h"

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramDataSource : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimHistogramDataSource();
    ~RimHistogramDataSource() override;

    caf::Signal<> dataSourceChanged;

    virtual std::string unitNameX() const = 0;
    virtual std::string unitNameY() const = 0;

    virtual std::vector<double> valuesX( RimHistogramPlot::GraphType graphType ) const                                                = 0;
    virtual std::vector<double> valuesY( RimHistogramPlot::GraphType graphType, RimHistogramPlot::FrequencyType frequencyType ) const = 0;

    virtual std::string name() const = 0;

    static std::vector<double> computeHistogramBins( double min, double max, int numBins, RimHistogramPlot::GraphType graphType );
    static std::vector<double> computeHistogramFrequencies( const std::vector<size_t>&      values,
                                                            RimHistogramPlot::GraphType     graphType,
                                                            RimHistogramPlot::FrequencyType frequencyType );
    static std::vector<double> computeHistogramFrequencies( const std::vector<double>&      values,
                                                            RimHistogramPlot::GraphType     graphType,
                                                            RimHistogramPlot::FrequencyType frequencyType );
};

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

#include "RigStatisticsMath.h"

#include "RimHistogramPlot.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafSignal.h"

#include <QString>

#include <limits>
#include <utility>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramDataSource : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class BinRangeMode
    {
        AUTOMATIC,
        USER_DEFINED
    };

    struct HistogramResult
    {
        HistogramResult()
            : p10( std::numeric_limits<double>::infinity() )
            , p90( std::numeric_limits<double>::infinity() )
            , mean( std::numeric_limits<double>::infinity() )
        {
        }

        std::vector<double> valuesX;
        std::vector<double> valuesY;
        double              p10;
        double              p90;
        double              mean;
    };

    RimHistogramDataSource();
    ~RimHistogramDataSource() override;

    caf::Signal<>                                    dataSourceChanged;
    caf::Signal<>                                    cumulativeChanged;
    caf::Signal<RigHistogramCalculator::BinningMode> binningModeChanged;

    virtual std::string unitNameX() const = 0;
    virtual std::string unitNameY() const = 0;

    virtual HistogramResult
        compute( RimHistogramPlot::GraphType graphType, RimHistogramPlot::FrequencyType frequencyType, bool cumulative = false ) const = 0;

    virtual bool showCumulativeCurve() const;
    virtual void setShowCumulativeCurve( bool showCumulativeCurve );

    void setBinningMode( RigHistogramCalculator::BinningMode binningMode );

    virtual std::vector<QString> filterDescriptions() const;
    static QString               userDefinedRangeFilterText( double min, double max );

    virtual void setDefaults() = 0;

    virtual std::string name() const = 0;

    static std::vector<double>
                               computeHistogramBins( double                              min,
                                                     double                              max,
                                                     int                                 numBins,
                                                     RimHistogramPlot::GraphType         graphType,
                                                     bool                                cumulative = false,
                                                     RigHistogramCalculator::BinningMode binningMode = RigHistogramCalculator::BinningMode::LINEAR );
    static std::vector<double> computeHistogramFrequencies( const std::vector<size_t>&      values,
                                                            RimHistogramPlot::GraphType     graphType,
                                                            RimHistogramPlot::FrequencyType frequencyType,
                                                            bool                            cumulative = false );
    static std::vector<double> computeHistogramFrequencies( const std::vector<double>&      values,
                                                            RimHistogramPlot::GraphType     graphType,
                                                            RimHistogramPlot::FrequencyType frequencyType,
                                                            bool                            cumulative = false );

    // Resolve the effective bin range: the user-defined range when USER_DEFINED, otherwise [dataMin, dataMax].
    // For logarithmic binning a non-positive minimum is replaced by smallestPositiveValue.
    static std::pair<double, double> computeBinRange( BinRangeMode                        binRangeMode,
                                                      double                              userMin,
                                                      double                              userMax,
                                                      double                              dataMin,
                                                      double                              dataMax,
                                                      RigHistogramCalculator::BinningMode binningMode,
                                                      double                              smallestPositiveValue );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void appendBinningUiOrdering( caf::PdmUiOrdering& uiOrdering, caf::PdmFieldHandle* numBinsField );

    std::pair<double, double> binRange( double dataMin, double dataMax, double smallestPositiveValue ) const;

    RigHistogramCalculator::BinningMode        binningMode() const;
    RigHistogramCalculator::OutOfRangeHandling outOfRangeHandling() const;
    bool                                       useUserDefinedBinRange() const;

    caf::PdmField<caf::AppEnum<RigHistogramCalculator::BinningMode>>        m_binningMode;
    caf::PdmField<caf::AppEnum<BinRangeMode>>                               m_binRangeMode;
    caf::PdmField<double>                                                   m_binRangeMin;
    caf::PdmField<double>                                                   m_binRangeMax;
    caf::PdmField<caf::AppEnum<RigHistogramCalculator::OutOfRangeHandling>> m_outOfRangeHandling;
};

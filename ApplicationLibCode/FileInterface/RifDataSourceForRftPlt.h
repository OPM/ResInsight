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

#include "RimViewWindow.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"

#include <QDate>
#include <QMetaType>
#include <QPointer>

class RimWellLogLasFile;
class RimEclipseCase;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimObservedFmuRftData;
class RimPressureDepthData;

//==================================================================================================
///
///
//==================================================================================================
class RifDataSourceForRftPlt
{
public:
    enum class SourceType
    {
        NONE,
        RFT_SIM_WELL_DATA,
        GRID_MODEL_CELL_DATA,
        SUMMARY_RFT,
        ENSEMBLE_RFT,
        OBSERVED_LAS_FILE,
        OBSERVED_FMU_RFT,
        OBSERVED_PRESSURE_DEPTH
    };

    RifDataSourceForRftPlt();
    RifDataSourceForRftPlt( SourceType sourceType, RimEclipseCase* eclCase );
    RifDataSourceForRftPlt( RimSummaryEnsemble* ensemble );
    RifDataSourceForRftPlt( RimSummaryCase* summaryCase, RimSummaryEnsemble* ensemble, RimEclipseCase* eclipseCase );
    RifDataSourceForRftPlt( RimWellLogLasFile* wellLogFile );
    RifDataSourceForRftPlt( RimObservedFmuRftData* observedFmuRftData );
    RifDataSourceForRftPlt( RimPressureDepthData* pressureDepthData );

    SourceType sourceType() const;

    RimEclipseCase*        eclCase() const;
    RimSummaryCase*        summaryCase() const;
    RimSummaryEnsemble*    ensemble() const;
    RimWellLogLasFile*     wellLogFile() const;
    RimObservedFmuRftData* observedFmuRftData() const;
    RimPressureDepthData*  pressureDepthData() const;

    static QString sourceTypeUiText( SourceType sourceType );

    std::vector<RiaDefines::EclipseUnitSystem> availableUnitSystems() const;

    auto operator<=>( const RifDataSourceForRftPlt& rhs ) const -> std::strong_ordering;

    // When operator<=>() is overloaded, no operator==() nor operator!=() are defined by default by the compiler
    // https://ggulgulia.medium.com/c-20-three-way-comparison-operator-part-2-fd520fb75e00
    bool operator==( const RifDataSourceForRftPlt& rhs ) const = default;

private:
    SourceType m_sourceType;

    caf::PdmPointer<RimEclipseCase>        m_eclCase;
    caf::PdmPointer<RimSummaryCase>        m_summaryCase;
    caf::PdmPointer<RimSummaryEnsemble>    m_ensemble;
    caf::PdmPointer<RimWellLogLasFile>     m_wellLogFile;
    caf::PdmPointer<RimObservedFmuRftData> m_observedFmuRftData;
    caf::PdmPointer<RimPressureDepthData>  m_pressureDepthData;
};

QTextStream& operator<<( QTextStream& str, const RifDataSourceForRftPlt& addr );
QTextStream& operator>>( QTextStream& str, RifDataSourceForRftPlt& addr );

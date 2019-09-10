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

class RimWellLogFile;
class RimEclipseCase;
class RifReaderRftInterface;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimObservedFmuRftData;

//==================================================================================================
///
///
//==================================================================================================
class RifDataSourceForRftPlt
{
public:
    enum SourceType
    {
        NONE,
        OBSERVED,
        RFT,
        GRID,
        ENSEMBLE_RFT,
        SUMMARY_RFT,
        OBSERVED_FMU_RFT
    };

    RifDataSourceForRftPlt();
    RifDataSourceForRftPlt( SourceType sourceType, RimEclipseCase* eclCase );
    RifDataSourceForRftPlt( SourceType sourceType, RimSummaryCaseCollection* ensemble );
    RifDataSourceForRftPlt( SourceType sourceType, RimSummaryCase* summaryCase, RimSummaryCaseCollection* ensemble );
    RifDataSourceForRftPlt( SourceType sourceType, RimWellLogFile* wellLogFile = nullptr );
    RifDataSourceForRftPlt( SourceType sourceType, RimObservedFmuRftData* observedFmuRftData );

    SourceType                sourceType() const;
    RimEclipseCase*           eclCase() const;
    RifReaderRftInterface*    rftReader() const;
    RimSummaryCaseCollection* ensemble() const;
    RimSummaryCase*           summaryCase() const;
    RimWellLogFile*           wellLogFile() const;
    RimObservedFmuRftData*    observedFmuRftData() const;

    static QString sourceTypeUiText( SourceType sourceType );

    friend QTextStream& operator>>( QTextStream& str, RifDataSourceForRftPlt& addr );
    friend bool         operator<( const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2 );

private:
    SourceType                                m_sourceType;
    caf::PdmPointer<RimEclipseCase>           m_eclCase;
    caf::PdmPointer<RimSummaryCase>           m_summaryCase;
    caf::PdmPointer<RimSummaryCaseCollection> m_ensemble;
    caf::PdmPointer<RimWellLogFile>           m_wellLogFile;
    caf::PdmPointer<RimObservedFmuRftData>    m_observedFmuRftData;
};

bool         operator==( const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2 );
QTextStream& operator<<( QTextStream& str, const RifDataSourceForRftPlt& addr );
QTextStream& operator>>( QTextStream& str, RifDataSourceForRftPlt& addr );
bool         operator<( const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2 );
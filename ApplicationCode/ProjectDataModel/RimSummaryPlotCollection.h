/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QPointer>
#include <QDockWidget>

class RimSummaryPlot;
class RicDropEnabledMainWindow;
class RifReaderEclipseSummary;
class RimEclipseResultCase;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryPlotCollection();
    virtual ~RimSummaryPlotCollection();

    RifReaderEclipseSummary* getOrCreateSummaryFileReader(const RimEclipseResultCase* eclipseCase);

    caf::PdmChildArrayField<RimSummaryPlot*> m_summaryPlots;

private:
    RifReaderEclipseSummary* createSummaryFileReader(const QString& eclipseCaseFilePathBasename);
    RifReaderEclipseSummary* getOrCreateSummaryFileReader(const QString& eclipseCaseFilePathBasename);

private:
    // Map from path to case to summary file reader objects
    std::map<QString, RifReaderEclipseSummary*> m_summaryFileReaders;
};

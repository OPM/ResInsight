/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimObservedData.h"

#include "../../Commands/SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildField.h"
#include "cvfObject.h"

class RifSummaryReaderInterface;

//==================================================================================================
//
//==================================================================================================
class RimCsvUserData : public RimObservedData
{
    CAF_PDM_HEADER_INIT;
public:
    RimCsvUserData();
    ~RimCsvUserData() override;

    void createSummaryReaderInterface() override;

    RifSummaryReaderInterface* summaryReader() override;

    QString errorMessagesFromReader() override;

    RicPasteAsciiDataToSummaryPlotFeatureUi* parseOptions() const;

private:
    caf::PdmChildField<RicPasteAsciiDataToSummaryPlotFeatureUi*>  m_parseOptions;
    cvf::ref<RifSummaryReaderInterface> m_summaryReader;
    QString                             m_errorText;
};

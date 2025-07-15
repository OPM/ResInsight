/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimSummaryCase.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfObject.h"

#include <QDateTime>

//==================================================================================================
//
//==================================================================================================
class RimCsvSummaryCase : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;

public:
    enum class FileType
    {
        REVEAL,
        STIMPLAN
    };

    RimCsvSummaryCase();

    QString caseName() const override;

    void setFileType( FileType fileType );

    void                       createSummaryReaderInterface() override;
    RifSummaryReaderInterface* summaryReader() override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    std::unique_ptr<RifSummaryReaderInterface> m_summaryReader;
    caf::PdmField<caf::AppEnum<FileType>>      m_fileType;
    caf::PdmField<QDateTime>                   m_startDate;
};

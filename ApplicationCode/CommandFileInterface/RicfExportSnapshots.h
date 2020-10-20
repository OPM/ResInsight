/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfCommandObject.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

//==================================================================================================
//
//
//
//==================================================================================================
class RicfExportSnapshots : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    // Values are exposed in gRPC .proto. Do not change without also changing .proto

    enum class PlotOutputFormat
    {
        PNG,
        PDF
    };
    typedef caf::AppEnum<PlotOutputFormat> PreferredOutputFormatEnum;

    enum class SnapshotsType
    {
        VIEWS,
        PLOTS,
        ALL
    };
    typedef caf::AppEnum<SnapshotsType> SnapshotsTypeEnum;

public:
    RicfExportSnapshots();

    caf::PdmScriptResponse execute() override;

private:
    caf::PdmField<SnapshotsTypeEnum>         m_type;
    caf::PdmField<QString>                   m_prefix;
    caf::PdmField<int>                       m_caseId;
    caf::PdmField<int>                       m_viewId;
    caf::PdmField<QString>                   m_exportFolder;
    caf::PdmField<PreferredOutputFormatEnum> m_plotOutputFormat;
};

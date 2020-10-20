/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
class RicfExportWellLogPlotDataResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfExportWellLogPlotDataResult();

public:
    caf::PdmField<std::vector<QString>> exportedFiles;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RicfExportWellLogPlotData : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    // Values are exposed in gRPC .proto. Do not change without also changing .proto
    enum class ExportFormat
    {
        LAS,
        ASCII
    };
    typedef caf::AppEnum<ExportFormat> ExportFormatEnum;

public:
    RicfExportWellLogPlotData();

    caf::PdmScriptResponse execute() override;

private:
    caf::PdmField<ExportFormatEnum> m_format;
    caf::PdmField<int>              m_viewId;
    caf::PdmField<QString>          m_folder;
    caf::PdmField<QString>          m_filePrefix;
    caf::PdmField<bool>             m_exportTvdRkb;
    caf::PdmField<bool>             m_capitalizeFileNames;
    caf::PdmField<double>           m_resampleInterval;
    caf::PdmField<bool>             m_convertCurveUnits;
};

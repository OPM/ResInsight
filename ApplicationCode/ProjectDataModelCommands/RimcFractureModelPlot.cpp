/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "RimcFractureModelPlot.h"

#include "RifFractureModelPlotExporter.h"

#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFractureModelPlot, RimcFractureModelPlot_exportToFile, "ExportToFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFractureModelPlot_exportToFile::RimcFractureModelPlot_exportToFile( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Fracture Model Plot", "", "", "Export Fracture Model Plot to File" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_directoryPath, "DirectoryPath", "", "", "", "Directory Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcFractureModelPlot_exportToFile::execute()
{
    RimFractureModelPlot* fractureModelPlot = self<RimFractureModelPlot>();

    RifFractureModelPlotExporter::writeToDirectory( fractureModelPlot,
                                                    fractureModelPlot->fractureModel()->useDetailedFluidLoss(),
                                                    m_directoryPath() );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFractureModelPlot_exportToFile::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFractureModelPlot_exportToFile::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFractureModelPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFractureModelPlot_exportToFile::isNullptrValidResult() const
{
    return true;
}

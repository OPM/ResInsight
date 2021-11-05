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

#include "RimDialogData.h"

#include "RimMockModelSettings.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"
#include "ExportCommands/RicExportCarfinUi.h"
#include "ExportCommands/RicExportEclipseSectorModelUi.h"
#include "ExportCommands/RicExportLgrUi.h"
#include "ExportCommands/RicExportWellPathsUi.h"
#include "FractureCommands/RicCreateMultipleFracturesUi.h"
#include "HoloLensCommands/RicHoloLensExportToFolderUi.h"
#include "RicCreateEnsembleSurfaceUi.h"
#include "RicCreateEnsembleWellLogUi.h"

CAF_PDM_SOURCE_INIT( RimDialogData, "RimDialogData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDialogData::RimDialogData()
{
    CAF_PDM_InitObject( "Dialog Data" );

    CAF_PDM_InitFieldNoDefault( &m_exportCarfin, "ExportCarfin", "Export Carfin" );
    m_exportCarfin = new RicExportCarfinUi;

    CAF_PDM_InitFieldNoDefault( &m_exportCompletionData, "ExportCompletionData", "Export Completion Data" );
    m_exportCompletionData = new RicExportCompletionDataSettingsUi();

    CAF_PDM_InitFieldNoDefault( &m_multipleFractionsData, "MultipleFractionsData", "Multiple Fractures Data" );
    m_multipleFractionsData = new RiuCreateMultipleFractionsUi();

    CAF_PDM_InitFieldNoDefault( &m_holoLenseExportToFolderData,
                                "HoloLenseExportToFolderData",
                                "Holo Lens Export To Folder Data" );
    m_holoLenseExportToFolderData = new RicHoloLensExportToFolderUi();

    CAF_PDM_InitFieldNoDefault( &m_exportWellPathsData, "ExportwellPathsData", "Export Well Paths Data" );
    m_exportWellPathsData = new RicExportWellPathsUi();

    CAF_PDM_InitFieldNoDefault( &m_exportLgrData, "ExportLgr", "LGR Export" );
    m_exportLgrData = new RicExportLgrUi();

    CAF_PDM_InitFieldNoDefault( &m_exportSectorModelData, "ExportSectorModel", "Export Sector Model" );
    m_exportSectorModelData = new RicExportEclipseSectorModelUi();

    CAF_PDM_InitFieldNoDefault( &m_mockModelSettings, "MockModelSettings", "Mock Model Settings" );
    m_mockModelSettings = new RimMockModelSettings();

    CAF_PDM_InitFieldNoDefault( &m_createEnsembleSurfaceUi, "CreateEnsembleSurfaceUi", "Create Ensmeble Surface Ui" );
    m_createEnsembleSurfaceUi = new RicCreateEnsembleSurfaceUi();

    CAF_PDM_InitFieldNoDefault( &m_createEnsembleWellLogUi, "CreateEnsembleWellLogUi", "Create Ensemble Well Log Ui" );
    m_createEnsembleWellLogUi = new RicCreateEnsembleWellLogUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDialogData::clearProjectSpecificData()
{
    m_multipleFractionsData->resetValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportCarfinUi* RimDialogData::exportCarfin() const
{
    return m_exportCarfin;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDialogData::exportCarfinDataAsString() const
{
    return m_exportCarfin->writeObjectToXmlString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDialogData::setExportCarfinDataFromString( const QString& data )
{
    m_exportCarfin->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
    m_exportCarfin->resolveReferencesRecursively();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi* RimDialogData::exportCompletionData() const
{
    return m_exportCompletionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCreateMultipleFractionsUi* RimDialogData::multipleFractionsData() const
{
    return m_multipleFractionsData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensExportToFolderUi* RimDialogData::holoLensExportToFolderData() const
{
    return m_holoLenseExportToFolderData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportWellPathsUi* RimDialogData::wellPathsExportData() const
{
    return m_exportWellPathsData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportLgrUi* RimDialogData::exportLgrData() const
{
    return m_exportLgrData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportEclipseSectorModelUi* RimDialogData::exportSectorModelUi() const
{
    return m_exportSectorModelData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMockModelSettings* RimDialogData::mockModelSettings() const
{
    return m_mockModelSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateEnsembleSurfaceUi* RimDialogData::createEnsembleSurfaceUi() const
{
    return m_createEnsembleSurfaceUi;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateEnsembleWellLogUi* RimDialogData::createEnsembleWellLogUi() const
{
    return m_createEnsembleWellLogUi;
}

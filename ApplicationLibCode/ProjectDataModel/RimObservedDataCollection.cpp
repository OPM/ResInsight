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

#include "RimObservedDataCollection.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "RifKeywordVectorParser.h"

#include "RimCsvUserData.h"
#include "RimMainPlotCollection.h"
#include "RimObservedEclipseUserData.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimPressureDepthData.h"
#include "RimProject.h"
#include "RimSummaryObservedDataFile.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmSettings.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafUtils.h"

#include <QFile>

CAF_PDM_SOURCE_INIT( RimObservedDataCollection, "ObservedDataCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedDataCollection::RimObservedDataCollection()
{
    CAF_PDM_InitObject( "Observed Data", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_observedDataArray, "ObservedDataArray", "" );
    CAF_PDM_InitFieldNoDefault( &m_observedFmuRftArray, "ObservedFmuRftDataArray", "" );
    CAF_PDM_InitFieldNoDefault( &m_observedPressureDepthArray, "PressureDepthDataArray", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::removeObservedSummaryData( RimObservedSummaryData* observedData )
{
    m_observedDataArray.removeChild( observedData );
    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::removeObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData )
{
    m_observedFmuRftArray.removeChild( observedFmuRftData );
    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedSummaryData*> RimObservedDataCollection::allObservedSummaryData() const
{
    return m_observedDataArray.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedFmuRftData*> RimObservedDataCollection::allObservedFmuRftData() const
{
    return m_observedFmuRftArray.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPressureDepthData*> RimObservedDataCollection::allPressureDepthData() const
{
    return m_observedPressureDepthArray.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimMainPlotCollection::current()->loadDataAndUpdateAllPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimObservedDataCollection::fileExists( const QString& fileName, QString* errorText /*= nullptr*/ )
{
    QFile file( fileName );
    if ( !file.exists() )
    {
        QString s = QString( "File does not exist, %1" ).arg( fileName );
        RiaLogging::error( s );

        if ( errorText ) errorText->append( s );
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void updateNewObservedDataCreated( caf::PdmObject* object )
{
    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::onObjectAppended( object );

    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();

    RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
    if ( mpw ) mpw->updateMultiPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedSummaryData* RimObservedDataCollection::createAndAddRsmObservedSummaryDataFromFile( const QString& fileName,
                                                                                               QString*       errorText /*= nullptr*/ )
{
    if ( !fileExists( fileName, errorText ) ) return nullptr;

    RimObservedSummaryData*     observedData        = nullptr;
    RimObservedEclipseUserData* columnBasedUserData = new RimObservedEclipseUserData();
    observedData                                    = columnBasedUserData;

    m_observedDataArray.push_back( observedData );
    observedData->setSummaryHeaderFileName( fileName );
    observedData->createSummaryReaderInterface();
    observedData->updateMetaData();

    if ( errorText && !observedData->errorMessagesFromReader().isEmpty() )
    {
        errorText->append( observedData->errorMessagesFromReader() );
    }

    RimProject::current()->assignCaseIdToSummaryCase( observedData );
    updateNewObservedDataCreated( observedData );

    updateConnectedEditors();

    return observedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedSummaryData* RimObservedDataCollection::createAndAddCvsObservedSummaryDataFromFile( const QString& fileName,
                                                                                               bool           useSavedFieldsValuesInDialog,
                                                                                               QString*       errorText /*= nullptr*/ )
{
    if ( !fileExists( fileName, errorText ) ) return nullptr;

    RimObservedSummaryData* observedData = nullptr;

    RimCsvUserData*                          userData     = new RimCsvUserData();
    RicPasteAsciiDataToSummaryPlotFeatureUi* parseOptions = userData->parseOptions();

    if ( useSavedFieldsValuesInDialog )
    {
        caf::PdmSettings::readFieldsFromApplicationStore( parseOptions, parseOptions->contextString() );
    }
    parseOptions->setUiModeImport( fileName );

    bool parseOptionsChanged = false;
    if ( parseOptions->uiModeImport() != RicPasteAsciiDataToSummaryPlotFeatureUi::UI_MODE_SILENT )
    {
        caf::PdmUiPropertyViewDialog propertyDialog( nullptr, parseOptions, "CSV Import Options", "" );
        if ( propertyDialog.exec() != QDialog::Accepted )
        {
            return nullptr;
        }

        parseOptionsChanged = true;
    }

    if ( useSavedFieldsValuesInDialog && parseOptionsChanged )
    {
        caf::PdmSettings::writeFieldsToApplicationStore( parseOptions, parseOptions->contextString() );
    }

    // userData->setParseOptions(parseOptionsUi.parseOptions());
    userData->setSummaryHeaderFileName( fileName );
    userData->createSummaryReaderInterface();
    userData->updateMetaData();

    if ( errorText && !userData->errorMessagesFromReader().isEmpty() )
    {
        errorText->append( userData->errorMessagesFromReader() );
    }

    if ( userData->summaryReader() )
    {
        m_observedDataArray.push_back( userData );
        observedData = userData;
    }
    else
    {
        delete userData;
        return nullptr;
    }

    RimProject::current()->assignCaseIdToSummaryCase( observedData );
    updateNewObservedDataCreated( observedData );

    updateConnectedEditors();

    return observedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedFmuRftData* RimObservedDataCollection::createAndAddFmuRftDataFromPath( const QString& directoryPath )
{
    QString name = QString( "Imported FMU RFT Data %1" ).arg( m_observedFmuRftArray.size() + 1 );

    RimObservedFmuRftData* fmuRftData = new RimObservedFmuRftData;
    fmuRftData->setDirectoryPath( directoryPath );
    fmuRftData->createRftReaderInterface();
    fmuRftData->setName( name );
    m_observedFmuRftArray.push_back( fmuRftData );

    updateNewObservedDataCreated( fmuRftData );
    updateConnectedEditors();

    return fmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureDepthData* RimObservedDataCollection::createAndAddPressureDepthDataFromPath( const QString& filePath )
{
    QString name = QString( "Imported Pressure/Depth Data %1" ).arg( m_observedPressureDepthArray.size() + 1 );

    RimPressureDepthData* data = new RimPressureDepthData;
    data->setFilePath( filePath );
    data->createRftReaderInterface();
    data->setName( name );
    m_observedPressureDepthArray.push_back( data );

    updateNewObservedDataCreated( data );
    updateConnectedEditors();

    return data;
}

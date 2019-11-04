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
#include "RimObservedEclipseUserData.h"
#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
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
    CAF_PDM_InitObject( "Observed Data", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_observedDataArray, "ObservedDataArray", "", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_observedFmuRftArray, "ObservedFmuRftDataArray", "", "", "", "" );
    m_observedDataArray.uiCapability()->setUiHidden( true );
    m_observedFmuRftArray.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedDataCollection::~RimObservedDataCollection()
{
    m_observedDataArray.deleteAllChildObjects();
    m_observedFmuRftArray.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::removeObservedSummaryData( RimObservedSummaryData* observedData )
{
    m_observedDataArray.removeChildObject( observedData );
    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedDataCollection::removeObservedFmuRftData( RimObservedFmuRftData* observedFmuRftData )
{
    m_observedFmuRftArray.removeChildObject( observedFmuRftData );
    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedSummaryData*> RimObservedDataCollection::allObservedSummaryData() const
{
    return m_observedDataArray.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedFmuRftData*> RimObservedDataCollection::allObservedFmuRftData() const
{
    return m_observedFmuRftArray.childObjects();
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
    RiuPlotMainWindowTools::selectAsCurrentItem( object );
    RiuPlotMainWindowTools::setExpanded( object );

    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();

    RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
    if ( mpw ) mpw->updateSummaryPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedSummaryData*
    RimObservedDataCollection::createAndAddRsmObservedSummaryDataFromFile( const QString& fileName,
                                                                           QString*       errorText /*= nullptr*/ )
{
    if ( !fileExists( fileName, errorText ) ) return nullptr;

    RimObservedSummaryData*     observedData        = nullptr;
    RimObservedEclipseUserData* columnBasedUserData = new RimObservedEclipseUserData();
    observedData                                    = columnBasedUserData;

    this->m_observedDataArray.push_back( observedData );
    observedData->setSummaryHeaderFileName( fileName );
    observedData->createSummaryReaderInterface();
    observedData->updateMetaData();
    observedData->updateOptionSensitivity();

    if ( errorText && !observedData->errorMessagesFromReader().isEmpty() )
    {
        errorText->append( observedData->errorMessagesFromReader() );
    }

    updateNewObservedDataCreated( observedData );

    this->updateConnectedEditors();

    return observedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedSummaryData*
    RimObservedDataCollection::createAndAddCvsObservedSummaryDataFromFile( const QString& fileName,
                                                                           bool           useSavedFieldsValuesInDialog,
                                                                           QString*       errorText /*= nullptr*/ )
{
    if ( !fileExists( fileName, errorText ) ) return nullptr;

    RimObservedSummaryData* observedData = nullptr;

    RimCsvUserData*                          userData     = new RimCsvUserData();
    RicPasteAsciiDataToSummaryPlotFeatureUi* parseOptions = userData->parseOptions();

    if ( useSavedFieldsValuesInDialog )
    {
        caf::PdmSettings::readFieldsFromApplicationStore( parseOptions );
    }
    parseOptions->setUiModeImport( fileName );

    if ( parseOptions->uiModeImport() != RicPasteAsciiDataToSummaryPlotFeatureUi::UI_MODE_SILENT )
    {
        caf::PdmUiPropertyViewDialog propertyDialog( nullptr, parseOptions, "CSV Import Options", "" );
        if ( propertyDialog.exec() != QDialog::Accepted )
        {
            return nullptr;
        }
    }

    caf::PdmSettings::writeFieldsToApplicationStore( parseOptions );

    // userData->setParseOptions(parseOptionsUi.parseOptions());
    userData->setSummaryHeaderFileName( fileName );
    userData->createSummaryReaderInterface();
    userData->updateMetaData();
    userData->updateOptionSensitivity();

    if ( errorText && !userData->errorMessagesFromReader().isEmpty() )
    {
        errorText->append( userData->errorMessagesFromReader() );
    }

    if ( userData->summaryReader() )
    {
        this->m_observedDataArray.push_back( userData );
        observedData = userData;
    }
    else
    {
        delete userData;
        return nullptr;
    }

    updateNewObservedDataCreated( observedData );

    this->updateConnectedEditors();

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
    this->updateConnectedEditors();

    return fmuRftData;
}

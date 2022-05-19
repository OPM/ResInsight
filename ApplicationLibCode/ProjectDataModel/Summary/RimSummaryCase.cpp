/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSummaryCase.h"

#include "RiaFilePathTools.h"
#include "RiaSummaryTools.h"

#include "RicfCommandObject.h"
#include "RifSummaryReaderInterface.h"

#include "RifEclipseSummaryAddress.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCaseCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfAssert.h"

#include "RiaEnsembleNameTools.h"
#include <QFileInfo>
#include <QRegularExpression>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimSummaryCase, "SummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase::RimSummaryCase()
    : nameChanged( this )
{
    CAF_PDM_InitScriptableObject( "Summary Case", ":/SummaryCase.svg", "", "The Base Class for all Summary Cases" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_displayName, "ShortName", "Display Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_displayNameOption, "NameSetting", "Name Setting" );

    CAF_PDM_InitScriptableField( &m_showSubNodesInTree, "ShowSubNodesInTree", false, "Show Summary Data Sub-Tree" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showSubNodesInTree );

    CAF_PDM_InitScriptableField( &m_useAutoShortName_OBSOLETE, "AutoShortyName", false, "Use Auto Display Name" );
    m_useAutoShortName_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_useAutoShortName_OBSOLETE.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryHeaderFilename, "SummaryHeaderFilename", "Summary Header File" );
    m_summaryHeaderFilename.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_caseId, "Id", -1, "Case ID" );
    m_caseId.registerKeywordAlias( "CaseId" );
    m_caseId.uiCapability()->setUiReadOnly( true );
    m_caseId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &m_dataVectorFolders, "DataVectorFolders", "Data Folders" );
    m_dataVectorFolders = new RimSummaryAddressCollection();
    m_dataVectorFolders.uiCapability()->setUiHidden( true );
    m_dataVectorFolders.xmlCapability()->disableIO();

    m_isObservedData = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase::~RimSummaryCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::summaryHeaderFilename() const
{
    return m_summaryHeaderFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setSummaryHeaderFileName( const QString& fileName )
{
    m_summaryHeaderFilename = fileName;

    this->updateAutoShortName();
    this->updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::isObservedData() const
{
    return m_isObservedData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setCaseRealizationParameters( const std::shared_ptr<RigCaseRealizationParameters>& crlParameters )
{
    m_crlParameters = crlParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigCaseRealizationParameters> RimSummaryCase::caseRealizationParameters() const
{
    return m_crlParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::hasCaseRealizationParameters() const
{
    return m_crlParameters != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimSummaryCase::ensemble() const
{
    RimSummaryCaseCollection* e;
    firstAncestorOrThisOfType( e );
    return e && e->isEnsemble() ? e : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::copyFrom( const RimSummaryCase& rhs )
{
    m_displayName               = rhs.m_displayName;
    m_useAutoShortName_OBSOLETE = rhs.m_useAutoShortName_OBSOLETE;
    m_summaryHeaderFilename     = rhs.m_summaryHeaderFilename;
    m_isObservedData            = rhs.m_isObservedData;

    this->updateTreeItemName();
    this->updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// Sorting operator for sets and maps. Sorts by summary case short name.
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase::operator<( const RimSummaryCase& rhs ) const
{
    return this->caseName() < rhs.caseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    if ( changedField == &m_displayNameOption )
    {
        updateAutoShortName();
        nameChanged.send();
    }
    else if ( changedField == &m_displayName )
    {
        updateTreeItemName();
        nameChanged.send();
    }
    else if ( changedField == &m_showSubNodesInTree )
    {
        updateConnectedEditors();
    }

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateOptionSensitivity()
{
    m_displayName.uiCapability()->setUiReadOnly( m_displayNameOption != RimCaseDisplayNameTools::DisplayName::CUSTOM );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimSummaryCase::rftReader()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::errorMessagesFromReader()
{
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::buildChildNodes()
{
    m_dataVectorFolders->deleteAllChildObjects();

    RifSummaryReaderInterface* reader = summaryReader();
    if ( !reader ) return;

    m_dataVectorFolders->updateFolderStructure( reader->allResultAddresses(), m_caseId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_displayName );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_summaryHeaderFilename );
    uiOrdering.add( &m_caseId );

    if ( ensemble() ) uiOrdering.add( &m_showSubNodesInTree );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( !ensemble() || m_showSubNodesInTree() )
    {
        if ( m_dataVectorFolders->isEmpty() ) buildChildNodes();
        m_dataVectorFolders->updateUiTreeOrdering( uiTreeOrdering );
    }

    uiTreeOrdering.skipRemainingChildren( true );

    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateTreeItemName()
{
    this->setUiName( displayCaseName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::displayCaseName() const
{
    return m_displayName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::nativeCaseName() const
{
    return caseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimSummaryCase::unitsSystem()
{
    RifSummaryReaderInterface* reader = summaryReader();
    if ( reader )
    {
        return reader->unitSystem();
    }
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setDisplayNameOption( RimCaseDisplayNameTools::DisplayName displayNameOption )
{
    m_displayNameOption = displayNameOption;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::initAfterRead()
{
    if ( m_caseId() == -1 )
    {
        RimProject::current()->assignCaseIdToSummaryCase( this );
    }

    if ( m_useAutoShortName_OBSOLETE )
    {
        m_displayNameOption = RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME;
    }

    updateOptionSensitivity();

    refreshMetaData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateAutoShortName()
{
    if ( m_displayNameOption == RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME )
    {
        m_displayName = caseName();
    }
    else if ( m_displayNameOption == RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME )
    {
        if ( ensemble() )
        {
            m_displayName = RiaEnsembleNameTools::uniqueShortNameForEnsembleCase( this );
        }
        else
        {
            m_displayName = RiaEnsembleNameTools::uniqueShortNameForSummaryCase( this );
        }
    }
    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::setCaseId( int caseId )
{
    m_caseId = caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryCase::caseId() const
{
    return m_caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::refreshMetaData()
{
    buildChildNodes();
    updateConnectedEditors();
}

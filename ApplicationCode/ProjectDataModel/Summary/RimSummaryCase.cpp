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

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmFieldScriptingCapability.h"

#include "cvfAssert.h"

#include <QFileInfo>
#include <QRegularExpression>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimSummaryCase, "SummaryCase" );

namespace caf
{
template <>
void AppEnum<RimSummaryCase::DisplayName>::setUp()
{
    addItem( RimSummaryCase::DisplayName::FULL_CASE_NAME, "FULL_CASE_NAME", "Full Case Name" );
    addItem( RimSummaryCase::DisplayName::SHORT_CASE_NAME, "SHORT_CASE_NAME", "Shortened Case Name" );
    addItem( RimSummaryCase::DisplayName::CUSTOM, "CUSTOM_NAME", "Custom Name" );
    setDefault( RimSummaryCase::DisplayName::SHORT_CASE_NAME );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase::RimSummaryCase()
    : nameChanged( this )
{
    CAF_PDM_InitScriptableObject( "Summary Case", ":/SummaryCase16x16.png", "", "The Base Class for all Summary Cases" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_displayName, "ShortName", "Display Name", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_displayNameOption, "NameSetting", "Name Setting", "", "", "" );

    CAF_PDM_InitScriptableField( &m_useAutoShortName_OBSOLETE, "AutoShortyName", false, "Use Auto Display Name", "", "", "" );
    m_useAutoShortName_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_useAutoShortName_OBSOLETE.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_summaryHeaderFilename, "SummaryHeaderFilename", "Summary Header File", "", "", "" );
    m_summaryHeaderFilename.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_caseId, "Id", -1, "Case ID", "", "", "" );
    m_caseId.registerKeywordAlias( "CaseId" );
    m_caseId.uiCapability()->setUiReadOnly( true );
    m_caseId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

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

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateOptionSensitivity()
{
    m_displayName.uiCapability()->setUiReadOnly( m_displayNameOption != DisplayName::CUSTOM );
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
void RimSummaryCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
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
RiaEclipseUnitTools::UnitSystemType RimSummaryCase::unitsSystem()
{
    RifSummaryReaderInterface* reader = summaryReader();
    if ( reader )
    {
        return reader->unitSystem();
    }
    return RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
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
        m_displayNameOption = DisplayName::SHORT_CASE_NAME;
    }

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCase::uniqueShortNameForCase( RimSummaryCase* summaryCase )
{
    QString                      ensembleName;
    std::vector<RimSummaryCase*> summaryCases;

    auto ensemble = summaryCase->ensemble();
    if ( ensemble )
    {
        summaryCases = ensemble->allSummaryCases();
    }
    else
    {
        RimProject::current()->descendantsIncludingThisOfType( summaryCases );
    }

    QRegularExpression trimRe( "^[^a-zA-Z0-9]+" );

    QStringList summaryFilePaths;
    summaryFilePaths.push_back( summaryCase->summaryHeaderFilename() );

    for ( auto otherSummaryCase : summaryCases )
    {
        if ( otherSummaryCase != summaryCase )
        {
            summaryFilePaths.push_back( otherSummaryCase->summaryHeaderFilename() );
        }
    }

    std::map<QString, QStringList> keyFileComponentsForAllFiles =
        RiaFilePathTools::keyPathComponentsForEachFilePath( summaryFilePaths );

    QStringList keyFileComponents = keyFileComponentsForAllFiles[summaryCase->summaryHeaderFilename()];
    CAF_ASSERT( !keyFileComponents.empty() );

    if ( !ensembleName.isEmpty() )
    {
        for ( auto& component : keyFileComponents )
        {
            component = component.replace( ensembleName, "" );
            component = component.replace( trimRe, "" );
        }
    }

    QStringList        shortNameComponents;
    QRegularExpression numberRe( "[0-9]+" );
    for ( auto keyComponent : keyFileComponents )
    {
        QStringList subComponents;
        QString     numberGroup = numberRe.match( keyComponent ).captured();
        if ( !numberGroup.isEmpty() )
        {
            keyComponent = keyComponent.replace( numberGroup, "" );

            QString stem = keyComponent.left( 4 );
            if ( !stem.isEmpty() ) subComponents.push_back( stem );
            subComponents.push_back( numberGroup );
        }
        else
        {
            subComponents.push_back( keyComponent.left( 6 ) );
        }

        shortNameComponents.push_back( subComponents.join( "-" ) );
    }
    return shortNameComponents.join( "," );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCase::updateAutoShortName()
{
    if ( m_displayNameOption == DisplayName::FULL_CASE_NAME )
    {
        m_displayName = caseName();
    }
    else if ( m_displayNameOption == DisplayName::SHORT_CASE_NAME )
    {
        m_displayName = RimSummaryCase::uniqueShortNameForCase( this );
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

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimCase.h"

#include "RiaEnsembleNameTools.h"

#include "RicfCommandObject.h"

#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationNamesCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTimeStepFilter.h"

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimCaseCollection.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectFactory.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimCase, "Case", "RimCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase::RimCase()
    : settingsChanged( this )
    , m_isInActiveDestruction( false )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Case", ":/Case48x48.png", "", "", "Case", "The ResInsight base class for Cases" );

    CAF_PDM_InitScriptableField( &m_caseUserDescription, "Name", QString(), "Case Name" );
    m_caseUserDescription.registerKeywordAlias( "CaseUserDescription" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_displayNameOption, "NameSetting", "Name Setting" );

    CAF_PDM_InitScriptableField( &m_caseId, "Id", -1, "Case ID" );
    m_caseId.registerKeywordAlias( "CaseId" );
    m_caseId.uiCapability()->setUiReadOnly( true );
    m_caseId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitScriptableFieldNoDefault( &m_caseFileName, "FilePath", "Case File Name" );
    m_caseFileName.registerKeywordAlias( "CaseFileName" );
    m_caseFileName.registerKeywordAlias( "GridFileName" );

    m_caseFileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_activeFormationNames, "DefaultFormationNames", "Formation Names File" );

    CAF_PDM_InitFieldNoDefault( &m_timeStepFilter, "TimeStepFilter", "Time Step Filter" );
    m_timeStepFilter.uiCapability()->setUiTreeChildrenHidden( true );
    m_timeStepFilter = new RimTimeStepFilter;

    CAF_PDM_InitFieldNoDefault( &m_2dIntersectionViewCollection, "IntersectionViewCollection", "2D Intersection Views", ":/CrossSections16x16.png" );
    m_2dIntersectionViewCollection = new Rim2dIntersectionViewCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase::~RimCase()
{
    m_isInActiveDestruction = true; // Needed because destruction of m_intersectionViews results in call to views()
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCase::caseId() const
{
    return m_caseId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::setCaseId( int id )
{
    m_caseId = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCase::caseUserDescription() const
{
    return m_caseUserDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::setCaseUserDescription( const QString& description )
{
    m_caseUserDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::setGridFileName( const QString& fileName )
{
    m_caseFileName.v().setPath( fileName );

    updateAutoShortName();
    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCase::gridFileName() const
{
    return m_caseFileName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase::DisplayNameEnum RimCase::displayNameType() const
{
    return m_displayNameOption();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::setDisplayNameType( RimCaseDisplayNameTools::DisplayName displayNameType )
{
    m_displayNameOption = displayNameType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::setCustomCaseName( const QString& caseName )
{
    m_displayNameOption   = RimCaseDisplayNameTools::DisplayName::CUSTOM;
    m_caseUserDescription = caseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimCase::views() const
{
    if ( m_isInActiveDestruction ) return std::vector<Rim3dView*>();

    std::vector<Rim3dView*>             allViews   = allSpecialViews();
    std::vector<Rim2dIntersectionView*> isectViews = m_2dIntersectionViewCollection->views();

    for ( auto view : isectViews )
    {
        allViews.push_back( view );
    }

    return allViews;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> RimCase::gridViews() const
{
    std::vector<RimGridView*> grViews;

    for ( Rim3dView* const view : views() )
    {
        RimGridView* grView = dynamic_cast<RimGridView*>( view );
        if ( grView ) grViews.push_back( grView );
    }
    return grViews;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimCase::displayModelOffset() const
{
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::setFormationNames( RimFormationNames* formationNames )
{
    m_activeFormationNames = formationNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimCase::activeFormationNames() const
{
    return m_activeFormationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimCase::uiToNativeTimeStepIndex( size_t uiTimeStepIndex )
{
    std::vector<size_t> nativeTimeIndices = m_timeStepFilter->filteredTimeSteps();

    if ( !nativeTimeIndices.empty() )
    {
        return nativeTimeIndices.at( uiTimeStepIndex );
    }

    return uiTimeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionViewCollection* RimCase::intersectionViewCollection()
{
    return m_2dIntersectionViewCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_displayNameOption )
    {
        updateAutoShortName();
    }
    else if ( changedField == &m_caseUserDescription )
    {
        updateTreeItemName();
    }

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_activeFormationNames )
    {
        RimProject* proj = RimProject::current();
        if ( proj && proj->activeOilField() && proj->activeOilField()->formationNamesCollection() )
        {
            for ( RimFormationNames* fnames : proj->activeOilField()->formationNamesCollection()->formationNamesList() )
            {
                options.push_back( caf::PdmOptionItemInfo( fnames->fileNameWoPath(), fnames, false, fnames->uiCapability()->uiIconProvider() ) );
            }
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::updateOptionSensitivity()
{
    m_caseUserDescription.uiCapability()->setUiReadOnly( m_displayNameOption != RimCaseDisplayNameTools::DisplayName::CUSTOM );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::initAfterRead()
{
    if ( m_caseId() == -1 )
    {
        RimProject::current()->assignCaseIdToCase( this );
    }

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2020.10.0" ) )
    {
        // Don't mess with case names in older projects: the user can have changed the name.
        m_displayNameOption = RimCaseDisplayNameTools::DisplayName::CUSTOM;
    }

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCase::userDescriptionField()
{
    return &m_caseUserDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::updateAutoShortName()
{
    if ( m_displayNameOption == RimCaseDisplayNameTools::DisplayName::FULL_CASE_NAME )
    {
        m_caseUserDescription = caseName();
    }
    else if ( m_displayNameOption == RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME )
    {
        if ( auto caseCollection = firstAncestorOrThisOfType<RimCaseCollection>() )
        {
            QStringList filePaths;
            auto        cases = caseCollection->cases();
            if ( cases.size() > 1 )
            {
                for ( int i = 0; i < std::min( 4, static_cast<int>( cases.size() ) ); ++i )
                {
                    auto caseInEnsemble = cases[i];
                    filePaths.push_back( caseInEnsemble->gridFileName() );
                }
            }

            filePaths.push_back( gridFileName() );

            m_caseUserDescription = RiaEnsembleNameTools::uniqueShortName( gridFileName(), filePaths, caseName() );
        }
        else
        {
            m_caseUserDescription = RimCase::uniqueShortNameCase( this, RimCaseDisplayNameTools::CASE_SHORT_NAME_LENGTH );
        }
    }
    updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase::updateTreeItemName()
{
    setUiName( m_caseUserDescription() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCase::caseName() const
{
    QFileInfo fileName( gridFileName() );
    return fileName.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCase::uniqueShortNameCase( RimCase* rimCase, int shortNameLengthLimit )
{
    std::set<QString> allAutoShortNames;

    std::vector<RimCase*> allCases = RimProject::current()->descendantsOfType<RimCase>();

    for ( RimCase* rCase : allCases )
    {
        if ( rCase && rCase != rimCase )
        {
            allAutoShortNames.insert( rimCase->caseName() );
        }
    }

    QString caseName = rimCase->caseName();
    return RimCaseDisplayNameTools::uniqueShortName( caseName, allAutoShortNames, shortNameLengthLimit );
}

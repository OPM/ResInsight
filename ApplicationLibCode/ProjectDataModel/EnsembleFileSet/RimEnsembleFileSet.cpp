/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimEnsembleFileSet.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaEnsembleNameTools.h"
#include "RiaFilePathTools.h"
#include "RiaStdStringTools.h"
#include "RiaTextStringTools.h"

#include "RimEnsembleFileSetCollection.h"
#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_SOURCE_INIT( RimEnsembleFileSet, "EnsembleFileSet" );

namespace internal
{
QString placeholderString()
{
    return "*";
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFileSet::RimEnsembleFileSet()
    : fileSetChanged( this )
    , nameChanged( this )

{
    CAF_PDM_InitObject( "Ensemble", ":/CreateGridCaseGroup16x16.png", "", "" );

    CAF_PDM_InitField( &m_pathPattern, "PathPattern", QString(), "Path Pattern", "", "", "" );
    CAF_PDM_InitField( &m_realizationSubSet, "RealizationSubSet", QString(), "Realization SubSet", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_groupingMode, "GroupingMode", "Grouping Mode" );

    CAF_PDM_InitScriptableField( &m_autoName, "CreateAutoName", true, "Auto Name" );
    CAF_PDM_InitScriptableField( &m_useKey1, "UseKey1", false, "Use First Path Part" );
    CAF_PDM_InitScriptableField( &m_useKey2, "UseKey2", false, "Use Second Path Part" );

    QString defaultText = RiaDefines::key1VariableName() + "-" + RiaDefines::key2VariableName();
    QString tooltipText = QString( "Variables in template is supported, and will be replaced to create name. Example '%1'" ).arg( defaultText );
    CAF_PDM_InitField( &m_nameTemplateString, "NameTemplateString", defaultText, "Name Template", "", tooltipText );

    nameField()->uiCapability()->setUiReadOnly( true );
    nameField()->xmlCapability()->disableIO();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEnsembleFileSet::createPaths( const QString& extension ) const
{
    // Append extension to the path pattern and return list of files matching the pattern

    QString pathPattern = m_pathPattern();
    if ( pathPattern.isEmpty() )
    {
        return {};
    }

    pathPattern += extension;

    return RiaEnsembleImportTools::createPathsFromPattern( pathPattern, m_realizationSubSet(), internal::placeholderString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::findAndSetPathPatternAndRangeString( const QStringList& filePaths )
{
    QStringList normalizedPaths;
    for ( const auto& path : filePaths )
    {
        normalizedPaths.append( RiaFilePathTools::toInternalSeparator( path ) );
    }

    const auto& [pattern, rangeString] = RiaEnsembleImportTools::findPathPattern( normalizedPaths, internal::placeholderString() );

    // find the pattern without extension by finding . and remove rest of string
    auto noExtension = pattern;
    auto dotIndex    = noExtension.lastIndexOf( '.' );
    if ( dotIndex != -1 )
    {
        noExtension = noExtension.left( dotIndex );
    }

    m_pathPattern       = noExtension;
    m_realizationSubSet = rangeString;
    fileSetChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::setNameTemplate( const QString& name )
{
    m_nameTemplateString = name;
    fileSetChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::updateName( const std::set<QString>& existingEnsembleNames )
{
    const auto [key1, key2] = nameKeys();

    QString templateText;
    if ( m_autoName )
    {
        templateText = nameTemplateText();
    }
    else
    {
        templateText = m_nameTemplateString();
    }

    std::map<QString, QString> keyValues = {
        { RiaDefines::key1VariableName(), QString::fromStdString( key1 ) },
        { RiaDefines::key2VariableName(), QString::fromStdString( key2 ) },
    };

    auto candidateName = RiaTextStringTools::replaceTemplateTextWithValues( templateText, keyValues );

    if ( m_autoName )
    {
        candidateName = candidateName.trimmed();

        // When using auto name, remove leading and trailing commas that may occur if key1 or key2 is empty
        if ( candidateName.startsWith( "," ) )
        {
            candidateName = candidateName.mid( 1 );
        }
        if ( candidateName.endsWith( "," ) )
        {
            candidateName = candidateName.left( candidateName.length() - 1 );
        }

        auto generateUniqueName = []( const QString& baseName, const std::set<QString>& existingNames )
        {
            // Avoid identical ensemble names by appending a number
            QString uniqueName = baseName;
            int     counter    = 1;
            while ( existingNames.find( uniqueName ) != existingNames.end() )
            {
                uniqueName = QString( "%1_%2" ).arg( baseName ).arg( counter++ );
            }
            return uniqueName;
        };

        candidateName = generateUniqueName( candidateName, existingEnsembleNames );
    }

    if ( name() == candidateName ) return;

    setName( candidateName );
    nameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::setUsePathKey1( bool useKey1 )
{
    m_useKey1 = useKey1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::setUsePathKey2( bool useKey2 )
{
    m_useKey2 = useKey2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_pathPattern )
    {
        if ( auto lineEdAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            lineEdAttr->placeholderText = "Enter path pattern...";
        }
    }
    else if ( field == &m_realizationSubSet )
    {
        if ( auto lineEdAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            lineEdAttr->placeholderText = "E.g. 0,1,2-5,10-20";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_autoName );
    if ( !m_autoName() )
    {
        uiOrdering.add( &m_nameTemplateString );
    }

    uiOrdering.add( nameField() );
    uiOrdering.add( &m_pathPattern );
    uiOrdering.add( &m_realizationSubSet );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_nameTemplateString || changedField == &m_autoName )
    {
        RimProject::current()->ensembleFileSetCollection()->updateFileSetNames();
    }
    else
    {
        fileSetChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicCreateEnsembleFromFileSetFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::setPathPattern( const QString& pathPattern )
{
    m_pathPattern = pathPattern;
    fileSetChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::setRangeString( const QString& rangeString )
{
    m_realizationSubSet = rangeString;
    fileSetChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string> RimEnsembleFileSet::nameKeys() const
{
    std::string key1 = "Undefined KEY1";
    std::string key2 = "Undefined KEY2";

    if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE )
    {
        auto pathPattern = m_pathPattern + ".SMSPEC";
        auto paths       = RiaEnsembleImportTools::createPathsFromPattern( pathPattern, "0", internal::placeholderString() );
        if ( !paths.empty() )
        {
            auto fileNames = RiaEnsembleNameTools::groupFilePathsFmu( { paths.front().toStdString() } );
            if ( !fileNames.empty() )
            {
                key1 = fileNames.begin()->first.first;
                key2 = fileNames.begin()->first.second;
            }
        }
    }
    else if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE )
    {
        auto pathPattern = m_pathPattern + ".SMSPEC";
        auto paths       = RiaEnsembleImportTools::createPathsFromPattern( pathPattern, "0-1", internal::placeholderString() );
        if ( paths.size() > 1 )
        {
            auto name1 = RiaFilePathTools::toInternalSeparator( paths[0] ).toStdString();
            auto name2 = RiaFilePathTools::toInternalSeparator( paths[1] ).toStdString();

            auto parts1 = RiaStdStringTools::splitString( name1, '/' );
            auto parts2 = RiaStdStringTools::splitString( name2, '/' );

            size_t commonParts = 0;
            for ( size_t i = 0; i < std::min( parts1.size(), parts2.size() ); i++ )
            {
                if ( parts1[i] == parts2[i] )
                {
                    commonParts++;
                }
                else
                {
                    break;
                }
            }

            if ( commonParts == 1 )
            {
                key2 = parts1[commonParts - 1];
            }
            else if ( commonParts > 1 )
            {
                key1 = parts1[commonParts - 2];
                key2 = parts1[commonParts - 1];
            }
        }
    }

    return { key1, key2 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleFileSet::nameTemplateText() const
{
    QString text;
    if ( m_useKey1() ) text += RiaDefines::key1VariableName();
    if ( m_useKey2() )
    {
        if ( !text.isEmpty() ) text += ", ";
        text += RiaDefines::key2VariableName();
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSet::setGroupingMode( RiaDefines::EnsembleGroupingMode groupingMode )
{
    m_groupingMode = groupingMode;
}

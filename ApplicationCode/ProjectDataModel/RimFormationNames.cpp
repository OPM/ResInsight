/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RimFormationNames.h"

#include "RigFormationNames.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"
#include "RiuPlotMainWindowTools.h"

#include "cafAssert.h"
#include "cafPdmUiFilePathEditor.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimFormationNames, "FormationNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames::RimFormationNames()
{
    CAF_PDM_InitObject( "Formation Names", ":/Formations16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_formationNamesFileName, "FormationNamesFileName", "File Name", "", "", "" );

    m_formationNamesFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames::~RimFormationNames()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( &m_formationNamesFileName == changedField )
    {
        updateUiTreeName();
        QString errorMessage;
        readFormationNamesFile( &errorMessage );
        if ( !errorMessage.isEmpty() )
        {
            QMessageBox::warning( nullptr, "Formation Names", errorMessage );
        }
        updateConnectedViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::setFileName( const QString& fileName )
{
    m_formationNamesFileName = fileName;

    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationNames::fileName()
{
    return m_formationNamesFileName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationNames::fileNameWoPath()
{
    QFileInfo fnameFileInfo( m_formationNamesFileName().path() );
    return fnameFileInfo.fileName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateConnectedViews()
{
    std::vector<RimCase*> objects;
    this->objectsWithReferringPtrFieldsOfType( objects );

    bool updatedTracks = false;
    for ( RimCase* caseObj : objects )
    {
        if ( caseObj )
        {
            caseObj->updateFormationNamesData();

            std::vector<RimWellLogTrack*> tracks;
            caseObj->objectsWithReferringPtrFieldsOfType( tracks );
            for ( RimWellLogTrack* track : tracks )
            {
                // The track may be referring to the case for other reasons than formations.
                if ( track->formationNamesCase() == caseObj )
                {
                    track->loadDataAndUpdate();
                    updatedTracks = true;
                }
            }
        }
    }
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::readFormationNamesFile( QString* errorMessage )
{
    QFile dataFile( m_formationNamesFileName().path() );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        if ( errorMessage )
            ( *errorMessage ) += "Could not open the File: " + ( m_formationNamesFileName().path() ) + "\n";
        return;
    }

    m_formationNamesData = new RigFormationNames;
    QTextStream stream( &dataFile );

    QFileInfo fileInfo( m_formationNamesFileName().path() );

    if ( fileInfo.fileName() == RimFormationNames::layerZoneTableFileName() )
    {
        readFmuFormationNameFile( stream, errorMessage );
    }
    else
    {
        readLyrFormationNameFile( stream, errorMessage );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    // m_formationNamesFileName =
    //     RimTools::relocateFile( m_formationNamesFileName(), newProjectPath, oldProjectPath, nullptr, nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationNames::layerZoneTableFileName()
{
    return "layer_zone_table.txt";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateUiTreeName()
{
    this->uiCapability()->setUiName( fileNameWoPath() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::readLyrFormationNameFile( QTextStream& stream, QString* errorMessage )
{
    int lineNumber = 1;
    while ( !stream.atEnd() )
    {
        QString     line     = stream.readLine();
        QStringList lineSegs = line.split( "'", QString::KeepEmptyParts );

        if ( lineSegs.size() == 0 ) continue; // Empty line
        if ( lineSegs.size() == 1 ) continue; // No name present. Comment line ?
        if ( lineSegs.size() == 2 )
        {
            if ( errorMessage ) ( *errorMessage ) += "Missing quote on line : " + QString::number( lineNumber ) + "\n";
            continue; // One quote present
        }

        if ( lineSegs.size() == 3 ) // Normal case
        {
            if ( lineSegs[0].contains( "--" ) ) continue; // Comment line
            QString formationName  = lineSegs[1];
            int     commentMarkPos = lineSegs[2].indexOf( "--" );
            QString numberString   = lineSegs[2];
            if ( commentMarkPos >= 0 ) numberString.truncate( commentMarkPos );

            QStringList numberWords = numberString.split( QRegExp( "-" ), QString::SkipEmptyParts );
            if ( numberWords.size() == 2 )
            {
                bool isNumber1 = false;
                bool isNumber2 = false;
                int  startK    = numberWords[0].toInt( &isNumber1 );
                int  endK      = numberWords[1].toInt( &isNumber2 );

                if ( !( isNumber2 && isNumber1 ) )
                {
                    if ( errorMessage )
                        ( *errorMessage ) += "Format error on line: " + QString::number( lineNumber ) + "\n";
                    continue;
                }

                int tmp = startK;
                startK  = tmp < endK ? tmp : endK;
                endK    = tmp > endK ? tmp : endK;

                m_formationNamesData->appendFormationRange( formationName, startK - 1, endK - 1 );
            }
            else if ( numberWords.size() == 1 )
            {
                bool isNumber1   = false;
                int  kLayerCount = numberWords[0].toInt( &isNumber1 );

                if ( !isNumber1 )
                {
                    if ( errorMessage )
                        ( *errorMessage ) += "Format error on line: " + QString::number( lineNumber ) + "\n";
                    continue;
                }

                m_formationNamesData->appendFormationRangeHeight( formationName, kLayerCount );
            }
            else
            {
                if ( errorMessage )
                    ( *errorMessage ) += "Format error on line: " + QString::number( lineNumber ) + "\n";
            }
        }

        ++lineNumber;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::readFmuFormationNameFile( QTextStream& stream, QString* errorMessage )
{
    int lineNumber = 1;

    QString currentFormationName;
    int     startK = -1;
    int     endK   = -1;

    while ( !stream.atEnd() )
    {
        QString line = stream.readLine();
        if ( line.isNull() )
        {
            // Make sure we append the last formation
            if ( !currentFormationName.isEmpty() )
            {
                m_formationNamesData->appendFormationRange( currentFormationName, startK - 1, endK - 1 );
            }
            break;
        }
        else
        {
            QTextStream lineStream( &line );

            double  kLayer;
            QString formationName;

            lineStream >> kLayer >> formationName;

            if ( lineStream.status() != QTextStream::Ok )
            {
                *errorMessage = QString( "Failed to parse line %1 of '%2'" )
                                    .arg( lineNumber )
                                    .arg( m_formationNamesFileName().path() );
                return;
            }

            if ( formationName != currentFormationName )
            {
                // Append previous formation
                if ( !currentFormationName.isEmpty() )
                {
                    m_formationNamesData->appendFormationRange( currentFormationName, startK - 1, endK - 1 );
                }

                // Start new formation
                currentFormationName = formationName;
                startK               = kLayer;
                endK                 = kLayer;
            }
            else
            {
                endK = kLayer;
            }
        }
    }

    // Append previous formation at the end of the stream
    if ( !currentFormationName.isEmpty() )
    {
        m_formationNamesData->appendFormationRange( currentFormationName, startK - 1, endK - 1 );
    }
}

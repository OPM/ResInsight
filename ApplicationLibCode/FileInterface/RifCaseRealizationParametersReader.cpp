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

#include "RifCaseRealizationParametersReader.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaTextStringTools.h"

#include "RifFileParseTools.h"
#include "RifOpmSummaryTools.h"

#include <QDir>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationReader::RifCaseRealizationReader( const QString& fileName )
{
    m_parameters = std::make_shared<RigCaseRealizationParameters>();
    m_fileName   = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationReader::~RifCaseRealizationReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::shared_ptr<RigCaseRealizationParameters> RifCaseRealizationReader::parameters() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RifCaseRealizationReader> RifCaseRealizationReader::createReaderFromFileName( const QString& fileName )
{
    std::shared_ptr<RifCaseRealizationReader> reader;

    if ( fileName.endsWith( parametersFileName() ) )
    {
        reader = std::make_shared<RifCaseRealizationParametersReader>( fileName );
    }
    else if ( fileName.endsWith( runSpecificationFileName() ) )
    {
        reader = std::make_shared<RifCaseRealizationRunspecificationReader>( fileName );
    }
    return reader;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationReader::parametersFileName()
{
    return "parameters.txt";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationReader::runSpecificationFileName()
{
    return "runspecification.xml";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::RifCaseRealizationParametersReader( const QString& fileName )
    : RifCaseRealizationReader( fileName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::~RifCaseRealizationParametersReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::parse()
{
    QFile file( m_fileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

    QTextStream dataStream( &file );

    int         lineNo = 0;
    QStringList errors;

    const auto decimalPoint = RiaStdStringTools::decimalPoint();

    while ( !dataStream.atEnd() )
    {
        QString line = dataStream.readLine();

        lineNo++;

        const auto stdLine                = line.toStdString();
        const auto trimmedLine            = RiaStdStringTools::trimString( stdLine );
        const auto [name, parameterValue] = RiaStdStringTools::splitAtWhitespace( trimmedLine );

        if ( name.empty() || parameterValue.empty() )
        {
            errors << QString( "RifCaseRealizationParametersReader: Invalid file format in line %1" ).arg( lineNo );
            continue;
        }

        if ( RiaStdStringTools::isNumber( parameterValue, decimalPoint ) )
        {
            double doubleValue = 0.0;
            if ( RiaStdStringTools::toDouble( parameterValue, doubleValue ) )
            {
                m_parameters->addParameter( QString::fromStdString( std::string( name ) ), doubleValue );
            }
            else
            {
                errors << QString( "RifCaseRealizationParametersReader: Invalid number format in line %1" ).arg( lineNo );
            }
        }
        else
        {
            m_parameters->addParameter( QString::fromStdString( std::string( name ) ), QString::fromStdString( std::string( parameterValue ) ) );
        }
    }

    for ( const auto& s : errors )
    {
        RiaLogging::warning( s );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationRunspecificationReader::RifCaseRealizationRunspecificationReader( const QString& fileName )
    : RifCaseRealizationReader( fileName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationRunspecificationReader::~RifCaseRealizationRunspecificationReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationRunspecificationReader::parse()
{
    QFile file( m_fileName );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

    QXmlStreamReader xml( &file );

    QStringList errors;

    QString paramName;

    while ( !xml.atEnd() )
    {
        xml.readNext();

        if ( xml.isStartElement() )
        {
            if ( RiaTextStringTools::isTextEqual( xml.name(), QString( "modifier" ) ) )
            {
                paramName = "";
            }

            if ( RiaTextStringTools::isTextEqual( xml.name(), QString( "id" ) ) )
            {
                paramName = xml.readElementText();
            }

            if ( RiaTextStringTools::isTextEqual( xml.name(), QString( "value" ) ) )
            {
                QString paramStrValue = xml.readElementText();

                if ( paramName.isEmpty() ) continue;

                if ( RiaTextStringTools::isNumber( paramStrValue, QLocale::c().decimalPoint() ) )
                {
                    bool   parseOk = true;
                    double value   = QLocale::c().toDouble( paramStrValue, &parseOk );
                    if ( parseOk )
                    {
                        m_parameters->addParameter( paramName, value );
                    }
                    else
                    {
                        errors
                            << QString( "RifCaseRealizationRunspecificationReader: Invalid number format in line %1" ).arg( xml.lineNumber() );
                    }
                }
                else
                {
                    m_parameters->addParameter( paramName, paramStrValue );
                }
            }
        }
        else if ( xml.isEndElement() )
        {
            if ( RiaTextStringTools::isTextEqual( xml.name(), QString( "modifier" ) ) )
            {
                paramName = "";
            }
        }
    }

    for ( const auto& s : errors )
    {
        RiaLogging::warning( s );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationParametersFileLocator::locate( const QString& modelPath )
{
    // Chosen to find parameters file for StimPlan ensembles.
    int MAX_LEVELS_UP = 5;
    int dirLevel      = 0;

    QDir qdir( modelPath );

    const QFileInfo dir( modelPath );
    if ( dir.isFile() )
        qdir.cdUp();
    else if ( !dir.isDir() )
        return "";

    do
    {
        QStringList files = qdir.entryList( QDir::Files | QDir::NoDotAndDotDot );
        for ( const QString& file : files )
        {
            if ( QString::compare( file, RifCaseRealizationReader::parametersFileName(), Qt::CaseInsensitive ) == 0 ||
                 QString::compare( file, RifCaseRealizationReader::runSpecificationFileName(), Qt::CaseInsensitive ) == 0 )
            {
                return qdir.absoluteFilePath( file );
            }
        }
        qdir.cdUp();

    } while ( dirLevel++ < MAX_LEVELS_UP );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifCaseRealizationParametersFileLocator::realizationNumber( const QString& modelPath )
{
    QDir    dir( modelPath );
    QString absolutePath = dir.absolutePath();

    auto realizationNumber = RifOpmSummaryTools::extractRealizationNumber( absolutePath );
    if ( realizationNumber.has_value() )
    {
        return realizationNumber.value();
    }

    return -1;
}

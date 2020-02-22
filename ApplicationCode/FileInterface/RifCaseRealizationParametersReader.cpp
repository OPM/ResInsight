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
#include "RifFileParseTools.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include <QDir>
#include <QString>
#include <QStringList>

#include <functional>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationReader::RifCaseRealizationReader( const QString& fileName )
{
    m_parameters = std::shared_ptr<RigCaseRealizationParameters>( new RigCaseRealizationParameters() );
    m_fileName   = fileName;
    m_file       = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationReader::~RifCaseRealizationReader()
{
    closeFile();
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
        reader.reset( new RifCaseRealizationParametersReader( fileName ) );
    }
    else if ( fileName.endsWith( runSpecificationFileName() ) )
    {
        reader.reset( new RifCaseRealizationRunspecificationReader( fileName ) );
    }
    return reader;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFile* RifCaseRealizationReader::openFile()
{
    if ( !m_file )
    {
        m_file = new QFile( m_fileName );
        if ( !m_file->open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            closeFile();
            throw FileParseException( QString( "Failed to open %1" ).arg( m_fileName ) );
        }
    }
    return m_file;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationReader::closeFile()
{
    if ( m_file )
    {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
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
    m_textStream = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::~RifCaseRealizationParametersReader()
{
    if ( m_textStream )
    {
        delete m_textStream;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::parse()
{
    int          lineNo     = 0;
    QTextStream* dataStream = openDataStream();

    QStringList errors;

    try
    {
        while ( !dataStream->atEnd() )
        {
            QString line = dataStream->readLine();

            lineNo++;
            QStringList cols = RifFileParseTools::splitLineAndTrim( line, QRegExp( "[ \t]" ), true );

            if ( cols.size() != 2 )
            {
                errors << QString( "RifEnsembleParametersReader: Invalid file format in line %1" ).arg( lineNo );

                continue;
            }

            QString& name     = cols[0];
            QString& strValue = cols[1];

            if ( RiaStdStringTools::isNumber( strValue.toStdString(), QLocale::c().decimalPoint().toLatin1() ) )
            {
                bool   parseOk = true;
                double value   = QLocale::c().toDouble( strValue, &parseOk );
                if ( parseOk )
                {
                    m_parameters->addParameter( name, value );
                }
                else
                {
                    errors << QString( "RifEnsembleParametersReader: Invalid number format in line %1" ).arg( lineNo );
                }
            }
            else
            {
                m_parameters->addParameter( name, strValue );
            }
        }

        closeDataStream();
    }
    catch ( ... )
    {
        closeDataStream();
        throw;
    }

    for ( const auto& s : errors )
    {
        RiaLogging::warning( s );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream* RifCaseRealizationParametersReader::openDataStream()
{
    auto file = openFile();

    m_textStream = new QTextStream( file );
    return m_textStream;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::closeDataStream()
{
    if ( m_textStream )
    {
        delete m_textStream;
        m_textStream = nullptr;
    }
    closeFile();
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
    auto             file = openFile();
    QXmlStreamReader xml( file );

    QStringList errors;

    QString paramName;

    while ( !xml.atEnd() )
    {
        xml.readNext();

        if ( xml.isStartElement() )
        {
            if ( xml.name() == "modifier" )
            {
                paramName = "";
            }

            if ( xml.name() == "id" )
            {
                paramName = xml.readElementText();
            }

            if ( xml.name() == "value" )
            {
                QString paramStrValue = xml.readElementText();

                if ( paramName.isEmpty() ) continue;

                if ( RiaStdStringTools::isNumber( paramStrValue.toStdString(), QLocale::c().decimalPoint().toLatin1() ) )
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
                            << QString( "RifCaseRealizationRunspecificationReader: Invalid number format in line %1" )
                                   .arg( xml.lineNumber() );
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
            if ( xml.name() == "modifier" )
            {
                paramName = "";
            }
        }
    }

    closeFile();

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
    int MAX_LEVELS_UP = 3;
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

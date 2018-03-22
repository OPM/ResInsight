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

#include "RifEnsambleParametersReader.h"
#include "RifFileParseTools.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include <QString>
#include <QStringList>
#include <QDir>


//--------------------------------------------------------------------------------------------------
/// Constants
//--------------------------------------------------------------------------------------------------
#define PARAMETERS_FILE_NAME    "parameters.txt"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEnsambleParametersReader::RifEnsambleParametersReader(const QString& fileName)
{
    m_fileName = fileName;
    m_file = nullptr;
    m_textStream = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEnsambleParametersReader::~RifEnsambleParametersReader()
{
    if (m_textStream)
    {
        delete m_textStream;
    }
    closeFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEnsambleParametersReader::parse()
{
    bool            errors = false;
    QTextStream*    dataStream = openDataStream();
    int             lineNo = 0;

    try
    {
        while (!dataStream->atEnd() && !errors)
        {
            QString line = dataStream->readLine();

            lineNo++;
            QStringList cols = RifFileParseTools::splitLineAndTrim(line, " ");

            if (cols.size() != 2)
            {
                throw FileParseException(QString("RifEnsambleParametersReader: Invalid file format in line %1").arg(lineNo));
            }

            QString& name = cols[0];
            QString& strValue = cols[1];

            if (!RiaStdStringTools::isNumber(strValue.toStdString(), QLocale::c().decimalPoint().toAscii()))
            {
                throw FileParseException(QString("RifEnsambleParametersReader: Invalid number format in line %1").arg(lineNo));
            }

            bool parseOk = true;
            double value = QLocale::c().toDouble(strValue, &parseOk);
            if (!parseOk)
            {
                throw FileParseException(QString("RifEnsambleParametersReader: Invalid number format in line %1").arg(lineNo));
            }

            m_parameters.addParameter(name, value);
        }

        closeDataStream();
    }
    catch (...)
    {
        closeDataStream();
        throw;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream* RifEnsambleParametersReader::openDataStream()
{
    if (!openFile()) return nullptr;

    m_textStream = new QTextStream(m_file);
    return m_textStream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEnsambleParametersReader::closeDataStream()
{
    if (m_textStream)
    {
        delete m_textStream;
        m_textStream = nullptr;
    }
    closeFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEnsambleParametersReader::openFile()
{
    if (!m_file)
    {
        m_file = new QFile(m_fileName);
        if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            RiaLogging::error(QString("Failed to open %1").arg(m_fileName));

            delete m_file;
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEnsambleParametersReader::closeFile()
{
    if (m_file)
    {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RifEnsambleParameters& RifEnsambleParametersReader::parameters() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEnsambleParameters::addParameter(const QString& name, double value)
{
    m_parameters[name] = value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, double> RifEnsambleParameters::parameters() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifEnsambleParametersFileLocator::locate(const QString& modelPath)
{
    int         MAX_LEVELS_UP = 2;

    int         dirLevel = 0;
    QDir        qdir(modelPath);

    do
    {
        QStringList files = qdir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& file : files)
        {
            if (QString::compare(file, PARAMETERS_FILE_NAME, Qt::CaseInsensitive) == 0)
            {
                return file;
            }
        }

    } while (dirLevel++ == MAX_LEVELS_UP);

    return "";
}

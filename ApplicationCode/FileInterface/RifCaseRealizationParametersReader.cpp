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
RifCaseRealizationParametersReader::RifCaseRealizationParametersReader(const QString& fileName)
{
    m_parameters = std::shared_ptr<RigCaseRealizationParameters>(new RigCaseRealizationParameters());
    m_fileName = fileName;
    m_file = nullptr;
    m_textStream = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::RifCaseRealizationParametersReader()
{
    m_parameters = std::shared_ptr<RigCaseRealizationParameters>(new RigCaseRealizationParameters());
    m_fileName = "";
    m_file = nullptr;
    m_textStream = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifCaseRealizationParametersReader::~RifCaseRealizationParametersReader()
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
void RifCaseRealizationParametersReader::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::parse()
{
    int             lineNo = 0;
    QTextStream*    dataStream = openDataStream();

    try
    {
        while (!dataStream->atEnd())
        {
            QString line = dataStream->readLine();

            lineNo++;
            QStringList cols = RifFileParseTools::splitLineAndTrim(line, " ");

            if (cols.size() != 2)
            {
                throw FileParseException(QString("RifEnsembleParametersReader: Invalid file format in line %1").arg(lineNo));
            }

            QString& name = cols[0];
            QString& strValue = cols[1];

            if (RiaStdStringTools::startsWithAlphabetic(strValue.toStdString()))
            {
                m_parameters->addParameter(name, strValue);
            }
            else
            {
                if (!RiaStdStringTools::isNumber(strValue.toStdString(), QLocale::c().decimalPoint().toAscii()))
                {
                    throw FileParseException(QString("RifEnsembleParametersReader: Invalid number format in line %1").arg(lineNo));
                }

                bool parseOk = true;
                double value = QLocale::c().toDouble(strValue, &parseOk);
                if (!parseOk)
                {
                    throw FileParseException(QString("RifEnsembleParametersReader: Invalid number format in line %1").arg(lineNo));
                }

                m_parameters->addParameter(name, value);
            }
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
QTextStream* RifCaseRealizationParametersReader::openDataStream()
{
    openFile();

    m_textStream = new QTextStream(m_file);
    return m_textStream;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::closeDataStream()
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
void RifCaseRealizationParametersReader::openFile()
{
    if (!m_file)
    {
        m_file = new QFile(m_fileName);
        if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            closeFile();
            //delete m_file;
            //m_file = nullptr;
            throw FileParseException(QString("Failed to open %1").arg(m_fileName));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifCaseRealizationParametersReader::closeFile()
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
const std::shared_ptr<RigCaseRealizationParameters> RifCaseRealizationParametersReader::parameters() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifCaseRealizationParametersFileLocator::locate(const QString& modelPath)
{
    int         MAX_LEVELS_UP = 2;
    int         dirLevel = 0;

    QDir        qdir(modelPath);

    const QFileInfo dir(modelPath);
    if (dir.isFile()) qdir.cdUp();
    else if (!dir.isDir()) return "";

    do
    {
        QStringList files = qdir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& file : files)
        {
            if (QString::compare(file, PARAMETERS_FILE_NAME, Qt::CaseInsensitive) == 0)
            {
                return qdir.absoluteFilePath(file);
            }
        }
        qdir.cdUp();

    } while (dirLevel++ < MAX_LEVELS_UP);

    return "";
}

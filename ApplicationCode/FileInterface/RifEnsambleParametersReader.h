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

#pragma once

#include "RiaPreferences.h"

#include "RifSummaryCaseRestartSelector.h"

#include <QString>
#include <QTextStream>

#include <string>
#include <vector>
#include <map>

class QStringList;
class QTextStream;
class QFile;

//==================================================================================================
//
//
//==================================================================================================
class RifEnsambleParameters
{
public:
    void                        addParameter(const QString& name, double value);
    std::map<QString, double>   parameters() const;

private:
    std::map<QString, double> m_parameters;
};

//==================================================================================================
//
//
//==================================================================================================
class RifEnsambleParametersReader
{
public:
    RifEnsambleParametersReader(const QString& fileName);
    ~RifEnsambleParametersReader();

    void                            parse();
    const RifEnsambleParameters&    parameters() const;

private:
    QTextStream*                    openDataStream();
    void                            closeDataStream();
    bool                            openFile();
    void                            closeFile();
private:
    RifEnsambleParameters   m_parameters;

    QString                 m_fileName;
    QFile*                  m_file;
    QTextStream*            m_textStream;
};

//==================================================================================================
//
//
//==================================================================================================
class RifEnsambleParametersFileLocator
{
public:
    static QString locate(const QString& modelPath);
};

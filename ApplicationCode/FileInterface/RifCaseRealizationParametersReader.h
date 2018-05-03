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

#include "RigCaseRealizationParameters.h"

#include <QString>
#include <QTextStream>

#include <string>
#include <vector>
#include <map>
#include <memory>

class QStringList;
class QTextStream;
class QFile;

//==================================================================================================
//
//
//==================================================================================================
class RifCaseRealizationParametersReader
{
public:
    RifCaseRealizationParametersReader();
    RifCaseRealizationParametersReader(const QString& fileName);
    ~RifCaseRealizationParametersReader();

    void                                                setFileName(const QString& fileName);
    void                                                parse();
    const std::shared_ptr<RigCaseRealizationParameters> parameters() const;

private:
    QTextStream*                    openDataStream();
    void                            closeDataStream();
    void                            openFile();
    void                            closeFile();
private:
    std::shared_ptr<RigCaseRealizationParameters>  m_parameters;

    QString                                 m_fileName;
    QFile*                                  m_file;
    QTextStream*                            m_textStream;
};


//==================================================================================================
//
//
//==================================================================================================
class RifCaseRealizationParametersFileLocator
{
public:
    static QString locate(const QString& modelPath);
};

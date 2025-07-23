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

#include "RifSummaryCaseRestartSelector.h"

#include "RigCaseRealizationParameters.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

#include <map>
#include <memory>
#include <string>
#include <vector>

class QTextStream;
class QXmlStreamReader;
class QFile;

//==================================================================================================
//
//
//==================================================================================================
class RifCaseRealizationReader
{
public:
    RifCaseRealizationReader( const QString& fileName );
    virtual ~RifCaseRealizationReader();

    virtual void                                        parse() = 0;
    const std::shared_ptr<RigCaseRealizationParameters> parameters() const;

    static std::shared_ptr<RifCaseRealizationReader> createReaderFromFileName( const QString& fileName );

    static QString parametersFileName();
    static QString runSpecificationFileName();

protected:
    std::shared_ptr<RigCaseRealizationParameters> m_parameters;

    QString m_fileName;
};

//==================================================================================================
//
//
//==================================================================================================
class RifCaseRealizationParametersReader : public RifCaseRealizationReader
{
public:
    RifCaseRealizationParametersReader( const QString& fileName );
    ~RifCaseRealizationParametersReader() override;

    void parse() override;
};

//==================================================================================================
//
//
//==================================================================================================
class RifCaseRealizationRunspecificationReader : public RifCaseRealizationReader
{
public:
    RifCaseRealizationRunspecificationReader( const QString& fileName );
    ~RifCaseRealizationRunspecificationReader() override;

    void parse() override;
};

//==================================================================================================
//
//
//==================================================================================================
class RifCaseRealizationParametersFileLocator
{
public:
    static QString locate( const QString& modelPath );
    static int     realizationNumber( const QString& modelPath );
};

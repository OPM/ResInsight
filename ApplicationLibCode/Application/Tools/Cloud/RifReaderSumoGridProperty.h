/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RifReaderInterface.h"

#include <QByteArray>
#include <QPointer>
#include <QString>

#include <map>
#include <vector>

class RiaSumoConnector;

//==================================================================================================
//
// Lazily fetches grid cell properties for a single Sumo grid realization. ResInsight's result
// machinery calls staticResult()/dynamicResult() the first time a property is displayed; this reader
// then downloads the corresponding roff blob from Sumo and decodes the cell values.
//
//==================================================================================================
class RifReaderSumoGridProperty : public RifReaderInterface
{
public:
    RifReaderSumoGridProperty( RiaSumoConnector* connector,
                               const QString&    caseId,
                               const QString&    ensembleName,
                               const QString&    gridName,
                               int               realization );

    void setStaticProperties( const std::vector<QString>& propertyNames );
    void setDynamicProperties( const std::map<QString, std::vector<QString>>& propertyNameToTimestamps );

    bool open( const QString& fileName, RigEclipseCaseData* eclipseCase ) override;

    bool staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values ) override;
    bool dynamicResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values ) override;

private:
    bool fetchAndDecode( const QString& propertyName, const QString& isoDateOrInterval, std::vector<double>* values );
    void prefetchFromTimeStep( const QString& propertyName, const std::vector<QString>& timestamps, size_t stepIndex );

private:
    QPointer<RiaSumoConnector> m_connector;
    QString                    m_caseId;
    QString                    m_ensembleName;
    QString                    m_gridName;
    int                        m_realization;

    RigEclipseCaseData* m_caseData; // set in open(); used for cell count and active cell masking

    std::vector<QString>                    m_staticProperties;
    std::map<QString, std::vector<QString>> m_dynamicTimestamps; // property name -> sorted iso timestamps
};

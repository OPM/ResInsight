/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-  Equinor ASA
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

#include "RifEclipseRftAddress.h"
#include "RifReaderRftInterface.h"

#include <map>
#include <string>
#include <vector>

#include "cvfObject.h"

#include <QDateTime>
#include <QDir>

//==================================================================================================
//
//
//==================================================================================================
class RifReaderFmuRft : public RifReaderRftInterface, public cvf::Object
{
public:
    struct Observation
    {
        double utmx;
        double utmy;
        double mdrkb;
        double tvdmsl;

        double  pressure;
        double  pressureError;
        QString formation;

        Observation();

        bool valid() const;
    };

    struct WellObservationSet
    {
        QDateTime                dateTime;
        int                      measurementIndex;
        std::vector<Observation> observations;

        WellObservationSet( const QDateTime& dateTime, int measurementIndex );
    };

public:
    RifReaderFmuRft( const QString& filePath );
    ~RifReaderFmuRft() override = default;

    static QStringList findSubDirectoriesWithFmuRftData( const QString& filePath );
    static bool        directoryContainsFmuRftData( const QString& filePath );
    static QString     wellPathFileName();

    std::vector<QString> labels( const RifEclipseRftAddress& rftAddress );

    std::set<RifEclipseRftAddress> eclipseRftAddresses() override;
    void values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) override;

    std::set<QDateTime>
                        availableTimeSteps( const QString&                                               wellName,
                                            const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) override;
    std::set<QDateTime> availableTimeSteps( const QString& wellName ) override;
    std::set<QDateTime> availableTimeSteps( const QString&                                     wellName,
                                            const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) override;

    std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) override;
    std::set<QString>                                     wellNames() override;

public:
    void load();

private:
    typedef std::pair<const QString, WellObservationSet> WellObservationPair;
    typedef std::map<QString, WellObservationSet>        WellObservationMap;

    WellObservationMap loadWellDates( QDir& dir, QString* errorMsg );
    static bool readTxtFile( const QString& txtFileName, QString* errorMsg, WellObservationSet* wellObservationSet );
    static bool readObsFile( const QString& obsFileName, QString* errorMsg, WellObservationSet* wellObservationSet );

private:
    QString            m_filePath;
    WellObservationMap m_allWellObservations;
};

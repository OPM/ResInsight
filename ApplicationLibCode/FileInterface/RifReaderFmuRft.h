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

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <QDateTime>
#include <QDir>

//==================================================================================================
//
//
//==================================================================================================
class RifReaderFmuRft : public RifReaderRftInterface
{
private:
    struct WellDate
    {
        QString   wellName;
        QDateTime dateTime;
        int       measurementId;
    };

    struct Location
    {
        double  utmx;
        double  utmy;
        double  mdrkb;
        double  tvdmsl;
        QString formation;
    };

    struct Observation
    {
        WellDate wellDate;
        Location location;
        double   pressure;
        double   pressureError;
    };

public:
    RifReaderFmuRft( const QString& filePath );
    ~RifReaderFmuRft() override = default;

    static QStringList findSubDirectoriesWithFmuRftData( const QString& filePath );
    static bool        directoryContainsFmuRftData( const QString& filePath );
    static QString     wellPathFileName();

    std::vector<QString> labels( const RifEclipseRftAddress& rftAddress );

    std::set<QString> formationNames( const QString& wellName );
    std::set<QString> formationNames( const QString& wellName, const QDateTime& timeStep );

    // Returns the measured depth (MDRKB) interval covering the given formation's observation point(s),
    // for the given well and time step. The interval extends halfway towards neighboring observation
    // points (sorted by MD) so that formations with a single observation point still get a usable,
    // non-degenerate depth range. Returns std::nullopt if no matching points exist.
    std::optional<std::pair<double, double>>
        formationDepthRange( const QString& wellName, const QDateTime& timeStep, const QString& formationName );

    // Interpolates the TVD (MSL) range corresponding to the given MD (RKB) range, using the
    // well/time step's own observed MD<->TVD relationship. Used to filter simulated RFT depth
    // samples that are only available as TVD (e.g. when no grid case is available to derive MD
    // from well-path intersections), so an MD-based depth/formation filter still applies
    // consistently. Returns std::nullopt if fewer than two observation points exist to interpolate.
    std::optional<std::pair<double, double>>
        convertMdRangeToTvd( const QString& wellName, const QDateTime& timeStep, double mdMin, double mdMax );

    // Computes the mean observed pressure and mean observed pressure error for the given well/time
    // step, optionally restricted to observation points within [mdMin, mdMax] (MD RKB). Returns
    // std::nullopt if no matching observation points exist.
    std::optional<std::pair<double, double>>
        observedPressureAndError( const QString& wellName, const QDateTime& timeStep, bool useDepthRange, double mdMin, double mdMax );

    std::set<RifEclipseRftAddress> eclipseRftAddresses() override;
    void                           values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) override;

    std::set<QDateTime> availableTimeSteps( const QString&                                               wellName,
                                            const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) override;
    std::set<QDateTime> availableTimeSteps( const QString& wellName ) override;
    std::set<QDateTime> availableTimeSteps( const QString&                                     wellName,
                                            const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) override;

    std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) override;
    std::set<QString>                                     wellNames() override;

    void importData();

private:
    static std::vector<WellDate> importWellDates( const QString& fileName );
    static std::vector<Location> importLocations( const QString& fileName );
    static std::vector<Observation>
        importObservations( const QString& fileName, const std::vector<Location>& locations, const WellDate& wellDate );

    std::vector<const Observation*> sortedObservationsForWellDate( const QString& wellName, const QDateTime& timeStep );

    static double interpolateTvdFromMd( const std::vector<const Observation*>& sortedObservations, double md );

private:
    QString                  m_filePath;
    std::vector<Observation> m_observations;
};

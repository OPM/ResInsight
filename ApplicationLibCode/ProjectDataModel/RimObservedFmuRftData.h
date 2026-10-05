/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RifReaderFmuRft.h"

#include "RimNamedObject.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <memory>
#include <optional>
#include <utility>

class RimObservedFmuRftData : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimObservedFmuRftData();

    void                   setDirectoryPath( const QString& path );
    void                   createRftReaderInterface();
    RifReaderRftInterface* rftReader();

    bool                 hasWell( const QString& wellPathName ) const;
    std::vector<QString> wells() const;
    std::vector<QString> labels( const RifEclipseRftAddress& rftAddress );

    std::vector<QString> formationNames( const QString& wellPathName, const QDateTime& timeStep );

    std::optional<std::pair<double, double>>
        formationDepthRange( const QString& wellPathName, const QDateTime& timeStep, const QString& formationName );

    // Interpolates the TVD (MSL) range corresponding to the given MD (RKB) range, using the
    // well/time step's own observed MD<->TVD relationship. See RifReaderFmuRft::convertMdRangeToTvd.
    std::optional<std::pair<double, double>>
        convertMdRangeToTvd( const QString& wellPathName, const QDateTime& timeStep, double mdMin, double mdMax );

    // Computes the mean observed pressure and mean observed pressure error for the given well/time
    // step, optionally restricted to the given MD (RKB) range.
    std::optional<std::pair<double, double>>
        observedPressureAndError( const QString& wellPathName, const QDateTime& timeStep, bool useDepthRange, double mdMin, double mdMax );

protected:
    void initAfterRead() override;

private:
    std::unique_ptr<RifReaderFmuRft> m_fmuRftReader;

    caf::PdmField<caf::FilePath>                  m_directoryPath;
    caf::PdmField<QString>                        m_directoryPath_OBSOLETE;
    caf::PdmProxyValueField<std::vector<QString>> m_wells;
};

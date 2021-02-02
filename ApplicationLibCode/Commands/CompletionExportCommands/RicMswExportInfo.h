/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RiaDefines.h"

#include "RicMswBranch.h"
#include "RicMswSegment.h"

#include <QString>

#include <memory>

class RimWellPath;
class RimFishbones;

//==================================================================================================
///
//==================================================================================================
class RicMswExportInfo
{
public:
    RicMswExportInfo( const RimWellPath*            wellPath,
                      RiaDefines::EclipseUnitSystem unitSystem,
                      double                        initialMD,
                      const QString&                lengthAndDepthText,
                      const QString&                pressureDropText );

    void setLinerDiameter( double linerDiameter );
    void setRoughnessFactor( double roughnessFactor );
    void setHasSubGridIntersections( bool subGridIntersections );

    const RimWellPath*            wellPath() const;
    RiaDefines::EclipseUnitSystem unitSystem() const;
    double                        topWellBoreVolume() const;
    double                        linerDiameter() const;
    double                        roughnessFactor() const;
    QString                       lengthAndDepthText() const;
    QString                       pressureDropText() const;
    bool                          hasSubGridIntersections() const;
    static double                 defaultDoubleValue();

    const RicMswBranch* mainBoreBranch() const;
    RicMswBranch*       mainBoreBranch();

private:
    const RimWellPath*            m_wellPath;
    RiaDefines::EclipseUnitSystem m_unitSystem;
    double                        m_topWellBoreVolume;
    double                        m_linerDiameter;
    double                        m_roughnessFactor;
    QString                       m_lengthAndDepthText;
    QString                       m_pressureDropText;
    bool                          m_hasSubGridIntersections;

    std::unique_ptr<RicMswBranch> m_mainBoreBranch;
};

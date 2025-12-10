/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimWellPath.h"

class RimOsduWellPath : public RimWellPath
{
    CAF_PDM_HEADER_INIT;

public:
    RimOsduWellPath();
    ~RimOsduWellPath() override;

    void    setWellId( const QString& wellId );
    QString wellId() const;

    void    setWellboreId( const QString& wellboreId );
    QString wellboreId() const;

    void    setWellboreTrajectoryId( const QString& wellboreTrajectoryId );
    QString wellboreTrajectoryId() const;

    void   setDatumElevationFromOsdu( double datumElevationFromOsdu );
    double datumElevationFromOsdu() const;

    void    setExistenceKind( const QString& existenceKind );
    QString existenceKind() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<QString> m_wellId;
    caf::PdmField<QString> m_wellboreId;
    caf::PdmField<QString> m_wellboreTrajectoryId;
    caf::PdmField<QString> m_existenceKind;
    caf::PdmField<double>  m_datumElevationFromOsdu;
};

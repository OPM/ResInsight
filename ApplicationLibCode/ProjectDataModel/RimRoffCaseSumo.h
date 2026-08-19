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

#include "RimEclipseCase.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QPointer>
#include <QString>

class RiaSumoConnector;
class RimSumoDataSource;

//==================================================================================================
//
// Eclipse grid case backed by a roff grid stored on Sumo. The grid geometry is downloaded as a
// blob through RiaSumoConnector and parsed in memory, so there is no grid file on disk.
//
//==================================================================================================
class RimRoffCaseSumo : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimRoffCaseSumo();
    ~RimRoffCaseSumo() override;

    // Create a grid case for a single realization of the given grid, linked back to the data source
    // so the case can be updated when the data source realization filter changes.
    static RimRoffCaseSumo* createFromDataSource( RimSumoDataSource* dataSource, const QString& gridName, int realization );

    void setSumoDataSource( RimSumoDataSource* dataSource );
    void setSumoCaseId( const QString& sumoCaseId );
    void setEnsembleName( const QString& ensembleName );
    void setGridName( const QString& gridName );
    void setRealization( int realization );

    QString gridName() const;
    int     realization() const;

    bool openEclipseGridFile() override;

    QString locationOnDisc() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    // Download the roff grid blob for this realization and parse it into the case data.
    bool downloadAndParseGrid();

    // Register the results and attach the property reader. Shared by the two ways the grid can end up on
    // this case: parsed and kept, or parsed and replaced by the grid shared with the other realizations.
    void finalizeCaseSetup();

    // Discover the available grid properties from Sumo, register them as cell results and attach a reader
    // that fetches the property data on demand. Static properties only in this first version.
    void registerSumoGridProperties();

private:
    caf::PdmPtrField<RimSumoDataSource*> m_sumoDataSource;
    caf::PdmField<QString>               m_sumoCaseId;
    caf::PdmField<QString>               m_ensembleName;
    caf::PdmField<QString>               m_gridName;
    caf::PdmField<int>                   m_realization;

    QPointer<RiaSumoConnector> m_sumoConnector;
};

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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "Cloud/RiaSumoConnector.h"

#include <QPointer>
#include <QString>

class RimSummarySumoDataSource;

//==================================================================================================
///
///
//==================================================================================================
class RimCloudDataSourceCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCloudDataSourceCollection();

    static RimCloudDataSourceCollection* instance();

    std::vector<RimSummarySumoDataSource*> sumoDataSources() const;

    static void createEnsemblesFromSelectedDataSources( const std::vector<RimSummarySumoDataSource*>& dataSources );

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    std::vector<RimSummarySumoDataSource*> addDataSources();
    void                                   addEnsembles();

private:
    caf::PdmField<QString>              m_sumoFieldName;
    caf::PdmField<QString>              m_sumoCaseId;
    caf::PdmField<std::vector<QString>> m_sumoEnsembleNames;

    caf::PdmField<bool>                                m_addDataSources;
    caf::PdmField<bool>                                m_addEnsembles;
    caf::PdmChildArrayField<RimSummarySumoDataSource*> m_sumoDataSources;

    QPointer<RiaSumoConnector> m_sumoConnector;
};

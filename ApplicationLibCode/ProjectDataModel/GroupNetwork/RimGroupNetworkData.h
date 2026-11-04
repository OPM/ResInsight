/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifGroupNetworkSummaryData.h"
#include "RigGroupNetwork.h"

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include <ctime>
#include <memory>
#include <vector>

class RimSummaryCase;

//==================================================================================================
/// A production network tree read from a deck (GRUPTREE or BRANPROP), with node and edge values from
/// a summary case. This is a data source, shown in the data source tree. The network is shown by one
/// or more RimGroupNetworkPlot objects.
//==================================================================================================
class RimGroupNetworkData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using TreeTypeEnum = caf::AppEnum<RigGroupNetwork::TreeType>;

    caf::Signal<> dataChanged;

public:
    RimGroupNetworkData();
    ~RimGroupNetworkData() override;

    QString         name() const;
    RimSummaryCase* summaryCase() const;
    void            setSummaryCase( RimSummaryCase* summaryCase );

    const RigGroupNetwork*               network() const;
    const RifGroupNetworkSummaryData*    summaryData() const;
    RigGroupNetwork::TreeType            treeType() const;
    QString                              statusText( const RigGroupNetwork::TreeRevision* shownTree ) const;
    const std::vector<std::time_t>&      timeSteps() const;
    const RigGroupNetwork::TreeRevision* treeAtDate( std::time_t date ) const;

    void reload();

    static QString dateText( std::time_t date );

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          initAfterRead() override;
    void                          appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void ensureLoaded() const;
    void clearCache();
    void clearTreeTypeCache();
    void updateDeckFile( const RimSummaryCase* previousCase );

private:
    caf::PdmField<QString>            m_name;
    caf::PdmPtrField<RimSummaryCase*> m_summaryCase;
    caf::PdmField<caf::FilePath>      m_deckFile;
    caf::PdmField<TreeTypeEnum>       m_treeType;

    mutable bool                                        m_isLoaded = false;
    mutable QString                                     m_loadError;
    mutable std::unique_ptr<RigGroupNetwork>            m_network;
    mutable std::unique_ptr<RifGroupNetworkSummaryData> m_summaryData;
    mutable std::vector<std::time_t>                    m_networkDates;
};

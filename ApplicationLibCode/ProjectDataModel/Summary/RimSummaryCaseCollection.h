/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "RifEclipseRftAddress.h"
#include "RifEclipseSummaryAddress.h"
#include "RifReaderEnsembleStatisticsRft.h"

#include "RigEnsembleParameter.h"

#include "RimObjectiveFunction.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <QString>

#include <utility>
#include <vector>

class RifReaderRftInterface;
class RifReaderEnsembleStatisticsRft;
class RimSummaryCase;

//==================================================================================================
///
//==================================================================================================
class RimSummaryCaseCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<>                caseNameChanged;
    caf::Signal<RimSummaryCase*> caseRemoved;

public:
    RimSummaryCaseCollection();
    ~RimSummaryCaseCollection() override;

    void                                       removeCase( RimSummaryCase* summaryCase );
    void                                       addCase( RimSummaryCase* summaryCase );
    virtual std::vector<RimSummaryCase*>       allSummaryCases() const;
    void                                       setName( const QString& name );
    QString                                    name() const;
    bool                                       isEnsemble() const;
    void                                       setAsEnsemble( bool isEnsemble );
    virtual std::set<RifEclipseSummaryAddress> ensembleSummaryAddresses() const;
    virtual std::set<time_t>                   ensembleTimeSteps() const;
    std::set<QString>                          wellsWithRftData() const;
    std::set<QDateTime>                        rftTimeStepsForWell( const QString& wellName ) const;
    RifReaderRftInterface*                     rftStatisticsReader();
    void                                       setEnsembleId( int ensembleId );
    int                                        ensembleId() const;
    bool                                       hasEnsembleParameters() const;

    std::vector<RigEnsembleParameter> variationSortedEnsembleParameters( bool excludeNoVariation = false ) const;
    std::vector<std::pair<RigEnsembleParameter, double>>
        correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address ) const;
    std::vector<std::pair<RigEnsembleParameter, double>>
        correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address, time_t selectedTimeStep ) const;
    std::vector<std::pair<RigEnsembleParameter, double>>
        parameterCorrelations( const RifEclipseSummaryAddress&  address,
                               time_t                           selectedTimeStep,
                               const std::vector<QString>&      selectedParameters = {},
                               const std::set<RimSummaryCase*>& selectedCases      = {} ) const;

    std::vector<std::pair<RigEnsembleParameter, double>>
        parameterCorrelationsAllTimeSteps( const RifEclipseSummaryAddress& address,
                                           const std::vector<QString>&     selectedParameters = {} ) const;

    std::vector<RigEnsembleParameter> alphabeticEnsembleParameters() const;

    RigEnsembleParameter ensembleParameter( const QString& paramName ) const;
    void                 calculateEnsembleParametersIntersectionHash();
    void                 clearEnsembleParametersHashes();

    void loadDataAndUpdate();

    static bool validateEnsembleCases( const std::vector<RimSummaryCase*> cases );
    bool        operator<( const RimSummaryCaseCollection& rhs ) const;

    RiaDefines::EclipseUnitSystem unitSystem() const;

private:
    RigEnsembleParameter createEnsembleParameter( const QString& paramName ) const;
    static void          sortByBinnedVariation( std::vector<RigEnsembleParameter>& parameterVector );
    friend class RimSummaryCaseCollection_TESTER;

    caf::PdmFieldHandle* userDescriptionField() override;
    QString              nameAndItemCount() const;
    void                 updateIcon();

    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onCaseNameChanged( const SignalEmitter* emitter );

protected:
    virtual void onLoadDataAndUpdate();
    void         updateReferringCurveSets();
    void         defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void         setNameAsReadOnly();

    caf::PdmChildArrayField<RimSummaryCase*> m_cases;

private:
    caf::PdmField<QString>           m_name;
    caf::PdmProxyValueField<QString> m_nameAndItemCount;
    caf::PdmField<bool>              m_isEnsemble;

    cvf::ref<RifReaderEnsembleStatisticsRft> m_statisticsEclipseRftReader;
    caf::PdmField<int>                       m_ensembleId;

    size_t m_commonAddressCount; // if different address count among cases, set to 0

    mutable std::vector<RigEnsembleParameter> m_cachedSortedEnsembleParameters;
};

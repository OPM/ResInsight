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

#include "RiaEclipseUnitTools.h"
#include "RifEclipseRftAddress.h"
#include "RifEclipseSummaryAddress.h"
#include "RifReaderEnsembleStatisticsRft.h"

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
class EnsembleParameter
{
public:
    enum Type
    {
        TYPE_NONE,
        TYPE_NUMERIC,
        TYPE_TEXT
    };
    enum Bins
    {
        NO_VARIATION  = -1,
        LOW_VARIATION = 0,
        MEDIUM_VARIATION,
        HIGH_VARIATION,
        NR_OF_VARIATION_BINS
    };
    QString               uiName() const;
    QString               name;
    Type                  type;
    std::vector<QVariant> values;
    double                minValue;
    double                maxValue;
    int                   variationBin;

    EnsembleParameter()
        : type( TYPE_NONE )
        , minValue( std::numeric_limits<double>::infinity() )
        , maxValue( -std::numeric_limits<double>::infinity() )
        , variationBin( static_cast<int>( MEDIUM_VARIATION ) )
    {
    }

    bool   isValid() const { return !name.isEmpty() && type != TYPE_NONE; }
    bool   isNumeric() const { return type == TYPE_NUMERIC; }
    bool   isText() const { return type == TYPE_TEXT; }
    double normalizedStdDeviation() const;

    bool operator<( const EnsembleParameter& other ) const;

private:
    double stdDeviation() const;
};

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

    const std::vector<EnsembleParameter>& variationSortedEnsembleParameters( bool excludeNoVariation = false ) const;
    std::vector<std::pair<EnsembleParameter, double>>
        parameterCorrelations( const RifEclipseSummaryAddress& address,
                               time_t                          selectedTimeStep,
                               bool                            spearman           = false,
                               const std::vector<QString>&     selectedParameters = {} ) const;
    std::vector<std::pair<EnsembleParameter, double>>
        parameterCorrelationsAllTimeSteps( const RifEclipseSummaryAddress& address,
                                           bool                            spearman           = false,
                                           const std::vector<QString>&     selectedParameters = {} ) const;

    std::vector<EnsembleParameter> alphabeticEnsembleParameters() const;

    EnsembleParameter ensembleParameter( const QString& paramName ) const;
    void              calculateEnsembleParametersIntersectionHash();
    void              clearEnsembleParametersHashes();

    void loadDataAndUpdate();

    static bool validateEnsembleCases( const std::vector<RimSummaryCase*> cases );
    bool        operator<( const RimSummaryCaseCollection& rhs ) const;

    RiaEclipseUnitTools::UnitSystem unitSystem() const;

private:
    EnsembleParameter createEnsembleParameter( const QString& paramName ) const;
    static void       sortByBinnedVariation( std::vector<EnsembleParameter>& parameterVector );
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

    mutable std::vector<EnsembleParameter> m_cachedSortedEnsembleParameters;
};

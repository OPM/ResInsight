#pragma once

#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

#include <map>
#include <string>
#include <vector>

class RimMockSummaryCase : public RimSummaryCase, public RifSummaryReaderInterface
{
public:
    void setName( const QString& name ) { m_name = name; }

    void addVector( const RifEclipseSummaryAddress& address,
                    const std::vector<time_t>&      timeSteps,
                    const std::vector<double>&      values,
                    const std::string&              unit = "" )
    {
        m_data[address] = { timeSteps, values, unit };
        m_allResultAddresses.insert( address );
    }

    // RimSummaryCase overrides
    void                       createSummaryReaderInterface() override {}
    RifSummaryReaderInterface* summaryReader() override { return this; }
    QString                    caseName() const override { return m_name; }

    // RifSummaryReaderInterface overrides
    std::vector<time_t> timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override
    {
        auto it = m_data.find( resultAddress );
        if ( it != m_data.end() ) return it->second.timeSteps;
        return {};
    }

    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override
    {
        auto it = m_data.find( resultAddress );
        if ( it != m_data.end() ) return { true, it->second.values };
        return { false, {} };
    }

    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override
    {
        auto it = m_data.find( resultAddress );
        if ( it != m_data.end() ) return it->second.unit;
        return {};
    }

    RiaDefines::EclipseUnitSystem unitSystem() const override { return RiaDefines::EclipseUnitSystem::UNITS_METRIC; }

    size_t keywordCount() const override { return m_allResultAddresses.size(); }

private:
    struct VectorData
    {
        std::vector<time_t> timeSteps;
        std::vector<double> values;
        std::string         unit;
    };

    QString                                        m_name = "MockCase";
    std::map<RifEclipseSummaryAddress, VectorData> m_data;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimObservedRsmspecColumnBasedData.h"
#include "RifSummaryReaderInterface.h"

CAF_PDM_SOURCE_INIT(RimObservedRsmspecColumnBasedData, "RimObservedRsmspecColumnBasedData");


class MyTestInterface : public RifSummaryReaderInterface
{

public:
    MyTestInterface()
    {
        m_timeSteps.push_back(1000);
        m_timeSteps.push_back(1010);
        m_timeSteps.push_back(1020);
        m_timeSteps.push_back(1030);
        m_timeSteps.push_back(1040);

        m_headers.push_back("sdflkj");
        m_headers.push_back("sdf");
        m_headers.push_back("sdfffff");
        m_headers.push_back("qwqwqw");
        m_headers.push_back("aaaaa");
        m_headers.push_back("absc");

    }
//     caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory> >  m_summaryCategory;
//     caf::PdmField<QString>                                                      m_identifierName;

    void setMetaData(RifEclipseSummaryAddress::SummaryVarCategory summaryCategory, const QString& identifierName)
    {
        m_category = summaryCategory;
        m_identifierName = identifierName;

        buildMetaData();
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual const std::vector<time_t>& timeSteps(const RifEclipseSummaryAddress& resultAddress) const override
    {
        return m_timeSteps;
    }


    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual bool values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) override
    {
        values->push_back(20);
        values->push_back(40);
        values->push_back(50);
        values->push_back(40);

        return true;
    }


    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual std::string unitName(const RifEclipseSummaryAddress& resultAddress) override
    {
        return "Unknown unit";
    }

private:
    void buildMetaData()
    {
        m_allResultAddresses.clear();

        for (size_t i = 0; i < m_headers.size(); i++)
        {
            RifEclipseSummaryAddress addr(m_category,
                                            m_headers[i].toStdString(),
                                            -1,
                                            -1,
                                            "",
                                            "",
                                            -1,
                                            "",
                                            -1,
                                            -1,
                                            -1);

            m_allResultAddresses.push_back(addr);
        }
    }

private:
    std::vector<time_t> m_timeSteps;
    std::vector<QString> m_headers;

    RifEclipseSummaryAddress::SummaryVarCategory m_category;
    QString m_identifierName;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedRsmspecColumnBasedData::RimObservedRsmspecColumnBasedData()
{
    CAF_PDM_InitObject("Observed RSMSPEC Column Based Data File", ":/Default.png", "", "");
    m_summaryHeaderFilename.uiCapability()->setUiName("File");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedRsmspecColumnBasedData::~RimObservedRsmspecColumnBasedData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedRsmspecColumnBasedData::setSummaryHeaderFilename(const QString& fileName)
{
    m_summaryHeaderFilename = fileName;

    this->updateAutoShortName();
    this->updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedRsmspecColumnBasedData::createSummaryReaderInterface()
{
    m_summeryReader = new MyTestInterface;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimObservedRsmspecColumnBasedData::summaryReader()
{
    return m_summeryReader.p();
}

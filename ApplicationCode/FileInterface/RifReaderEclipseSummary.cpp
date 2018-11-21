/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifReaderEclipseSummary.h"
#include "RiaStringEncodingTools.h"
#include "RiaFilePathTools.h"
#include "RifReaderEclipseOutput.h"
#include "RifEclipseSummaryTools.h"

#include <string>
#include <assert.h>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDir>

#include "ert/ecl/ecl_sum.h"
#include "ert/ecl/smspec_node.h"
#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_kw.h"

std::vector<time_t> getTimeSteps(ecl_sum_type* ecl_sum)
{
    std::vector<time_t> timeSteps;

    if (ecl_sum)
    {
        time_t_vector_type* steps = ecl_sum_alloc_time_vector(ecl_sum, false);

        if (steps)
        {
            for (int i = 0; i < time_t_vector_size(steps); i++)
            {
                timeSteps.push_back(time_t_vector_iget(steps, i));
            }
            free(steps);
        }
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ecl_sum_type* openEclSum(const QString& inHeaderFileName, bool includeRestartFiles)
{
    QString headerFileName;
    QStringList dataFileNames;
    QString nativeHeaderFileName = QDir::toNativeSeparators(inHeaderFileName);
    RifEclipseSummaryTools::findSummaryFiles(nativeHeaderFileName, &headerFileName, &dataFileNames);

    if (headerFileName.isEmpty() || dataFileNames.size() == 0) return nullptr;

    assert(!headerFileName.isEmpty());
    assert(dataFileNames.size() > 0);

    stringlist_type* dataFiles = stringlist_alloc_new();
    for (int i = 0; i < dataFileNames.size(); i++)
    {
        stringlist_append_copy(dataFiles, RiaStringEncodingTools::toNativeEncoded(dataFileNames[i]).data());
    }

    bool lazyLoad = true;
    std::string itemSeparatorInVariableNames = ":";
    ecl_sum_type* ecl_sum = ecl_sum_fread_alloc(RiaStringEncodingTools::toNativeEncoded(headerFileName).data(),
                                                dataFiles,
                                                itemSeparatorInVariableNames.data(),
                                                includeRestartFiles,
                                                lazyLoad,
                                                ECL_FILE_CLOSE_STREAM);

    stringlist_free(dataFiles);

    return ecl_sum;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void closeEclSum(ecl_sum_type* ecl_sum)
{
    if(ecl_sum) ecl_sum_free(ecl_sum);
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::RifReaderEclipseSummary()
    : m_ecl_sum(nullptr), 
      m_ecl_SmSpec(nullptr)
{
    m_valuesCache.reset(new ValuesCache());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::~RifReaderEclipseSummary()
{
    if (m_ecl_sum)
    {
        ecl_sum_free(m_ecl_sum);
        m_ecl_sum = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::open(const QString& headerFileName, bool includeRestartFiles)
{
    assert(m_ecl_sum == nullptr); 
    
    m_ecl_sum = openEclSum(headerFileName, includeRestartFiles);

    if (m_ecl_sum)
    {
        m_timeSteps.clear();
        m_ecl_SmSpec = ecl_sum_get_smspec(m_ecl_sum);
        m_timeSteps = getTimeSteps(m_ecl_sum);

        buildMetaData();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifRestartFileInfo> RifReaderEclipseSummary::getRestartFiles(const QString& headerFileName, bool* hasWarnings)
{
    CVF_ASSERT(hasWarnings);

    std::vector<RifRestartFileInfo> restartFiles;
    m_warnings.clear();
    *hasWarnings = false;

    RifRestartFileInfo currFile;
    currFile.fileName = headerFileName;
    while(!currFile.fileName.isEmpty())
    {
        // Due to a weakness in libecl regarding restart summary header file selection,
        // do some extra checking
        {
            QString formattedHeaderExtension = ".FSMSPEC";
            QString nonformattedHeaderExtension = ".SMSPEC";
            QString formattedDataFileExtension = ".FUNSMRY";

            if (currFile.fileName.endsWith(nonformattedHeaderExtension, Qt::CaseInsensitive))
            {
                QString formattedHeaderFile = currFile.fileName;
                formattedHeaderFile.replace(nonformattedHeaderExtension, formattedHeaderExtension, Qt::CaseInsensitive);
                QString formattedDateFile = currFile.fileName;
                formattedDateFile.replace(nonformattedHeaderExtension, formattedDataFileExtension, Qt::CaseInsensitive);

                QFileInfo nonformattedHeaderFileInfo = QFileInfo(currFile.fileName);
                QFileInfo formattedHeaderFileInfo = QFileInfo(formattedHeaderFile);
                QFileInfo formattedDateFileInfo = QFileInfo(formattedDateFile);
                if (formattedHeaderFileInfo.lastModified() < nonformattedHeaderFileInfo.lastModified() &&
                    formattedHeaderFileInfo.exists() && !formattedDateFileInfo.exists())
                {
                    m_warnings.push_back(QString("RifReaderEclipseSummary: Formatted summary header file without an\n") +
                                         QString("associated data file detected.\n") +
                                         QString("This may cause a failure reading summary origin data.\n") +
                                         QString("To avoid this problem, please delete or rename the.FSMSPEC file."));
                    *hasWarnings = true;
                    break;
                }
            }
            QString prevFile = currFile.fileName;
            currFile = getRestartFile(currFile.fileName);

            // Fix to stop potential infinite loop
            if (currFile.fileName == prevFile)
            {
                m_warnings.push_back("RifReaderEclipseSummary: Restart file reference loop detected");
                *hasWarnings = true;
                break;
            }

        }

        if (!currFile.fileName.isEmpty())
            restartFiles.push_back(currFile);
    }
    return restartFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RifReaderEclipseSummary::getFileInfo(const QString& headerFileName)
{
    RifRestartFileInfo          fileInfo;
    ecl_sum_type*               ecl_sum = openEclSum(headerFileName, false);
    std::vector<time_t>         timeSteps = getTimeSteps(ecl_sum);
    if (timeSteps.size() > 0)
    {
        fileInfo.fileName = headerFileName;
        fileInfo.startDate = timeSteps.front();
        fileInfo.endDate = timeSteps.back();
    }
    closeEclSum(ecl_sum);
    return fileInfo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress addressFromErtSmSpecNode(const smspec_node_type * ertSumVarNode)
{
    if (smspec_node_get_var_type(ertSumVarNode) == ECL_SMSPEC_INVALID_VAR) 
    {
        return RifEclipseSummaryAddress();
    }

    RifEclipseSummaryAddress::SummaryVarCategory sumCategory(RifEclipseSummaryAddress::SUMMARY_INVALID);
    std::string        quantityName;
    int                regionNumber(-1);
    int                regionNumber2(-1);
    std::string        wellGroupName;
    std::string        wellName;
    int                wellSegmentNumber(-1);
    std::string        lgrName;
    int                cellI(-1);
    int                cellJ(-1);
    int                cellK(-1);
    int                aquiferNumber(-1);
    bool               isErrorResult(false);

    quantityName = smspec_node_get_keyword(ertSumVarNode);

    switch (smspec_node_get_var_type(ertSumVarNode))
    {
        case ECL_SMSPEC_AQUIFER_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
            aquiferNumber = smspec_node_get_num(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_WELL_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL;
            wellName = smspec_node_get_wgname(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_REGION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_REGION;
            regionNumber = smspec_node_get_num(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_FIELD_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_FIELD;
        }
        break;
        case ECL_SMSPEC_GROUP_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
            wellGroupName = smspec_node_get_wgname(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_BLOCK_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_BLOCK;
            
            const int* ijk = smspec_node_get_ijk(ertSumVarNode);
            cellI = ijk[0];
            cellJ = ijk[1];
            cellK = ijk[2];
        }
        break;
        case ECL_SMSPEC_COMPLETION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            const int* ijk = smspec_node_get_ijk(ertSumVarNode);
            cellI = ijk[0];
            cellJ = ijk[1];
            cellK = ijk[2];
        }
        break;
        case ECL_SMSPEC_LOCAL_BLOCK_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR;
            lgrName = smspec_node_get_lgr_name(ertSumVarNode);
            const int* ijk = smspec_node_get_lgr_ijk(ertSumVarNode);
            cellI = ijk[0];
            cellJ = ijk[1];
            cellK = ijk[2];
        }
        break;
        case ECL_SMSPEC_LOCAL_COMPLETION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            lgrName = smspec_node_get_lgr_name(ertSumVarNode);
            const int* ijk = smspec_node_get_lgr_ijk(ertSumVarNode);
            cellI = ijk[0];
            cellJ = ijk[1];
            cellK = ijk[2];
        }
        break;
        case ECL_SMSPEC_LOCAL_WELL_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_LGR;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            lgrName = smspec_node_get_lgr_name(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_NETWORK_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_NETWORK;
        }
        break;
        case ECL_SMSPEC_REGION_2_REGION_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION;
            regionNumber = smspec_node_get_R1(ertSumVarNode);
            regionNumber2 = smspec_node_get_R2(ertSumVarNode);

        }
        break;
        case ECL_SMSPEC_SEGMENT_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
            wellName = smspec_node_get_wgname(ertSumVarNode);
            wellSegmentNumber = smspec_node_get_num(ertSumVarNode);
        }
        break;
        case ECL_SMSPEC_MISC_VAR:
        {
            sumCategory = RifEclipseSummaryAddress::SUMMARY_MISC;
        }
        break;
        default:
            CVF_ASSERT(false);
        break;
    }

    return RifEclipseSummaryAddress(sumCategory, 
                                    quantityName, 
                                    regionNumber, 
                                    regionNumber2, 
                                    wellGroupName, 
                                    wellName, 
                                    wellSegmentNumber, 
                                    lgrName, 
                                    cellI, cellJ, cellK,
                                    aquiferNumber,
                                    isErrorResult);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseSummary::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    assert(m_ecl_sum != nullptr);

    int variableIndex = indexFromAddress(resultAddress);

    if ( variableIndex < 0 ) return false;

    values->clear();
    values->reserve(timeStepCount());

    const std::vector<double>& cachedValues = m_valuesCache->getValues(resultAddress);
    if (!cachedValues.empty())
    {
        values->insert(values->begin(), cachedValues.begin(), cachedValues.end());
    }
    else if (m_ecl_SmSpec)
    {
        const smspec_node_type* ertSumVarNode = ecl_smspec_iget_node(m_ecl_SmSpec, variableIndex);
        int paramsIndex = smspec_node_get_params_index(ertSumVarNode);

        double_vector_type* dataValues = ecl_sum_alloc_data_vector(m_ecl_sum, paramsIndex, false);

        if (dataValues)
        {
            for (int i = 0; i < double_vector_size(dataValues); i++)
            {
                values->push_back(double_vector_iget(dataValues, i));
            }
            free(dataValues);

            m_valuesCache->insertValues(resultAddress, *values);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseSummary::timeStepCount() const
{
    assert(m_ecl_sum != nullptr);

    if (m_ecl_SmSpec == nullptr) return 0;

    return ecl_sum_get_data_length(m_ecl_sum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifReaderEclipseSummary::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    assert(m_ecl_sum != nullptr);

    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseSummary::indexFromAddress(const RifEclipseSummaryAddress& resultAddress) const
{
    auto it = m_resultAddressToErtNodeIdx.find(resultAddress);
    if (it != m_resultAddressToErtNodeIdx.end())
    {
        return it->second;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::buildMetaData()
{
    m_allResultAddresses.clear();
    m_resultAddressToErtNodeIdx.clear();

    if(m_ecl_SmSpec)
    {
        int varCount = ecl_smspec_num_nodes(m_ecl_SmSpec);
        for(int i = 0; i < varCount; i++)
        {
            const smspec_node_type * ertSumVarNode = ecl_smspec_iget_node(m_ecl_SmSpec, i);
            RifEclipseSummaryAddress addr = addressFromErtSmSpecNode(ertSumVarNode);
            m_allResultAddresses.insert(addr);
            m_resultAddressToErtNodeIdx[addr] = i;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RifReaderEclipseSummary::getRestartFile(const QString& headerFileName)
{
    ecl_sum_type* ecl_sum = openEclSum(headerFileName, true);

    const ecl_smspec_type* smspec = ecl_sum ? ecl_sum_get_smspec(ecl_sum) : nullptr;
    const char* rstCase = smspec ? ecl_smspec_get_restart_case(smspec) : nullptr;
    QString restartCase = rstCase? RiaFilePathTools::canonicalPath(RiaStringEncodingTools::fromNativeEncoded(rstCase)) : "";
    closeEclSum(ecl_sum);

    if (!restartCase.isEmpty())
    {
        QString path = QFileInfo(restartCase).dir().path();
        QString restartBase = QDir(restartCase).dirName();
        
        char* smspec_header = ecl_util_alloc_exfilename(path.toStdString().data(), restartBase.toStdString().data(), ECL_SUMMARY_HEADER_FILE, false /*unformatted*/, 0);
        QString restartFileName = RiaFilePathTools::toInternalSeparator(RiaStringEncodingTools::fromNativeEncoded(smspec_header));
        free(smspec_header);

        return getFileInfo(restartFileName);
    }
    return RifRestartFileInfo();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifReaderEclipseSummary::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    if (!m_ecl_SmSpec) return "";

    int variableIndex = indexFromAddress(resultAddress);

    if(variableIndex < 0) return "";

    const smspec_node_type * ertSumVarNode = ecl_smspec_iget_node(m_ecl_SmSpec, variableIndex);
    return smspec_node_get_unit(ertSumVarNode);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::markForCachePurge(const RifEclipseSummaryAddress& address)
{
    m_valuesCache->markAddressForPurge(address);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::purgeCache()
{
    ValuesCache::purge();
}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::populateVectorFromStringList(stringlist_type* stringList, std::vector<std::string>* strings)
{
    assert(stringList && strings);

    for (int i = 0; i < stringlist_get_size(stringList); i++)
    {
        strings->push_back(stringlist_iget(stringList, i));
    }
}

#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>                       RifReaderEclipseSummary::ValuesCache::EMPTY_VECTOR;
std::set<RifReaderEclipseSummary::ValuesCache*> RifReaderEclipseSummary::ValuesCache::m_instances;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::ValuesCache::ValuesCache()
{
    // Register instance
    m_instances.insert(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary::ValuesCache::~ValuesCache()
{
    // Deregister instance
    m_instances.erase(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::ValuesCache::insertValues(const RifEclipseSummaryAddress& address, const std::vector<double>& values)
{
    m_cachedValues[address] = values;
    m_purgeList.erase(address);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifReaderEclipseSummary::ValuesCache::getValues(const RifEclipseSummaryAddress& address) const
{
    if (m_cachedValues.find(address) != m_cachedValues.end())
    {
        return m_cachedValues.at(address);
    }
    return EMPTY_VECTOR;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::ValuesCache::markAddressForPurge(const RifEclipseSummaryAddress& address)
{
    m_purgeList.insert(address);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::ValuesCache::purge()
{
    for (auto instance : m_instances) instance->purgeData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseSummary::ValuesCache::purgeData()
{
    for (const auto purgeAddr : m_purgeList)
    {
        m_cachedValues.erase(purgeAddr);
    }
    m_purgeList.clear();
}

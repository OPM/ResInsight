/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RifReaderOpmParserInput.h"

#include "RifReaderEclipseOutput.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RimEclipseInputCaseOpm.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RiuMainWindow.h"
#include "RiuProcessMonitor.h"

#include "cafProgressInfo.h"
#include "cvfBase.h"

#include "opm/parser/eclipse/Deck/DeckItem.hpp"
#include "opm/parser/eclipse/Deck/Section.hpp"
#include "opm/parser/eclipse/Parser/MessageContainer.hpp"
#include "opm/parser/eclipse/Parser/ParseContext.hpp"
#include "opm/parser/eclipse/Parser/Parser.hpp"

#include "../generated-source/include/opm/parser/eclipse/Parser/ParserKeywords/F.hpp"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderOpmParserInput::importGridPropertiesFaults(const QString& fileName, RigCaseData* caseData)
{
    RiuMainWindow::instance()->processMonitor()->addStringToLog(QString("\nStarted reading of grid and properties from file : " + fileName + "\n"));

    {
        std::shared_ptr<const Opm::EclipseGrid> eclipseGrid;
        std::string errorMessage;

        std::shared_ptr<Opm::Deck> deck;

        try
        {
            Opm::Parser parser;

            // A default ParseContext will set up all parsing errors to throw exceptions
            Opm::ParseContext parseContext;
            RifReaderOpmParserInput::initUsingWarnings(&parseContext);

            deck = parser.parseFile(fileName.toStdString(), parseContext);

            if (deck)
            {
                eclipseGrid = Opm::Parser::parseGrid(*deck, parseContext);

                if (eclipseGrid && eclipseGrid->c_ptr())
                {
                    RifReaderEclipseOutput::transferGeometry(eclipseGrid->c_ptr(), caseData);
                }
                else
                {
                    throw std::invalid_argument("No valid 3D grid detected");
                }

                Opm::TableManager tableManager(*deck);

                Opm::Eclipse3DProperties properties(*deck, tableManager, *eclipseGrid);

                std::vector<std::string> predefKeywords = RifReaderOpmParserInput::knownPropertyKeywords();
                for (auto keyword : predefKeywords)
                {
                    if (properties.supportsGridProperty(keyword))
                    {
                        if (properties.hasDeckDoubleGridProperty(keyword))
                        {
                            auto allValues = properties.getDoubleGridProperty(keyword).getData();

                            QString newResultName = caseData->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(QString::fromStdString(keyword));
                            size_t resultIndex = RifReaderOpmParserPropertyReader::findOrCreateResult(newResultName, caseData);

                            if (resultIndex != cvf::UNDEFINED_SIZE_T)
                            {
                                std::vector< std::vector<double> >& newPropertyData = caseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
                                newPropertyData.push_back(allValues);
                            }
                        }
                        else if (properties.hasDeckIntGridProperty(keyword))
                        {
                            auto intValues = properties.getIntGridProperty(keyword).getData();

                            QString newResultName = caseData->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(QString::fromStdString(keyword));
                            size_t resultIndex = RifReaderOpmParserPropertyReader::findOrCreateResult(newResultName, caseData);

                            if (resultIndex != cvf::UNDEFINED_SIZE_T)
                            {
                                std::vector< std::vector<double> >& newPropertyData = caseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);

                                std::vector<double> doubleValues;

                                doubleValues.insert(std::end(doubleValues), std::begin(intValues), std::end(intValues));

                                newPropertyData.push_back(doubleValues);
                            }
                        }
                    }

                    if (caseData->results(RifReaderInterface::MATRIX_RESULTS)->resultCount() == 0)
                    {
                        // Eclipse3DProperties was not able to extract results. This is often the case when reading a GRDECL file directly
                        // Parse for known keywords by analyzing the present keywords in the deck

                        RifReaderOpmParserPropertyReader propertyReader(deck);

                        std::set<std::string> keywordsOnFile = propertyReader.keywords();
                        std::vector<std::string> predefKeywords = RifReaderOpmParserInput::knownPropertyKeywords();
                        for (auto keyword : predefKeywords)
                        {
                            if (std::find(keywordsOnFile.begin(), keywordsOnFile.end(), keyword) != keywordsOnFile.end())
                            {
                                QString newResultName = caseData->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(QString::fromStdString(keyword));
                                propertyReader.copyPropertyToCaseData(keyword, caseData, newResultName);
                            }
                        }
                    }

                    if (caseData->mainGrid())
                    {
                        cvf::Collection<RigFault> faults;
                        importFaults(*deck, &faults);
                        if (faults.size() > 0)
                        {
                            caseData->mainGrid()->setFaults(faults);
                        }
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            errorMessage = e.what();
        }
        catch (...)
        {
            errorMessage = "Unknown exception throwm from Opm::Parser";
        }

        if (deck)
        {
            const Opm::MessageContainer& messages = deck->getMessageContainer();
            if (messages.size() > 0)
            {
                RiuMainWindow::instance()->processMonitor()->addStringToLog("\n\nLog messages from Deck : \n");
            }
            for (auto m : messages)
            {
                RiuMainWindow::instance()->processMonitor()->addStringToLog("  Deck : " + QString::fromStdString(m.message) + "\n");
            }
        }

        if (eclipseGrid)
        {
            const Opm::MessageContainer& messages = eclipseGrid->getMessageContainer();
            if (messages.size() > 0)
            {
                RiuMainWindow::instance()->processMonitor()->addStringToLog("\n\nLog messages from EclipseGrid : \n");
            }
            for (auto m : messages)
            {
                RiuMainWindow::instance()->processMonitor()->addStringToLog("  EclipseGrid :" + QString::fromStdString(m.message) + "\n");
            }
        }

        if (errorMessage.size() > 0)
        {
            RiuMainWindow::instance()->processMonitor()->addStringToLog("\n\nError messages : \n");
            RiuMainWindow::instance()->processMonitor()->addStringToLog("  " + QString::fromStdString(errorMessage) + "\n");
            RiuMainWindow::instance()->processMonitor()->addStringToLog(QString("Failed reading of grid and properties from file : " + fileName + "\n"));
        }
        else
        {
            RiuMainWindow::instance()->processMonitor()->addStringToLog(QString("Completed reading of grid and properties from file : " + fileName + "\n"));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderOpmParserInput::readFaults(const QString& fileName, cvf::Collection<RigFault>* faults)
{
    {
        std::string errorMessage;

        try
        {
            Opm::Parser parser;

            // A default ParseContext will set up all parsing errors to throw exceptions
            Opm::ParseContext parseContext;
            RifReaderOpmParserInput::initUsingWarnings(&parseContext);

            auto deckptr = parser.parseFile(fileName.toStdString(), parseContext);
            const Opm::Deck& deck = *deckptr;

            importFaults(deck, faults);

        }
        catch (std::exception& e)
        {
            errorMessage = e.what();
        }
        catch (...)
        {
            errorMessage = "Unknown exception throwm from Opm::Parser";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderOpmParserInput::importFaults(const Opm::Deck& deck, cvf::Collection<RigFault>* faults)
{
    {
        std::string errorMessage;

        try
        {
            RigFault* fault = NULL;

            // The following is based on Opm::FaultCollection
            // Not possible to use this class, as the logic in ResInsight handles IJK-values instead
            // of cell indices
            const auto& faultKeywords = deck.getKeywordList<Opm::ParserKeywords::FAULTS>();
            for (auto keyword_iter = faultKeywords.begin(); keyword_iter != faultKeywords.end(); ++keyword_iter) {
                const auto& faultsKeyword = *keyword_iter;
                for (auto iter = faultsKeyword->begin(); iter != faultsKeyword->end(); ++iter) {
                    const auto& faultRecord = *iter;
                    const std::string& faultName = faultRecord.getItem(0).get< std::string >(0);
                    int I1 = faultRecord.getItem(1).get< int >(0) - 1;
                    int I2 = faultRecord.getItem(2).get< int >(0) - 1;
                    int J1 = faultRecord.getItem(3).get< int >(0) - 1;
                    int J2 = faultRecord.getItem(4).get< int >(0) - 1;
                    int K1 = faultRecord.getItem(5).get< int >(0) - 1;
                    int K2 = faultRecord.getItem(6).get< int >(0) - 1;

                    const std::string& faceText = faultRecord.getItem(7).get< std::string >(0);

                    cvf::StructGridInterface::FaceEnum cellFaceEnum = RifReaderOpmParserInput::faceEnumFromText(QString::fromStdString(faceText));

                    QString name = QString::fromStdString(faultName);

                    // Guard against invalid cell ranges by limiting lowest possible range value to zero
                    cvf::CellRange cellrange(CVF_MAX(I1, 0), CVF_MAX(J1, 0), CVF_MAX(K1, 0), CVF_MAX(I2, 0), CVF_MAX(J2, 0), CVF_MAX(K2, 0));

                    if (!(fault && fault->name() == name))
                    {
                        if (findFaultByName(*faults, name) == cvf::UNDEFINED_SIZE_T)
                        {
                            RigFault* newFault = new RigFault;
                            newFault->setName(name);

                            faults->push_back(newFault);
                        }

                        size_t faultIndex = findFaultByName(*faults, name);
                        if (faultIndex == cvf::UNDEFINED_SIZE_T)
                        {
                            CVF_ASSERT(faultIndex != cvf::UNDEFINED_SIZE_T);
                            continue;
                        }

                        fault = faults->at(faultIndex);
                    }

                    CVF_ASSERT(fault);

                    fault->addCellRangeForFace(cellFaceEnum, cellrange);
                }
            }
        }
        catch (std::exception& e)
        {
            errorMessage = e.what();
        }
        catch (...)
        {
            errorMessage = "Unknown exception throwm from Opm::Parser";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifReaderOpmParserInput::knownPropertyKeywords()
{
    std::vector<std::string> knownKeywords;
    knownKeywords.push_back("AQUIFERA");
    knownKeywords.push_back("ACTNUM");
    knownKeywords.push_back("EQLNUM");
    knownKeywords.push_back("FIPNUM");
    knownKeywords.push_back("KRG");
    knownKeywords.push_back("KRGR");
    knownKeywords.push_back("KRO");
    knownKeywords.push_back("KRORG");
    knownKeywords.push_back("KRORW");
    knownKeywords.push_back("KRW");
    knownKeywords.push_back("KRWR");
    knownKeywords.push_back("MINPVV");
    knownKeywords.push_back("MULTPV");
    knownKeywords.push_back("MULTX");
    knownKeywords.push_back("MULTX-");
    knownKeywords.push_back("MULTY");
    knownKeywords.push_back("MULTY-");
    knownKeywords.push_back("MULTZ");
    knownKeywords.push_back("NTG");
    knownKeywords.push_back("PCG");
    knownKeywords.push_back("PCW");
    knownKeywords.push_back("PERMX");
    knownKeywords.push_back("PERMY");
    knownKeywords.push_back("PERMZ");
    knownKeywords.push_back("PORO");
    knownKeywords.push_back("PVTNUM");
    knownKeywords.push_back("SATNUM");
    knownKeywords.push_back("SGCR");
    knownKeywords.push_back("SGL");
    knownKeywords.push_back("SGLPC");
    knownKeywords.push_back("SGU");
    knownKeywords.push_back("SGWCR");
    knownKeywords.push_back("SWATINIT");
    knownKeywords.push_back("SWCR");
    knownKeywords.push_back("SWGCR");
    knownKeywords.push_back("SWL");
    knownKeywords.push_back("SWLPC");
    knownKeywords.push_back("TRANX");
    knownKeywords.push_back("TRANY");
    knownKeywords.push_back("TRANZ");

    return knownKeywords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifReaderOpmParserInput::allParserConfigKeys()
{
    std::vector<std::string> configKeys;
    configKeys.push_back(Opm::ParseContext::PARSE_UNKNOWN_KEYWORD);
    configKeys.push_back(Opm::ParseContext::PARSE_RANDOM_TEXT);
    configKeys.push_back(Opm::ParseContext::PARSE_RANDOM_SLASH);
    configKeys.push_back(Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD);
    configKeys.push_back(Opm::ParseContext::PARSE_EXTRA_DATA);
    configKeys.push_back(Opm::ParseContext::PARSE_MISSING_INCLUDE);
    configKeys.push_back(Opm::ParseContext::UNSUPPORTED_SCHEDULE_GEO_MODIFIER);
    configKeys.push_back(Opm::ParseContext::UNSUPPORTED_COMPORD_TYPE);
    configKeys.push_back(Opm::ParseContext::UNSUPPORTED_INITIAL_THPRES);
    configKeys.push_back(Opm::ParseContext::INTERNAL_ERROR_UNINITIALIZED_THPRES);
    configKeys.push_back(Opm::ParseContext::PARSE_MISSING_SECTIONS);

    return configKeys;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifReaderOpmParserInput::findFaultByName(const cvf::Collection<RigFault>& faults, const QString& name)
{
    for (size_t i = 0; i < faults.size(); i++)
    {
        if (faults.at(i)->name() == name)
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::StructGridInterface::FaceEnum RifReaderOpmParserInput::faceEnumFromText(const QString& faceString)
{
    QString upperCaseText = faceString.toUpper().trimmed();

    if (upperCaseText == "X" || upperCaseText == "X+" || upperCaseText == "I" || upperCaseText == "I+") return cvf::StructGridInterface::POS_I;
    if (upperCaseText == "Y" || upperCaseText == "Y+" || upperCaseText == "J" || upperCaseText == "J+") return cvf::StructGridInterface::POS_J;
    if (upperCaseText == "Z" || upperCaseText == "Z+" || upperCaseText == "K" || upperCaseText == "K+") return cvf::StructGridInterface::POS_K;

    if (upperCaseText == "X-" || upperCaseText == "I-") return cvf::StructGridInterface::NEG_I;
    if (upperCaseText == "Y-" || upperCaseText == "J-") return cvf::StructGridInterface::NEG_J;
    if (upperCaseText == "Z-" || upperCaseText == "K-") return cvf::StructGridInterface::NEG_K;

    return cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderOpmParserInput::initUsingWarnings(Opm::ParseContext* parseContext)
{
    if (!parseContext) return;

    for (auto state : allParserConfigKeys())
    {
        parseContext->addKey(state);
    }

    // Treat all parsing errors as warnings
    parseContext->update(Opm::InputError::WARN);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifReaderOpmParserPropertyReader::findOrCreateResult(const QString& newResultName, RigCaseData* reservoir)
{
    size_t resultIndex = reservoir->results(RifReaderInterface::MATRIX_RESULTS)->findScalarResultIndex(newResultName);
    if (resultIndex == cvf::UNDEFINED_SIZE_T)
    {
        resultIndex = reservoir->results(RifReaderInterface::MATRIX_RESULTS)->addEmptyScalarResult(RimDefines::INPUT_PROPERTY, newResultName, false);
    }

    return resultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderOpmParserPropertyReader::readAllProperties(std::shared_ptr< Opm::Deck > deck, RigCaseData* caseData, std::map<QString, QString>* newResults)
{
    std::set<std::string> uniqueKeywords;
    for (auto it = deck->begin(); it != deck->end(); it++)
    {
        uniqueKeywords.insert(it->name());
    }

    for (auto keyword : uniqueKeywords)
    {
        bool isItemCountEqual = RifReaderOpmParserPropertyReader::isDataItemCountIdenticalToMainGridCellCount(deck, keyword, caseData);

        if (isItemCountEqual)
        {
            std::vector<double> allValues;

            RifReaderOpmParserPropertyReader::getAllValuesForKeyword(deck, keyword, allValues);

            QString keywordName = QString::fromStdString(keyword);
            QString newResultName = caseData->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(keywordName);
            size_t resultIndex = findOrCreateResult(newResultName, caseData);
            if (resultIndex != cvf::UNDEFINED_SIZE_T)
            {
                std::vector< std::vector<double> >& newPropertyData = caseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
                newPropertyData.push_back(allValues);
            }

            newResults->insert(std::make_pair(newResultName, keywordName));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderOpmParserPropertyReader::RifReaderOpmParserPropertyReader(std::shared_ptr< Opm::Deck > deck)
    : m_deck(deck)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderOpmParserPropertyReader::RifReaderOpmParserPropertyReader(const QString& fileName)
{
    open(fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmParserPropertyReader::open(const QString& fileName)
{
    {
        std::string errorMessage;

        try
        {
            Opm::Parser parser;

            // A default ParseContext will set up all parsing errors to throw exceptions
            Opm::ParseContext parseContext;

            // Treat all parsing errors as warnings
            parseContext.update(Opm::InputError::WARN);

            m_deck = parser.parseFile(fileName.toStdString(), parseContext);
        }
        catch (std::exception& e)
        {
            errorMessage = e.what();
        }
        catch (...)
        {
            errorMessage = "Unknown exception throwm from Opm::Parser";
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::string> RifReaderOpmParserPropertyReader::keywords() const
{
    std::set<std::string> ids;

    if (m_deck)
    {
        for (auto it = m_deck->begin(); it != m_deck->end(); it++)
        {
            ids.insert(it->name());
        }
    }

    return ids;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmParserPropertyReader::copyPropertyToCaseData(const std::string& keywordName, RigCaseData* caseData, const QString& resultName)
{
    {
        std::string errorMessage;

        try
        {
            if (m_deck->hasKeyword(keywordName))
            {
                bool isItemCountEqual = isDataItemCountIdenticalToMainGridCellCount(m_deck, keywordName, caseData);
                if (isItemCountEqual)
                {
                    std::vector<double> allValues;

                    getAllValuesForKeyword(m_deck, keywordName, allValues);

                    size_t resultIndex = RifReaderOpmParserPropertyReader::findOrCreateResult(resultName, caseData);
                    if (resultIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        std::vector< std::vector<double> >& newPropertyData = caseData->results(RifReaderInterface::MATRIX_RESULTS)->cellScalarResults(resultIndex);
                        newPropertyData.push_back(allValues);
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            errorMessage = e.what();
        }
        catch (...)
        {
            errorMessage = "Unknown exception throwm from Opm::Parser";
        }

        QString fileName = QString::fromStdString(m_deck->getDataFile());

        if (errorMessage.size() > 0)
        {
            RiuMainWindow::instance()->processMonitor()->addStringToLog("  " + QString::fromStdString(errorMessage) + "\n");
            RiuMainWindow::instance()->processMonitor()->addStringToLog(QString("Error detected while reading property %1 from file : %2\n").arg(QString::fromStdString(keywordName)).arg(fileName));
        }
        else
        {
            RiuMainWindow::instance()->processMonitor()->addStringToLog(QString("Completed reading of property %1 from file : %2\n").arg(QString::fromStdString(keywordName)).arg(fileName));
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderOpmParserPropertyReader::getAllValuesForKeyword(std::shared_ptr< Opm::Deck > deck, const std::string& keyword, std::vector<double>& allValues)
{
    for (auto deckKeyword : deck->getKeywordList(keyword))
    {
        if (deckKeyword->isDataKeyword() && deckKeyword->size() == 1)
        {
            auto deckRecord = deckKeyword->getDataRecord();
            if (deckRecord.size() == 1)
            {
                if (deckRecord.getDataItem().typeof() == Opm::DeckItem::integer)
                {
                    auto opmData = deckKeyword->getIntData();
                    allValues.insert(std::end(allValues), std::begin(opmData), std::end(opmData));
                }
                else if (deckRecord.getDataItem().typeof() == Opm::DeckItem::fdouble)
                {
                    auto opmData = deckKeyword->getRawDoubleData();
                    allValues.insert(std::end(allValues), std::begin(opmData), std::end(opmData));
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmParserPropertyReader::isDataItemCountIdenticalToMainGridCellCount(std::shared_ptr< Opm::Deck > deck, const std::string& keyword, RigCaseData* caseData)
{
    bool isEqual = false;
    {
        size_t valueCount = 0;
        for (auto deckKeyword : deck->getKeywordList(keyword))
        {
            if (deckKeyword->isDataKeyword())
            {
                valueCount += deckKeyword->getDataSize();
            }
        }

        if (valueCount == caseData->mainGrid()->cellCount())
        {
            isEqual = true;
        }
    }

    return isEqual;
}


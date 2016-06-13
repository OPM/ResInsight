/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/DeckTimeStep.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/SCHEDULESection.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/W.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicVector.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Events.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/GroupTree.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/Compsegs.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/SegmentSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/OilVaporizationProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Tuning.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellInjectionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellPolymerProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellProductionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellSet.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

namespace Opm {

    Schedule::Schedule(const ParseContext& parseContext,
                       std::shared_ptr<const EclipseGrid> grid,
                       DeckConstPtr deckptr,
                       IOConfigPtr ioConfig) :
            Schedule(parseContext, grid, *deckptr, ioConfig)
    {
    }

    Schedule::Schedule(const ParseContext& parseContext, std::shared_ptr<const EclipseGrid> grid, const Deck& deck,
            IOConfigPtr ioConfig) :
            m_grid(grid)
    {
        initFromDeck(parseContext, deck, ioConfig);
    }

    boost::posix_time::ptime Schedule::getStartTime() const {
        return m_timeMap->getStartTime(/*timeStepIdx=*/0);
    }

    void Schedule::initFromDeck(const ParseContext& parseContext, const Deck& deck, IOConfigPtr ioConfig) {
        initializeNOSIM(deck);
        createTimeMap(deck);
        m_tuning.reset(new Tuning(m_timeMap));
        m_events.reset(new Events(m_timeMap));
        m_modifierDeck.reset( new DynamicVector<std::shared_ptr<Deck> >( m_timeMap , std::shared_ptr<Deck>( 0 ) ));
        addGroup( "FIELD", 0 );
        initRootGroupTreeNode(getTimeMap());
        initOilVaporization(getTimeMap());

        if (Section::hasSCHEDULE(deck)) {
            std::shared_ptr<SCHEDULESection> scheduleSection = std::make_shared<SCHEDULESection>(deck);
            iterateScheduleSection(parseContext , *scheduleSection , ioConfig);
        }
    }

    void Schedule::initOilVaporization(TimeMapConstPtr timeMap) {
        m_oilvaporizationproperties.reset(new DynamicState<OilVaporizationPropertiesPtr>(timeMap, OilVaporizationPropertiesPtr()));
    }

    void Schedule::initRootGroupTreeNode(TimeMapConstPtr timeMap) {
        m_rootGroupTree.reset(new DynamicState<GroupTreePtr>(timeMap, GroupTreePtr(new GroupTree())));
    }

    void Schedule::initializeNOSIM(const Deck& deck) {
        if (deck.hasKeyword("NOSIM")){
            nosim = true;
        } else {
            nosim = false;
        }
    }

    void Schedule::createTimeMap(const Deck& deck) {
        boost::posix_time::ptime startTime(defaultStartDate);
        if (deck.hasKeyword("START")) {
             const auto& startKeyword = deck.getKeyword("START");
            startTime = TimeMap::timeFromEclipse(startKeyword.getRecord(0));
        }

        m_timeMap.reset(new TimeMap(startTime));
    }

    void Schedule::iterateScheduleSection(const ParseContext& parseContext , const SCHEDULESection& section, IOConfigPtr ioConfig) {
        /*
          geoModifiers is a list of geo modifiers which can be found in the schedule
          section. This is only partly supported, support is indicated by the bool
          value. The keywords which are supported will be assembled in a per-timestep
          'minideck', whereas ParseContext::UNSUPPORTED_SCHEDULE_GEO_MODIFIER will be
          consulted for the others.
        */

        const std::map<std::string,bool> geoModifiers = {{"MULTFLT"  , true},
                                                         {"MULTPV"   , false},
                                                         {"MULTX"    , false},
                                                         {"MULTX-"   , false},
                                                         {"MULTY"    , false},
                                                         {"MULTY-"   , false},
                                                         {"MULTZ"    , false},
                                                         {"MULTZ-"   , false},
                                                         {"MULTREGT" , false},
                                                         {"MULTR"    , false},
                                                         {"MULTR-"   , false},
                                                         {"MULTSIG"  , false},
                                                         {"MULTSIGV" , false},
                                                         {"MULTTHT"  , false},
                                                         {"MULTTHT-" , false}};

        size_t currentStep = 0;
        std::vector<std::pair< const DeckKeyword* , size_t> > rftProperties;
        std::vector<std::pair< const DeckKeyword* , size_t> > IOConfigSettings;

        for (size_t keywordIdx = 0; keywordIdx < section.size(); ++keywordIdx) {
            const auto& keyword = section.getKeyword(keywordIdx);

            if (keyword.name() == "DATES") {
                handleDATES(keyword);
                currentStep += keyword.size();
            }

            if (keyword.name() == "TSTEP") {
                handleTSTEP(keyword);
                currentStep += keyword.getRecord(0).getItem(0).size(); // This is a bit weird API.
            }

            if (keyword.name() == "WELSPECS") {
                handleWELSPECS(section, keyword, currentStep);
            }

            if (keyword.name() == "WCONHIST")
                handleWCONHIST(keyword, currentStep);

            if (keyword.name() == "WCONPROD")
                handleWCONPROD(keyword, currentStep);

            if (keyword.name() == "WCONINJE")
                handleWCONINJE(section, keyword, currentStep);

            if (keyword.name() == "WPOLYMER")
                handleWPOLYMER(keyword, currentStep);

            if (keyword.name() == "WSOLVENT")
                handleWSOLVENT(keyword, currentStep);

            if (keyword.name() == "WCONINJH")
                handleWCONINJH(section, keyword, currentStep);

            if (keyword.name() == "WGRUPCON")
                handleWGRUPCON(keyword, currentStep);

            if (keyword.name() == "COMPDAT")
                handleCOMPDAT(keyword, currentStep);

            if (keyword.name() == "WELSEGS")
                handleWELSEGS(keyword, currentStep);

            if (keyword.name() == "COMPSEGS")
                handleCOMPSEGS(keyword, currentStep);

            if (keyword.name() == "WELOPEN")
                handleWELOPEN(keyword, currentStep , section.hasKeyword("COMPLUMP"));

            if (keyword.name() == "WELTARG")
                handleWELTARG(section, keyword, currentStep);

            if (keyword.name() == "GRUPTREE")
                handleGRUPTREE(keyword, currentStep);

            if (keyword.name() == "GCONINJE")
                handleGCONINJE(section, keyword, currentStep);

            if (keyword.name() == "GCONPROD")
                handleGCONPROD(keyword, currentStep);

            if (keyword.name() == "GEFAC")
                handleGEFAC(keyword, currentStep);

            if (keyword.name() == "TUNING")
                handleTUNING(keyword, currentStep);

            if (keyword.name() == "NOSIM")
                handleNOSIM();

            if (keyword.name() == "RPTRST")
                IOConfigSettings.push_back( std::make_pair( &keyword , currentStep ));

            if (keyword.name() == "RPTSCHED")
                IOConfigSettings.push_back( std::make_pair( &keyword , currentStep ));

            if (keyword.name() == "WRFT")
                rftProperties.push_back( std::make_pair( &keyword , currentStep ));

            if (keyword.name() == "WRFTPLT")
                rftProperties.push_back( std::make_pair( &keyword , currentStep ));

            if (keyword.name() == "WPIMULT")
                handleWPIMULT(keyword, currentStep);

            if (keyword.name() == "COMPORD")
                handleCOMPORD(parseContext , keyword, currentStep);

            if (keyword.name() == "DRSDT")
                handleDRSDT(keyword, currentStep);

            if (keyword.name() == "DRVDT")
                handleDRVDT(keyword, currentStep);

            if (keyword.name() == "VAPPARS")
                handleVAPPARS(keyword, currentStep);


            if (geoModifiers.find( keyword.name() ) != geoModifiers.end()) {
                bool supported = geoModifiers.at( keyword.name() );
                if (supported) {
                    /*
                      If the deck stored at currentStep is a null pointer (i.e. evaluates
                      to false) we must first create a new deck and install that under
                      index currentstep; then we fetch the deck (newly created - or old)
                      from the container and add the keyword.
                    */
                    if (!m_modifierDeck->iget(currentStep))
                        m_modifierDeck->iset( currentStep , std::make_shared<Deck>( ));

                    m_modifierDeck->iget( currentStep )->addKeyword( keyword );
                    m_events->addEvent( ScheduleEvents::GEO_MODIFIER , currentStep);

                } else {
                    std::string msg = "OPM does not support grid property modifier " + keyword.name() + " in the Schedule section. Error at report: " + std::to_string( currentStep );
                    parseContext.handleError( ParseContext::UNSUPPORTED_SCHEDULE_GEO_MODIFIER , msg );
                }
            }
        }

        m_timeMap->initFirstTimestepsYears();
        m_timeMap->initFirstTimestepsMonths();

        for (auto rftPair = rftProperties.begin(); rftPair != rftProperties.end(); ++rftPair) {
            const DeckKeyword& keyword = *rftPair->first;
            size_t timeStep = rftPair->second;
            if (keyword.name() == "WRFT") {
                handleWRFT(keyword,  timeStep);
            }

            if (keyword.name() == "WRFTPLT"){
                handleWRFTPLT(keyword, timeStep);
            }
        }

        for (auto restartPair = IOConfigSettings.begin(); restartPair != IOConfigSettings.end(); ++restartPair) {
            const DeckKeyword& keyword = *restartPair->first;
            size_t timeStep = restartPair->second;
            if ((keyword.name() == "RPTRST") && (m_timeMap->size() > timeStep+1 )) {
                handleRPTRST(keyword, timeStep + 1, ioConfig);
              } else if ((keyword.name() == "RPTSCHED") && (m_timeMap->size() > timeStep+1 )){
                handleRPTSCHED(keyword, timeStep + 1, ioConfig);
            }
        }

        checkUnhandledKeywords(section);
    }

    void Schedule::checkUnhandledKeywords(const SCHEDULESection& /*section*/) const
    {
    }


    void Schedule::handleDATES( const DeckKeyword& keyword) {
        m_timeMap->addFromDATESKeyword(keyword);
    }

    void Schedule::handleTSTEP( const DeckKeyword& keyword) {
        m_timeMap->addFromTSTEPKeyword(keyword);
    }

    bool Schedule::handleGroupFromWELSPECS(const std::string& groupName, GroupTreePtr newTree) const {
        bool treeUpdated = false;
        if (!newTree->getNode(groupName)) {
            treeUpdated = true;
            newTree->updateTree(groupName);
        }
        return treeUpdated;
    }


    void Schedule::handleCOMPORD(const ParseContext& parseContext, const DeckKeyword& compordKeyword, size_t /* currentStep */) {
        for (const auto& record : compordKeyword) {
            const auto& methodItem = record.getItem<ParserKeywords::COMPORD::ORDER_TYPE>();
            if ((methodItem.get< std::string >(0) != "TRACK")  && (methodItem.get< std::string >(0) != "INPUT")) {
                std::string msg = "The COMPORD keyword only handles 'TRACK' or 'INPUT' order.";
                parseContext.handleError( ParseContext::UNSUPPORTED_COMPORD_TYPE , msg );
            }
        }
    }



    void Schedule::handleWELSPECS( const SCHEDULESection& section, const DeckKeyword& keyword, size_t currentStep) {
        bool needNewTree = false;
        GroupTreePtr newTree = m_rootGroupTree->get(currentStep)->deepCopy();

        for (size_t recordNr = 0; recordNr < keyword.size(); recordNr++) {
            const auto& record = keyword.getRecord(recordNr);
            const std::string& wellName = record.getItem("WELL").getTrimmedString(0);
            const std::string& groupName = record.getItem("GROUP").getTrimmedString(0);

            if (!hasGroup(groupName))
                addGroup(groupName , currentStep);

            if (!hasWell(wellName)) {
                WellCompletion::CompletionOrderEnum wellCompletionOrder = WellCompletion::TRACK;

                DeckTimeStepConstPtr deckTimeStep = section.getDeckTimeStep(currentStep);
                if (deckTimeStep->hasKeyword("COMPORD")) {
                     const auto& compord = deckTimeStep->getKeyword("COMPORD");

                    for (size_t compordRecordNr = 0; compordRecordNr < compord.size(); compordRecordNr++) {
                        const auto& compordRecord = compord.getRecord(compordRecordNr);

                        const std::string& wellNamePattern = compordRecord.getItem(0).getTrimmedString(0);
                        if (Well::wellNameInWellNamePattern(wellName, wellNamePattern)) {
                            const std::string& compordString = compordRecord.getItem(1).getTrimmedString(0);
                            wellCompletionOrder = WellCompletion::CompletionOrderEnumFromString(compordString);
                        }
                    }
                }
                addWell(wellName, record, currentStep, wellCompletionOrder);
            }

            WellConstPtr currentWell = getWell(wellName);
            checkWELSPECSConsistency(currentWell, keyword, recordNr);

            addWellToGroup( getGroup(groupName) , getWell(wellName) , currentStep);
            if (handleGroupFromWELSPECS(groupName, newTree))
                needNewTree = true;
        }

        if (needNewTree) {
            m_rootGroupTree->update(currentStep, newTree);
            m_events->addEvent( ScheduleEvents::GROUP_CHANGE , currentStep);
        }
    }

    void Schedule::handleVAPPARS( const DeckKeyword& keyword, size_t currentStep){
        for( const auto& record : keyword ) {
            double vap = record.getItem("OIL_VAP_PROPENSITY").get< double >(0);
            double density = record.getItem("OIL_DENSITY_PROPENSITY").get< double >(0);
            OilVaporizationPropertiesPtr vappars = OilVaporizationProperties::createOilVaporizationPropertiesVAPPARS(vap, density);
            setOilVaporizationProperties(vappars, currentStep);

        }
    }

    void Schedule::handleDRVDT( const DeckKeyword& keyword, size_t currentStep){
        for( const auto& record : keyword ) {
            double max = record.getItem("DRVDT_MAX").getSIDouble(0);
            OilVaporizationPropertiesPtr drvdt = OilVaporizationProperties::createOilVaporizationPropertiesDRVDT(max);
            setOilVaporizationProperties(drvdt, currentStep);

        }
    }


    void Schedule::handleDRSDT( const DeckKeyword& keyword, size_t currentStep){
        for( const auto& record : keyword ) {
            double max = record.getItem("DRSDT_MAX").getSIDouble(0);
            std::string option = record.getItem("Option").get< std::string >(0);
            OilVaporizationPropertiesPtr drsdt = OilVaporizationProperties::createOilVaporizationPropertiesDRSDT(max, option);
            setOilVaporizationProperties(drsdt, currentStep);
        }
    }



    void Schedule::checkWELSPECSConsistency(WellConstPtr well, const DeckKeyword& keyword, size_t recordIdx) {
        const auto& record = keyword.getRecord(recordIdx);
        if (well->getHeadI() != record.getItem("HEAD_I").get< int >(0) - 1) {
            std::string msg =
                "Unable process WELSPECS for well " + well->name() + ", HEAD_I deviates from existing value";
            m_messages.error(keyword.getFileName(), msg, keyword.getLineNumber());
            throw std::invalid_argument(msg);
        }
        if (well->getHeadJ() != record.getItem("HEAD_J").get< int >(0) - 1) {
            std::string msg =
                "Unable process WELSPECS for well " + well->name() + ", HEAD_J deviates from existing value";
            m_messages.error(keyword.getFileName(), msg, keyword.getLineNumber());
            throw std::invalid_argument(msg);
        }
    }

    void Schedule::handleWCONProducer( const DeckKeyword& keyword, size_t currentStep, bool isPredictionMode) {
        for( const auto& record : keyword ) {
            const std::string& wellNamePattern =
                record.getItem("WELL").getTrimmedString(0);

            const WellCommon::StatusEnum status =
                WellCommon::StatusFromString(record.getItem("STATUS").getTrimmedString(0));

            const std::vector<WellPtr> wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                WellProductionProperties properties;


                if (isPredictionMode) {
                    auto addGrupProductionControl = well->isAvailableForGroupControl(currentStep);
                    properties = WellProductionProperties::prediction( record, addGrupProductionControl );
 		} else {
                    const WellProductionProperties& prev_properties = well->getProductionProperties(currentStep);
                    double BHPLimit = prev_properties.BHPLimit;
                    properties = WellProductionProperties::history( BHPLimit , record);
                }

                if (status != WellCommon::SHUT) {
                    const std::string& cmodeString =
                        record.getItem("CMODE").getTrimmedString(0);

                    WellProducer::ControlModeEnum control =
                        WellProducer::ControlModeFromString(cmodeString);

                    if (properties.hasProductionControl(control)) {
                        properties.controlMode = control;
                    }
                    else {
                        std::string msg =
                            "Tried to set invalid control: " +
                            cmodeString + " for well: " + well->name();
                        m_messages.error(keyword.getFileName(), msg, keyword.getLineNumber());
                        throw std::invalid_argument(msg);
                    }
                }
                updateWellStatus( well , currentStep , status );
                if (well->setProductionProperties(currentStep, properties))
                    m_events->addEvent( ScheduleEvents::PRODUCTION_UPDATE , currentStep);
                
                if ( !well->getAllowCrossFlow() && !isPredictionMode && (properties.OilRate + properties.WaterRate + properties.GasRate) == 0 ) {

                    std::string msg =
                            "Well " + well->name() + " is a history matched well with zero rate where crossflow is banned. " +
                            "This well will be closed at " + std::to_string ( m_timeMap->getTimePassedUntil(currentStep) / (60*60*24) ) + " days";
                    m_messages.info(msg);
                    updateWellStatus(well, currentStep, WellCommon::StatusEnum::SHUT );
                }
            }
        }
    }

    void Schedule::updateWellStatus(std::shared_ptr<Well> well, size_t reportStep , WellCommon::StatusEnum status) {
        if (well->setStatus( reportStep , status ))
            m_events->addEvent( ScheduleEvents::WELL_STATUS_CHANGE , reportStep );
    }


    void Schedule::handleWCONHIST(const DeckKeyword& keyword, size_t currentStep) {
        handleWCONProducer(keyword, currentStep, false);
    }

    void Schedule::handleWCONPROD( const DeckKeyword& keyword, size_t currentStep) {
        handleWCONProducer(keyword, currentStep, true);
    }

    static Opm::Value<int> getValueItem( const DeckItem& item ){
        Opm::Value<int> data(item.name());
        if(item.hasValue(0)) {
            int tempValue = item.get< int >(0);
            if( tempValue >0){
                data.setValue(tempValue-1);
            }
        }
        return data;
    }


    void Schedule::handleWPIMULT( const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            double wellPi = record.getItem("WELLPI").get< double >(0);
            std::vector<WellPtr> wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                CompletionSetConstPtr currentCompletionSet = well->getCompletions(currentStep);

                CompletionSetPtr newCompletionSet(new CompletionSet( ));

                Opm::Value<int> I  = getValueItem(record.getItem("I"));
                Opm::Value<int> J  = getValueItem(record.getItem("J"));
                Opm::Value<int> K  = getValueItem(record.getItem("K"));
                Opm::Value<int> FIRST = getValueItem(record.getItem("FIRST"));
                Opm::Value<int> LAST = getValueItem(record.getItem("LAST"));

                size_t completionSize = currentCompletionSet->size();

                for(size_t i = 0; i < completionSize;i++) {

                    CompletionConstPtr currentCompletion = currentCompletionSet->get(i);

                    if (FIRST.hasValue()) {
                        if (i < (size_t) FIRST.getValue()) {
                            newCompletionSet->add(currentCompletion);
                            continue;
                        }
                    }
                    if (LAST.hasValue()) {
                        if (i > (size_t) LAST.getValue()) {
                            newCompletionSet->add(currentCompletion);
                            continue;
                        }
                    }

                    int ci = currentCompletion->getI();
                    int cj = currentCompletion->getJ();
                    int ck = currentCompletion->getK();

                    if (I.hasValue() && (!(I.getValue() == ci) )) {
                        newCompletionSet->add(currentCompletion);
                        continue;
                    }

                    if (J.hasValue() && (!(J.getValue() == cj) )) {
                        newCompletionSet->add(currentCompletion);
                        continue;
                    }

                    if (K.hasValue() && (!(K.getValue() == ck) )) {
                        newCompletionSet->add(currentCompletion);
                        continue;
                    }

                    CompletionPtr newCompletion = std::make_shared<Completion>(currentCompletion, wellPi);
                    newCompletionSet->add(newCompletion);
                }
                well->addCompletionSet(currentStep, newCompletionSet);



            }
        }
    }


    void Schedule::handleWCONINJE( const SCHEDULESection& section, const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            std::vector<WellPtr> wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                WellInjector::TypeEnum injectorType = WellInjector::TypeFromString( record.getItem("TYPE").getTrimmedString(0) );
                WellCommon::StatusEnum status = WellCommon::StatusFromString( record.getItem("STATUS").getTrimmedString(0));

                updateWellStatus( well , currentStep , status );
                WellInjectionProperties properties(well->getInjectionPropertiesCopy(currentStep));

                properties.injectorType = injectorType;
                properties.predictionMode = true;

                if (!record.getItem("RATE").defaultApplied(0)) {
                    properties.surfaceInjectionRate = convertInjectionRateToSI(record.getItem("RATE").get< double >(0) , injectorType, section.getActiveUnitSystem());
                    properties.addInjectionControl(WellInjector::RATE);
                } else
                    properties.dropInjectionControl(WellInjector::RATE);


                if (!record.getItem("RESV").defaultApplied(0)) {
                    properties.reservoirInjectionRate = record.getItem("RESV").getSIDouble(0);
                    properties.addInjectionControl(WellInjector::RESV);
                } else
                    properties.dropInjectionControl(WellInjector::RESV);


                if (!record.getItem("THP").defaultApplied(0)) {
                    properties.THPLimit       = record.getItem("THP").getSIDouble(0);
                    properties.VFPTableNumber = record.getItem("VFP_TABLE").get< int >(0);
                    properties.addInjectionControl(WellInjector::THP);
                } else
                    properties.dropInjectionControl(WellInjector::THP);

                /*
                  What a mess; there is a sensible default BHP limit
                  defined, so the BHPLimit can be safely set
                  unconditionally - but should we make BHP control
                  available based on that default value - currently we
                  do not do that.
                */
                properties.BHPLimit = record.getItem("BHP").getSIDouble(0);
                if (!record.getItem("BHP").defaultApplied(0)) {
                    properties.addInjectionControl(WellInjector::BHP);
                } else
                    properties.dropInjectionControl(WellInjector::BHP);

                if (well->isAvailableForGroupControl(currentStep))
                    properties.addInjectionControl(WellInjector::GRUP);
                else
                    properties.dropInjectionControl(WellInjector::GRUP);
                {
                    const std::string& cmodeString = record.getItem("CMODE").getTrimmedString(0);
                    WellInjector::ControlModeEnum controlMode = WellInjector::ControlModeFromString( cmodeString );
                    if (properties.hasInjectionControl( controlMode))
                        properties.controlMode = controlMode;
                    else {
                        throw std::invalid_argument("Tried to set invalid control: " + cmodeString + " for well: " + wellNamePattern);
                    }
                }

                if (well->setInjectionProperties(currentStep, properties))
                    m_events->addEvent( ScheduleEvents::INJECTION_UPDATE , currentStep );

                if ( ! well->getAllowCrossFlow() && (properties.surfaceInjectionRate == 0) ) {
                    std::string msg =
                            "Well " + well->name() + " is an injector with zero rate where crossflow is banned. " +
                            "This well will be closed at " + std::to_string ( m_timeMap->getTimePassedUntil(currentStep) / (60*60*24) ) + " days";
                    m_messages.info(msg);
                    updateWellStatus(well, currentStep, WellCommon::StatusEnum::SHUT );
                }
            }
        }
    }


    void Schedule::handleWPOLYMER( const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            std::vector<WellPtr> wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;

                WellPolymerProperties properties(well->getPolymerPropertiesCopy(currentStep));

                properties.m_polymerConcentration = record.getItem("POLYMER_CONCENTRATION").getSIDouble(0);
                properties.m_saltConcentration = record.getItem("SALT_CONCENTRATION").getSIDouble(0);

                const auto& group_polymer_item = record.getItem("GROUP_POLYMER_CONCENTRATION");
                const auto& group_salt_item = record.getItem("GROUP_SALT_CONCENTRATION");

                if (!group_polymer_item.defaultApplied(0)) {
                    throw std::logic_error("Sorry explicit setting of \'GROUP_POLYMER_CONCENTRATION\' is not supported!");
                }

                if (!group_salt_item.defaultApplied(0)) {
                    throw std::logic_error("Sorry explicit setting of \'GROUP_SALT_CONCENTRATION\' is not supported!");
                }
                well->setPolymerProperties(currentStep, properties);
            }
        }
    }

    void Schedule::handleWSOLVENT( const DeckKeyword& keyword, size_t currentStep) {

        for( const auto& record : keyword ) {
            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            std::vector<WellPtr> wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                WellInjectionProperties injectionProperties = well->getInjectionProperties( currentStep );
                if (well->isInjector( currentStep ) && injectionProperties.injectorType == WellInjector::GAS) {
                    double fraction = record.getItem("SOLVENT_FRACTION").get< double >(0);
                    well->setSolventFraction(currentStep, fraction);
                } else {
                    throw std::invalid_argument("WSOLVENT keyword can only be applied to Gas injectors");
                }
            }
        }
    }

    void Schedule::handleWCONINJH( const SCHEDULESection& section,  const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& wellName = record.getItem("WELL").getTrimmedString(0);
            WellPtr well = getWell(wellName);

            // convert injection rates to SI
            WellInjector::TypeEnum injectorType = WellInjector::TypeFromString( record.getItem("TYPE").getTrimmedString(0));
            double injectionRate = record.getItem("RATE").get< double >(0);
            injectionRate = convertInjectionRateToSI(injectionRate, injectorType, section.getActiveUnitSystem());

            WellCommon::StatusEnum status = WellCommon::StatusFromString( record.getItem("STATUS").getTrimmedString(0));

            updateWellStatus(well ,  currentStep , status );
            WellInjectionProperties properties(well->getInjectionPropertiesCopy(currentStep));

            properties.injectorType = injectorType;

            const std::string& cmodeString = record.getItem("CMODE").getTrimmedString(0);
            WellInjector::ControlModeEnum controlMode = WellInjector::ControlModeFromString( cmodeString );
            if (!record.getItem("RATE").defaultApplied(0)) {
                properties.surfaceInjectionRate = injectionRate;
                properties.addInjectionControl(controlMode);
                properties.controlMode = controlMode;
            }
            properties.predictionMode = false;

            if (well->setInjectionProperties(currentStep, properties))
                m_events->addEvent( ScheduleEvents::INJECTION_UPDATE , currentStep );

            if ( ! well->getAllowCrossFlow() && (injectionRate == 0) ) {
                std::string msg =
                        "Well " + well->name() + " is an injector with zero rate where crossflow is banned. " +
                        "This well will be closed at " + std::to_string ( m_timeMap->getTimePassedUntil(currentStep) / (60*60*24) ) + " days";
                m_messages.info(msg);
                updateWellStatus(well, currentStep, WellCommon::StatusEnum::SHUT );
            }
        }
    }



    void Schedule::handleWELOPEN( const DeckKeyword& keyword, size_t currentStep , bool hascomplump) {

        for( const auto& record : keyword ) {
            bool haveCompletionData = false;
            for (size_t i=2; i<7; i++) {
                const auto& item = record.getItem(i);
                if (!item.defaultApplied(0)) {
                    haveCompletionData = true;
                    break;
                }
            }

            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            const std::vector<WellPtr>& wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;

                if(haveCompletionData){
                    CompletionSetConstPtr currentCompletionSet = well->getCompletions(currentStep);

                    CompletionSetPtr newCompletionSet(new CompletionSet( ));

                    Opm::Value<int> I  = getValueItem(record.getItem("I"));
                    Opm::Value<int> J  = getValueItem(record.getItem("J"));
                    Opm::Value<int> K  = getValueItem(record.getItem("K"));
                    Opm::Value<int> C1 = getValueItem(record.getItem("C1"));
                    Opm::Value<int> C2 = getValueItem(record.getItem("C2"));

                    if(hascomplump && (C2.hasValue() || C1.hasValue())){
                        std::cerr << "ERROR the keyword COMPLUMP is not supported used when C1 or C2 in WELOPEN have values" << std::endl;
                        throw std::exception();
                    }

                    size_t completionSize = currentCompletionSet->size();

                    for(size_t i = 0; i < completionSize;i++) {

                        CompletionConstPtr currentCompletion = currentCompletionSet->get(i);

                        if (C1.hasValue()) {
                            if (i < (size_t) C1.getValue()) {
                                newCompletionSet->add(currentCompletion);
                                continue;
                            }
                        }
                        if (C2.hasValue()) {
                            if (i > (size_t) C2.getValue()) {
                                newCompletionSet->add(currentCompletion);
                                continue;
                            }
                        }

                        int ci = currentCompletion->getI();
                        int cj = currentCompletion->getJ();
                        int ck = currentCompletion->getK();

                        if (I.hasValue() && (!(I.getValue() == ci) )) {
                            newCompletionSet->add(currentCompletion);
                            continue;
                        }

                        if (J.hasValue() && (!(J.getValue() == cj) )) {
                            newCompletionSet->add(currentCompletion);
                            continue;
                        }

                        if (K.hasValue() && (!(K.getValue() == ck) )) {
                            newCompletionSet->add(currentCompletion);
                            continue;
                        }
                        WellCompletion::StateEnum completionStatus = WellCompletion::StateEnumFromString(record.getItem("STATUS").getTrimmedString(0));
                        CompletionPtr newCompletion = std::make_shared<Completion>(currentCompletion, completionStatus);
                        newCompletionSet->add(newCompletion);
                    }

                    well->addCompletionSet(currentStep, newCompletionSet);
                    m_events->addEvent(ScheduleEvents::COMPLETION_CHANGE, currentStep);
                    if (newCompletionSet->allCompletionsShut())
                        updateWellStatus( well , currentStep , WellCommon::StatusEnum::SHUT);

                }
                else if(!haveCompletionData) {
                    WellCommon::StatusEnum status = WellCommon::StatusFromString( record.getItem("STATUS").getTrimmedString(0));
                    if (status == WellCommon::StatusEnum::OPEN && !well->canOpen(currentStep)) {
                        std::string msg =
                                "Well " + well->name() + " where crossflow is banned has zero total rate. " +
                                "This well is prevented from opening at " + std::to_string ( m_timeMap->getTimePassedUntil(currentStep) / (60*60*24) ) + " days";
                        m_messages.info(msg);
                        continue;
                    }
                    updateWellStatus( well , currentStep , status );
                }
            }
        }
    }

    /*
      The documentation for the WELTARG keyword says that the well
      must have been fully specified and initialized using one of the
      WCONxxxx keywords prior to modifying the well using the WELTARG
      keyword.

      The following implementation of handling the WELTARG keyword
      does not check or enforce in any way that this is done (i.e. it
      is not checked or verified that the well is initialized with any
      WCONxxxx keyword).
    */

    void Schedule::handleWELTARG( const SCHEDULESection& section ,  const DeckKeyword& keyword, size_t currentStep) {
        Opm::UnitSystem unitSystem = section.getActiveUnitSystem();
        double siFactorL = unitSystem.parse("LiquidSurfaceVolume/Time")->getSIScaling();
        double siFactorG = unitSystem.parse("GasSurfaceVolume/Time")->getSIScaling();
        double siFactorP = unitSystem.parse("Pressure")->getSIScaling();

        for( const auto& record : keyword ) {

            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            const std::string& cMode = record.getItem("CMODE").getTrimmedString(0);
            double newValue = record.getItem("NEW_VALUE").get< double >(0);
            const std::vector<WellPtr>& wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                if(well->isProducer(currentStep)){
                    WellProductionProperties prop = well->getProductionPropertiesCopy(currentStep);

                    if (cMode == "ORAT"){
                        prop.OilRate = newValue * siFactorL;
                    }
                    else if (cMode == "WRAT"){
                        prop.WaterRate = newValue * siFactorL;
                    }
                    else if (cMode == "GRAT"){
                        prop.GasRate = newValue * siFactorG;
                    }
                    else if (cMode == "LRAT"){
                        prop.LiquidRate = newValue * siFactorL;
                    }
                    else if (cMode == "RESV"){
                        prop.ResVRate = newValue * siFactorL;
                    }
                    else if (cMode == "BHP"){
                        prop.BHPLimit = newValue * siFactorP;
                        /* For wells controlled by WCONHIST the BHP value given by the
                           WCHONHIST keyword can not be used to control the well - i.e BHP
                           control is not natively available - however when BHP has been
                           specified with WELTARG we can enable BHP control.
                        */
                        if (prop.predictionMode == false)
                            prop.addProductionControl(WellProducer::BHP);
                    }
                    else if (cMode == "THP"){
                        prop.THPLimit = newValue * siFactorP;
                    }
                    else if (cMode == "VFP"){
                        prop.VFPTableNumber = static_cast<int> (newValue);
                    }
                    else if (cMode == "GUID"){
                        well->setGuideRate(currentStep, newValue);
                    }
                    else{
                        throw std::invalid_argument("Invalid keyword (MODE) supplied");
                    }

                    well->setProductionProperties(currentStep, prop);
                }else{
                    WellInjectionProperties prop = well->getInjectionPropertiesCopy(currentStep);
                    if (cMode == "BHP"){
                        prop.BHPLimit = newValue * siFactorP;
                        /* For wells controlled by WCONINJH the BHP value given by the
                           WCHONINJH keyword can not be used to control the well - i.e BHP
                           control is not natively available - however when BHP has been
                           specified with WELTARG we can enable BHP control.
                        */
                        if (prop.predictionMode == false)
                            prop.addInjectionControl(WellInjector::BHP);
                    }
                    else if (cMode == "ORAT"){
                        if(prop.injectorType == WellInjector::TypeEnum::OIL){
                            prop.surfaceInjectionRate = newValue * siFactorL;
                        }else{
                             std::invalid_argument("Well type must be OIL to set the oil rate");
                        }
                    }
                    else if (cMode == "WRAT"){
                        if(prop.injectorType == WellInjector::TypeEnum::WATER){
                            prop.surfaceInjectionRate = newValue * siFactorL;
                        }else{
                             std::invalid_argument("Well type must be WATER to set the water rate");
                        }
                    }
                    else if (cMode == "GRAT"){
                        if(prop.injectorType == WellInjector::TypeEnum::GAS){
                            prop.surfaceInjectionRate = newValue * siFactorG;
                        }else{
                            std::invalid_argument("Well type must be GAS to set the gas rate");
                        }
                    }
                    else if (cMode == "THP"){
                        prop.THPLimit = newValue * siFactorP;
                    }
                    else if (cMode == "VFP"){
                        prop.VFPTableNumber = static_cast<int> (newValue);
                    }
                    else if (cMode == "GUID"){
                        well->setGuideRate(currentStep, newValue);
                    }
                    else if (cMode == "RESV"){
                        prop.reservoirInjectionRate = newValue * siFactorL;
                    }
                    else{
                        throw std::invalid_argument("Invalid keyword (MODE) supplied");
                    }

                    well->setInjectionProperties(currentStep, prop);
                }


            }
        }
    }

    void Schedule::handleGCONINJE( const SCHEDULESection& section,  const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& groupName = record.getItem("GROUP").getTrimmedString(0);
            GroupPtr group = getGroup(groupName);

            {
                Phase::PhaseEnum phase = Phase::PhaseEnumFromString( record.getItem("PHASE").getTrimmedString(0) );
                group->setInjectionPhase( currentStep , phase );
            }
            {
                GroupInjection::ControlEnum controlMode = GroupInjection::ControlEnumFromString( record.getItem("CONTROL_MODE").getTrimmedString(0) );
                group->setInjectionControlMode( currentStep , controlMode );
            }

            Phase::PhaseEnum wellPhase = Phase::PhaseEnumFromString( record.getItem("PHASE").getTrimmedString(0));

            // calculate SI injection rates for the group
            double surfaceInjectionRate = record.getItem("SURFACE_TARGET").get< double >(0);
            surfaceInjectionRate = convertInjectionRateToSI(surfaceInjectionRate, wellPhase, section.getActiveUnitSystem());
            double reservoirInjectionRate = record.getItem("RESV_TARGET").getSIDouble(0);

            group->setSurfaceMaxRate( currentStep , surfaceInjectionRate);
            group->setReservoirMaxRate( currentStep , reservoirInjectionRate);
            group->setTargetReinjectFraction( currentStep , record.getItem("REINJ_TARGET").getSIDouble(0));
            group->setTargetVoidReplacementFraction( currentStep , record.getItem("VOIDAGE_TARGET").getSIDouble(0));

            group->setProductionGroup(currentStep, false);
        }
    }

    void Schedule::handleGCONPROD( const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& groupName = record.getItem("GROUP").getTrimmedString(0);
            GroupPtr group = getGroup(groupName);
            {
                GroupProduction::ControlEnum controlMode = GroupProduction::ControlEnumFromString( record.getItem("CONTROL_MODE").getTrimmedString(0) );
                group->setProductionControlMode( currentStep , controlMode );
            }
            group->setOilTargetRate( currentStep , record.getItem("OIL_TARGET").getSIDouble(0));
            group->setGasTargetRate( currentStep , record.getItem("GAS_TARGET").getSIDouble(0));
            group->setWaterTargetRate( currentStep , record.getItem("WATER_TARGET").getSIDouble(0));
            group->setLiquidTargetRate( currentStep , record.getItem("LIQUID_TARGET").getSIDouble(0));
            group->setReservoirVolumeTargetRate( currentStep , record.getItem("RESERVOIR_FLUID_TARGET").getSIDouble(0));
            {
                GroupProductionExceedLimit::ActionEnum exceedAction = GroupProductionExceedLimit::ActionEnumFromString(record.getItem("EXCEED_PROC").getTrimmedString(0) );
                group->setProductionExceedLimitAction( currentStep , exceedAction );
            }

            group->setProductionGroup(currentStep, true);
        }
    }


    void Schedule::handleGEFAC( const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& groupName = record.getItem("GROUP").getTrimmedString(0);
            GroupPtr group = getGroup(groupName);

            group->setGroupEfficiencyFactor(currentStep, record.getItem("EFFICIENCY_FACTOR").get< double >(0));

            const std::string& transfer_str = record.getItem("TRANSFER_EXT_NET").getTrimmedString(0);
            bool transfer = (transfer_str == "YES") ? true : false;
            group->setTransferGroupEfficiencyFactor(currentStep, transfer);
        }
    }


    void Schedule::handleTUNING( const DeckKeyword& keyword, size_t currentStep) {

        int numrecords = keyword.size();

        if (numrecords > 0) {
            const auto& record1 = keyword.getRecord(0);

            double TSINIT = record1.getItem("TSINIT").getSIDouble(0);
            m_tuning->setTSINIT(currentStep, TSINIT);

            double TSMAXZ = record1.getItem("TSMAXZ").getSIDouble(0);
            m_tuning->setTSMAXZ(currentStep, TSMAXZ);

            double TSMINZ = record1.getItem("TSMINZ").getSIDouble(0);
            m_tuning->setTSMINZ(currentStep, TSMINZ);

            double TSMCHP = record1.getItem("TSMCHP").getSIDouble(0);
            m_tuning->setTSMCHP(currentStep, TSMCHP);

            double TSFMAX = record1.getItem("TSFMAX").get< double >(0);
            m_tuning->setTSFMAX(currentStep, TSFMAX);

            double TSFMIN = record1.getItem("TSFMIN").get< double >(0);
            m_tuning->setTSFMIN(currentStep, TSFMIN);

            double TSFCNV = record1.getItem("TSFCNV").get< double >(0);
            m_tuning->setTSFCNV(currentStep, TSFCNV);

            double TFDIFF = record1.getItem("TFDIFF").get< double >(0);
            m_tuning->setTFDIFF(currentStep, TFDIFF);

            double THRUPT = record1.getItem("THRUPT").get< double >(0);
            m_tuning->setTHRUPT(currentStep, THRUPT);

            const auto& TMAXWCdeckItem = record1.getItem("TMAXWC");
            if (TMAXWCdeckItem.hasValue(0)) {
                double TMAXWC = TMAXWCdeckItem.getSIDouble(0);
                m_tuning->setTMAXWC(currentStep, TMAXWC);
            }
        }


        if (numrecords > 1) {
            const auto& record2 = keyword.getRecord(1);

            double TRGTTE = record2.getItem("TRGTTE").get< double >(0);
            m_tuning->setTRGTTE(currentStep, TRGTTE);

            double TRGCNV = record2.getItem("TRGCNV").get< double >(0);
            m_tuning->setTRGCNV(currentStep, TRGCNV);

            double TRGMBE = record2.getItem("TRGMBE").get< double >(0);
            m_tuning->setTRGMBE(currentStep, TRGMBE);

            double TRGLCV = record2.getItem("TRGLCV").get< double >(0);
            m_tuning->setTRGLCV(currentStep, TRGLCV);

            double XXXTTE = record2.getItem("XXXTTE").get< double >(0);
            m_tuning->setXXXTTE(currentStep, XXXTTE);

            double XXXCNV = record2.getItem("XXXCNV").get< double >(0);
            m_tuning->setXXXCNV(currentStep, XXXCNV);

            double XXXMBE = record2.getItem("XXXMBE").get< double >(0);
            m_tuning->setXXXMBE(currentStep, XXXMBE);

            double XXXLCV = record2.getItem("XXXLCV").get< double >(0);
            m_tuning->setXXXLCV(currentStep, XXXLCV);

            double XXXWFL = record2.getItem("XXXWFL").get< double >(0);
            m_tuning->setXXXWFL(currentStep, XXXWFL);

            double TRGFIP = record2.getItem("TRGFIP").get< double >(0);
            m_tuning->setTRGFIP(currentStep, TRGFIP);

            const auto& TRGSFTdeckItem = record2.getItem("TRGSFT");
            if (TRGSFTdeckItem.hasValue(0)) {
                double TRGSFT = TRGSFTdeckItem.get< double >(0);
                m_tuning->setTRGSFT(currentStep, TRGSFT);
            }

            double THIONX = record2.getItem("THIONX").get< double >(0);
            m_tuning->setTHIONX(currentStep, THIONX);

            int TRWGHT = record2.getItem("TRWGHT").get< int >(0);
            m_tuning->setTRWGHT(currentStep, TRWGHT);
        }


        if (numrecords > 2) {
            const auto& record3 = keyword.getRecord(2);

            int NEWTMX = record3.getItem("NEWTMX").get< int >(0);
            m_tuning->setNEWTMX(currentStep, NEWTMX);

            int NEWTMN = record3.getItem("NEWTMN").get< int >(0);
            m_tuning->setNEWTMN(currentStep, NEWTMN);

            int LITMAX = record3.getItem("LITMAX").get< int >(0);
            m_tuning->setLITMAX(currentStep, LITMAX);

            int LITMIN = record3.getItem("LITMIN").get< int >(0);
            m_tuning->setLITMIN(currentStep, LITMIN);

            int MXWSIT = record3.getItem("MXWSIT").get< int >(0);
            m_tuning->setMXWSIT(currentStep, MXWSIT);

            int MXWPIT = record3.getItem("MXWPIT").get< int >(0);
            m_tuning->setMXWPIT(currentStep, MXWPIT);

            double DDPLIM = record3.getItem("DDPLIM").getSIDouble(0);
            m_tuning->setDDPLIM(currentStep, DDPLIM);

            double DDSLIM = record3.getItem("DDSLIM").get< double >(0);
            m_tuning->setDDSLIM(currentStep, DDSLIM);

            double TRGDPR = record3.getItem("TRGDPR").getSIDouble(0);
            m_tuning->setTRGDPR(currentStep, TRGDPR);

            const auto& XXXDPRdeckItem = record3.getItem("XXXDPR");
            if (XXXDPRdeckItem.hasValue(0)) {
                double XXXDPR = XXXDPRdeckItem.getSIDouble(0);
                m_tuning->setXXXDPR(currentStep, XXXDPR);
            }
        }
    }


    void Schedule::handleNOSIM() {
        nosim = true;
    }

    void Schedule::handleRPTRST( const DeckKeyword& keyword, size_t currentStep, IOConfigPtr ioConfig) {
        const auto& record = keyword.getRecord(0);

        size_t basic = 1;
        size_t freq  = 0;
        size_t found_basic = 0;
        bool handle_RPTRST_BASIC = false;

        const auto& item = record.getItem(0);

        for (size_t index = 0; index < item.size(); ++index) {
            const std::string& mnemonic = item.get< std::string >(index);

            found_basic = mnemonic.find("BASIC=");
            if (found_basic != std::string::npos) {
                std::string basic_no = mnemonic.substr(found_basic+6, mnemonic.size());
                basic = boost::lexical_cast<size_t>(basic_no);
                handle_RPTRST_BASIC = true;
            }

            size_t found_freq = mnemonic.find("FREQ=");
            if (found_freq != std::string::npos) {
                std::string freq_no = mnemonic.substr(found_freq+5, mnemonic.size());
                freq = boost::lexical_cast<size_t>(freq_no);
            }
        }


        /* If no BASIC mnemonic is found, either it is not present or we might
           have an old data set containing integer controls instead of mnemonics.
           BASIC integer switch is integer control nr 1, FREQUENCY is integer
           control nr 6 */


        if (found_basic == std::string::npos) {
            if (item.size() >= 1)  {
                const std::string& integer_control_basic = item.get< std::string >(0);
                try {
                    basic = boost::lexical_cast<size_t>(integer_control_basic);
                    if (0 != basic ) // Peculiar special case in eclipse, - not documented
                                     // This ignore of basic = 0 for the integer mnemonics case
                                     // is done to make flow write restart file at the same intervals
                                     // as eclipse for the Norne data set. There might be some rules
                                     // we are missing here.
                    {
                        handle_RPTRST_BASIC = true;
                    }
                } catch (boost::bad_lexical_cast &) {
                    //do nothing
                }
            }

            if (item.size() >= 6) { //if frequency is set
                const std::string& integer_control_frequency = item.get< std::string >(5);
                try {
                    freq = boost::lexical_cast<size_t>(integer_control_frequency);
                } catch (boost::bad_lexical_cast &) {
                    //do nothing
                }
            }
        }

        if (handle_RPTRST_BASIC) {
            ioConfig->handleRPTRSTBasic(m_timeMap, currentStep, basic, freq);
        }
    }


    void Schedule::handleRPTSCHED( const DeckKeyword& keyword, size_t step, IOConfigPtr ioConfig) {
        const auto& record = keyword.getRecord(0);

        size_t restart = 0;
        size_t found_mnemonic_RESTART = 0;
        size_t found_mnemonic_NOTHING = 0;
        const auto& item = record.getItem(0);
        bool handle_RPTSCHED_RESTART = false;

        for (size_t index = 0; index < item.size(); ++index) {
            const std::string& mnemonic = item.get< std::string >(index);

            found_mnemonic_RESTART = mnemonic.find("RESTART=");
            if (found_mnemonic_RESTART != std::string::npos) {
                std::string restart_no = mnemonic.substr(found_mnemonic_RESTART+8, mnemonic.size());
                restart = boost::lexical_cast<size_t>(restart_no);
                handle_RPTSCHED_RESTART = true;
            }
            found_mnemonic_NOTHING = mnemonic.find("NOTHING");
            if (found_mnemonic_NOTHING != std::string::npos) {
                restart = 0;
                handle_RPTSCHED_RESTART = true;
            }
        }


        /* If no RESTART mnemonic is found, either it is not present or we might
           have an old data set containing integer controls instead of mnemonics.
           Restart integer switch is integer control nr 7 */

        if (found_mnemonic_RESTART == std::string::npos) {
            if (item.size() >= 7)  {
                const std::string& integer_control = item.get< std::string >(6);
                try {
                    restart = boost::lexical_cast<size_t>(integer_control);
                    handle_RPTSCHED_RESTART = true;
                } catch (boost::bad_lexical_cast &) {
                    //do nothing
                }
            }
        }


        if (handle_RPTSCHED_RESTART) {
            ioConfig->handleRPTSCHEDRestart(m_timeMap, step, restart);
        }

    }

    void Schedule::handleCOMPDAT( const DeckKeyword& keyword, size_t currentStep) {
        std::map<std::string , std::vector< CompletionPtr> > completionMapList = Completion::completionsFromCOMPDATKeyword( keyword );
        std::map<std::string , std::vector< CompletionPtr> >::iterator iter;

        for( iter= completionMapList.begin(); iter != completionMapList.end(); iter++) {
            const std::string wellName = iter->first;
            WellPtr well = getWell(wellName);
            well->addCompletions(currentStep, iter->second);
        }
        m_events->addEvent(ScheduleEvents::COMPLETION_CHANGE, currentStep);
    }

    void Schedule::handleWELSEGS( const DeckKeyword& keyword, size_t currentStep) {
        SegmentSetPtr newSegmentset= std::make_shared<SegmentSet>();
        newSegmentset->segmentsFromWELSEGSKeyword(keyword);

        const std::string& well_name = newSegmentset->wellName();
        WellPtr well = getWell(well_name);

        // update multi-segment related information for the well
        well->addSegmentSet(currentStep, newSegmentset);
    }

    void Schedule::handleCOMPSEGS( const DeckKeyword& keyword, size_t currentStep) {
        const auto& record1 = keyword.getRecord(0);
        const std::string& well_name = record1.getItem("WELL").getTrimmedString(0);
        WellPtr well = getWell(well_name);

        std::vector<CompsegsPtr> compsegs_vector = Compsegs::compsegsFromCOMPSEGSKeyword(keyword, m_grid);

        SegmentSetConstPtr current_segmentSet = well->getSegmentSet(currentStep);
        Compsegs::processCOMPSEGS(compsegs_vector, current_segmentSet);

        CompletionSetConstPtr current_completionSet = well->getCompletions(currentStep);
        // it is necessary to update the segment related information for some completions.
        CompletionSetPtr new_completionSet = CompletionSetPtr(current_completionSet->shallowCopy());
        Compsegs::updateCompletionsWithSegment(compsegs_vector, new_completionSet);

        well->addCompletionSet(currentStep, new_completionSet);
    }

    void Schedule::handleWGRUPCON( const DeckKeyword& keyword, size_t currentStep) {
        for( const auto& record : keyword ) {
            const std::string& wellName = record.getItem("WELL").getTrimmedString(0);
            WellPtr well = getWell(wellName);

            bool availableForGroupControl = convertEclipseStringToBool(record.getItem("GROUP_CONTROLLED").getTrimmedString(0));
            well->setAvailableForGroupControl(currentStep, availableForGroupControl);

            well->setGuideRate(currentStep, record.getItem("GUIDE_RATE").get< double >(0));

            if (!record.getItem("PHASE").defaultApplied(0)) {
                std::string guideRatePhase = record.getItem("PHASE").getTrimmedString(0);
                well->setGuideRatePhase(currentStep, GuideRate::GuideRatePhaseEnumFromString(guideRatePhase));
            } else
                well->setGuideRatePhase(currentStep, GuideRate::UNDEFINED);

            well->setGuideRateScalingFactor(currentStep, record.getItem("SCALING_FACTOR").get< double >(0));
        }
    }

    void Schedule::handleGRUPTREE( const DeckKeyword& keyword, size_t currentStep) {
        GroupTreePtr currentTree = m_rootGroupTree->get(currentStep);
        GroupTreePtr newTree = currentTree->deepCopy();
        for( const auto& record : keyword ) {
            const std::string& childName = record.getItem("CHILD_GROUP").getTrimmedString(0);
            const std::string& parentName = record.getItem("PARENT_GROUP").getTrimmedString(0);
            newTree->updateTree(childName, parentName);

            if (!hasGroup(parentName))
                addGroup( parentName , currentStep );

            if (!hasGroup(childName))
                addGroup( childName , currentStep );
        }
        m_rootGroupTree->update(currentStep, newTree);
    }

    void Schedule::handleWRFT( const DeckKeyword& keyword, size_t currentStep) {

        /* Rule for handling RFT: Request current RFT data output for specified wells, plus output when
         * any well is subsequently opened
         */

        for( const auto& record : keyword ) {

            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            const std::vector<WellPtr> wells = getWells(wellNamePattern);

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                well->setRFTActive(currentStep, true);
                size_t numStep = m_timeMap->numTimesteps();
                if(currentStep<numStep){
                    well->setRFTActive(currentStep+1, false);
                }
            }
        }

        for (auto iter = m_wells.begin(); iter != m_wells.end(); ++iter) {
            WellPtr well = *iter;
            well->setRFTForWellWhenFirstOpen(m_timeMap->numTimesteps(), currentStep);
        }
    }

    void Schedule::handleWRFTPLT( const DeckKeyword& keyword,  size_t currentStep) {

        for( const auto& record : keyword ) {

            const std::string& wellNamePattern = record.getItem("WELL").getTrimmedString(0);
            const std::vector<WellPtr> wells = getWells(wellNamePattern);

            RFTConnections::RFTEnum RFTKey = RFTConnections::RFTEnumFromString(record.getItem("OUTPUT_RFT").getTrimmedString(0));

            PLTConnections::PLTEnum PLTKey = PLTConnections::PLTEnumFromString(record.getItem("OUTPUT_PLT").getTrimmedString(0));

            for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                switch(RFTKey){
                    case RFTConnections::RFTEnum::YES:
                        well->setRFTActive(currentStep, true);
                        break;
                    case RFTConnections::RFTEnum::REPT:
                        well->setRFTActive(currentStep, true);
                        break;
                    case RFTConnections::RFTEnum::TIMESTEP:
                        well->setRFTActive(currentStep, true);
                        break;
                    case RFTConnections::RFTEnum::FOPN:
                        well->setRFTForWellWhenFirstOpen(m_timeMap->numTimesteps(),currentStep);
                        break;
                    case RFTConnections::RFTEnum::NO:
                        well->setRFTActive(currentStep, false);
                        break;
                }

                switch(PLTKey){
                    case PLTConnections::PLTEnum::YES:
                        well->setPLTActive(currentStep, true);
                        break;
                    case PLTConnections::PLTEnum::REPT:
                        well->setPLTActive(currentStep, true);
                        break;
                    case PLTConnections::PLTEnum::TIMESTEP:
                        well->setPLTActive(currentStep, true);
                        break;
                    case PLTConnections::PLTEnum::NO:
                        well->setPLTActive(currentStep, false);
                        break;
                }
            }
        }
    }

    TimeMapConstPtr Schedule::getTimeMap() const {
        return m_timeMap;
    }

    GroupTreePtr Schedule::getGroupTree(size_t timeStep) const {
        return m_rootGroupTree->get(timeStep);
    }

    void Schedule::addWell(const std::string& wellName, const DeckRecord& record, size_t timeStep, WellCompletion::CompletionOrderEnum wellCompletionOrder) {
        // We change from eclipse's 1 - n, to a 0 - n-1 solution
        int headI = record.getItem("HEAD_I").get< int >(0) - 1;
        int headJ = record.getItem("HEAD_J").get< int >(0) - 1;
        Phase::PhaseEnum preferredPhase = Phase::PhaseEnumFromString(record.getItem("PHASE").getTrimmedString(0));
        Value<double> refDepth("REF_DEPTH");
        WellPtr well;
        const auto& refDepthItem = record.getItem("REF_DEPTH");

        if (refDepthItem.hasValue(0))
            refDepth.setValue( refDepthItem.getSIDouble(0));

        bool allowCrossFlow = true;
        const std::string& allowCrossFlowStr = record.getItem<ParserKeywords::WELSPECS::CROSSFLOW>().getTrimmedString(0);
        if (allowCrossFlowStr == "NO")
            allowCrossFlow = false;

        well = std::make_shared<Well>(wellName, m_grid , headI, headJ, refDepth, preferredPhase, m_timeMap , timeStep, wellCompletionOrder, allowCrossFlow);
        m_wells.insert( wellName  , well);
        m_events->addEvent( ScheduleEvents::NEW_WELL , timeStep );
    }

    size_t Schedule::numWells() const {
        return m_wells.size();
    }

    size_t Schedule::numWells(size_t timestep) const {
      std::vector<WellConstPtr> wells = getWells(timestep);
      return wells.size();
    }

    bool Schedule::hasWell(const std::string& wellName) const {
        return m_wells.hasKey( wellName );
    }

    std::vector<WellConstPtr> Schedule::getWells() const {
        return getWells(m_timeMap->size()-1);
    }

    std::vector<WellConstPtr> Schedule::getWells(size_t timeStep) const {
        if (timeStep >= m_timeMap->size()) {
            throw std::invalid_argument("Timestep to large");
        }

        std::vector<WellConstPtr> wells;
        for (auto iter = m_wells.begin(); iter != m_wells.end(); ++iter) {
            WellConstPtr well = *iter;
            if (well->hasBeenDefined(timeStep)) {
                wells.push_back(well);
            }
        }
        return wells;
    }

    WellPtr Schedule::getWell(const std::string& wellName) {
        return m_wells.get( wellName );
    }

    const Well& Schedule::getWell(const std::string& wellName) const {
        return *m_wells.get( wellName );
    }


    /*
      Observe that this method only returns wells which have state ==
      OPEN; it does not include wells in state AUTO which might have
      been opened by the simulator.
    */

    std::vector<WellPtr> Schedule::getOpenWells(size_t timeStep) {
        std::vector<WellPtr> wells;
        for (auto well_iter = m_wells.begin(); well_iter != m_wells.end(); ++well_iter) {
            auto well = *well_iter;
            if (well->getStatus( timeStep ) == WellCommon::OPEN)
                wells.push_back( well );
        }
        return wells;
    }


    std::vector<WellPtr> Schedule::getWells(const std::string& wellNamePattern) {
        std::vector<WellPtr> wells;
        size_t wildcard_pos = wellNamePattern.find("*");
        if (wildcard_pos == wellNamePattern.length()-1) {
            for (auto wellIter = m_wells.begin(); wellIter != m_wells.end(); ++wellIter) {
                WellPtr well = *wellIter;
                if (Well::wellNameInWellNamePattern(well->name(), wellNamePattern)) {
                    wells.push_back (well);
                }
            }
        }
        else {
            wells.push_back(getWell(wellNamePattern));
        }
        return wells;
    }

    void Schedule::addGroup(const std::string& groupName, size_t timeStep) {
        if (!m_timeMap) {
            throw std::invalid_argument("TimeMap is null, can't add group named: " + groupName);
        }
        GroupPtr group(new Group(groupName, m_timeMap , timeStep));
        m_groups[ groupName ] = group;
        m_events->addEvent( ScheduleEvents::NEW_GROUP , timeStep );
    }

    size_t Schedule::numGroups() const {
        return m_groups.size();
    }

    bool Schedule::hasGroup(const std::string& groupName) const {
        return m_groups.find(groupName) != m_groups.end();
    }

    bool Schedule::initOnly() const {
        return nosim;
    }

    GroupPtr Schedule::getGroup(const std::string& groupName) const {
        if (hasGroup(groupName)) {
            return m_groups.at(groupName);
        } else
            throw std::invalid_argument("Group: " + groupName + " does not exist");
    }

    std::vector< const Group* > Schedule::getGroups() const {
        std::vector< const Group* > groups;

        for( const auto& itr : m_groups )
            groups.push_back( itr.second.get() );

        return groups;
    }

    void Schedule::addWellToGroup( GroupPtr newGroup , WellPtr well , size_t timeStep) {
        const std::string currentGroupName = well->getGroupName(timeStep);
        if (currentGroupName != "") {
            GroupPtr currentGroup = getGroup( currentGroupName );
            currentGroup->delWell( timeStep , well->name());
        }
        well->setGroupName(timeStep , newGroup->name());
        newGroup->addWell(timeStep , well);
    }


    double Schedule::convertInjectionRateToSI(double rawRate, WellInjector::TypeEnum wellType, const Opm::UnitSystem &unitSystem) {
        switch (wellType) {
        case WellInjector::MULTI:
            // multi-phase controlled injectors are a really funny
            // construct in Eclipse: the quantity controlled for is
            // not physically meaningful, i.e. Eclipse adds up
            // MCFT/day and STB/day.
            throw std::logic_error("There is no generic way to handle multi-phase injectors at this level!");

        case WellInjector::OIL:
        case WellInjector::WATER:
            return rawRate * unitSystem.parse("LiquidSurfaceVolume/Time")->getSIScaling();

        case WellInjector::GAS:
            return rawRate * unitSystem.parse("GasSurfaceVolume/Time")->getSIScaling();

        default:
            throw std::logic_error("Unknown injector type");
        }
    }

    double Schedule::convertInjectionRateToSI(double rawRate, Phase::PhaseEnum wellPhase, const Opm::UnitSystem& unitSystem) {
        switch (wellPhase) {
        case Phase::OIL:
        case Phase::WATER:
            return rawRate * unitSystem.parse("LiquidSurfaceVolume/Time")->getSIScaling();

        case Phase::GAS:
            return rawRate * unitSystem.parse("GasSurfaceVolume/Time")->getSIScaling();

        default:
            throw std::logic_error("Unknown injection phase");
        }
    }

    bool Schedule::convertEclipseStringToBool(const std::string& eclipseString) {
        std::string lowerTrimmed = boost::algorithm::to_lower_copy(eclipseString);
        boost::algorithm::trim(lowerTrimmed);

        if (lowerTrimmed == "y" || lowerTrimmed == "yes") {
            return true;
        }
        else if (lowerTrimmed == "n" || lowerTrimmed == "no") {
            return false;
        }
        else throw std::invalid_argument("String " + eclipseString + " not recognized as a boolean-convertible string.");
    }

    size_t Schedule::getMaxNumCompletionsForWells(size_t timestep) const {
      size_t ncwmax = 0;
      const std::vector<WellConstPtr>& wells = getWells();
      for (auto wellIter=wells.begin(); wellIter != wells.end(); ++wellIter) {
        WellConstPtr wellPtr = *wellIter;
        CompletionSetConstPtr completionsSetPtr = wellPtr->getCompletions(timestep);

        if (completionsSetPtr->size() > ncwmax )
          ncwmax = completionsSetPtr->size();

      }
      return ncwmax;
    }

    TuningPtr Schedule::getTuning() const {
      return m_tuning;
    }

    std::shared_ptr<const Deck> Schedule::getModifierDeck(size_t timeStep) const {
        return m_modifierDeck->iget( timeStep );
    }


    const MessageContainer& Schedule::getMessageContainer() const {
        return m_messages;
    }


    MessageContainer& Schedule::getMessageContainer() {
        return m_messages;
    }


    const Events& Schedule::getEvents() const {
        return *m_events;
    }

    OilVaporizationPropertiesConstPtr Schedule::getOilVaporizationProperties(size_t timestep){
        return m_oilvaporizationproperties->get(timestep);
    }

    void Schedule::setOilVaporizationProperties(const OilVaporizationPropertiesPtr vapor, size_t timestep){
        m_oilvaporizationproperties->update(timestep, vapor);
    }

    bool Schedule::hasOilVaporizationProperties(){
        return m_oilvaporizationproperties->size() > 0;
    }

}

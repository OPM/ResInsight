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

#include <iostream>

#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Completion.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/SegmentSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Util/Value.hpp>

#include <ert/ecl/ecl_grid.h>


namespace Opm {

    Well::Well(const std::string& name_, int headI,
               int headJ, Value<double> refDepth , Phase::PhaseEnum preferredPhase,
               TimeMapConstPtr timeMap, size_t creationTimeStep,
               WellCompletion::CompletionOrderEnum completionOrdering,
               bool allowCrossFlow, bool automaticShutIn)
        : m_status(new DynamicState<WellCommon::StatusEnum>(timeMap, WellCommon::SHUT)),
          m_isAvailableForGroupControl(new DynamicState<int>(timeMap, true)),
          m_guideRate(new DynamicState<double>(timeMap, -1.0)),
          m_guideRatePhase(new DynamicState<GuideRate::GuideRatePhaseEnum>(timeMap, GuideRate::UNDEFINED)),
          m_guideRateScalingFactor(new DynamicState<double>(timeMap, 1.0)),
          m_isProducer(new DynamicState<int>(timeMap, true)) ,
          m_completions( new DynamicState<CompletionSetConstPtr>( timeMap , CompletionSetConstPtr( new CompletionSet()) )),
          m_productionProperties( new DynamicState<WellProductionProperties>(timeMap, WellProductionProperties() )),
          m_injectionProperties( new DynamicState<WellInjectionProperties>(timeMap, WellInjectionProperties() )),
          m_polymerProperties( new DynamicState<WellPolymerProperties>(timeMap, WellPolymerProperties() )),
          m_econproductionlimits( new DynamicState<WellEconProductionLimits>(timeMap, WellEconProductionLimits()) ),
          m_solventFraction( new DynamicState<double>(timeMap, 0.0 )),
          m_groupName( new DynamicState<std::string>( timeMap , "" )),
          m_rft( new DynamicState<int>(timeMap,false)),
          m_plt( new DynamicState<int>(timeMap,false)),
          m_timeMap( timeMap ),
          m_headI(headI),
          m_headJ(headJ),
          m_refDepth(refDepth),
          m_preferredPhase(preferredPhase),
          m_comporder(completionOrdering),
          m_allowCrossFlow(allowCrossFlow),
          m_automaticShutIn(automaticShutIn),
          m_segmentset(new DynamicState<SegmentSetConstPtr>(timeMap, SegmentSetPtr(new SegmentSet())))
    {
        m_name = name_;
        m_creationTimeStep = creationTimeStep;


    }

    const std::string& Well::name() const {
        return m_name;
    }


    void Well::switchToProducer( size_t timeStep) {
        WellInjectionProperties p = getInjectionPropertiesCopy(timeStep);

        p.BHPLimit = 0;
        p.dropInjectionControl( Opm::WellInjector::BHP );
        setInjectionProperties( timeStep , p );
    }


    void Well::switchToInjector( size_t timeStep) {
        WellProductionProperties p = getProductionPropertiesCopy(timeStep);

        p.BHPLimit = 0;
        p.dropProductionControl( Opm::WellProducer::BHP );
        setProductionProperties( timeStep , p );
    }


    double Well::production_rate( Phase::PhaseEnum phase, size_t timestep ) const {
        if( !this->isProducer( timestep ) ) return 0.0;

        const auto& p = this->getProductionProperties( timestep );

        switch( phase ) {
            case Phase::WATER: return p.WaterRate;
            case Phase::OIL:   return p.OilRate;
            case Phase::GAS:   return p.GasRate;
        }

        throw std::logic_error( "Unreachable state. Invalid PhaseEnum value. "
                                "This is likely a programming error." );
    }

    double Well::injection_rate( Phase::PhaseEnum phase, size_t timestep ) const {
        if( !this->isInjector( timestep ) ) return 0.0;

        const auto& i = this->getInjectionProperties( timestep );
        const auto type = i.injectorType;

        if( phase == Phase::WATER && type != WellInjector::WATER ) return 0.0;
        if( phase == Phase::OIL   && type != WellInjector::OIL   ) return 0.0;
        if( phase == Phase::GAS   && type != WellInjector::GAS   ) return 0.0;

        return i.surfaceInjectionRate;
    }

    bool Well::setProductionProperties(size_t timeStep , const WellProductionProperties newProperties) {
        if (isInjector(timeStep))
            switchToProducer( timeStep );

        m_isProducer->update(timeStep , true);
        return m_productionProperties->update(timeStep, newProperties);
    }

    WellProductionProperties Well::getProductionPropertiesCopy(size_t timeStep) const {
        return m_productionProperties->get(timeStep);
    }

    const WellProductionProperties& Well::getProductionProperties(size_t timeStep) const {
        return m_productionProperties->at(timeStep);
    }

    bool Well::setInjectionProperties(size_t timeStep , const WellInjectionProperties newProperties) {
        if (isProducer(timeStep))
            switchToInjector( timeStep );

        m_isProducer->update(timeStep , false);
        return m_injectionProperties->update(timeStep, newProperties);
    }

    WellInjectionProperties Well::getInjectionPropertiesCopy(size_t timeStep) const {
        return m_injectionProperties->get(timeStep);
    }

    const WellInjectionProperties& Well::getInjectionProperties(size_t timeStep) const {
        return m_injectionProperties->at(timeStep);
    }

    bool Well::setPolymerProperties(size_t timeStep , const WellPolymerProperties newProperties) {
        m_isProducer->update(timeStep , false);
        return m_polymerProperties->update(timeStep, newProperties);
    }

    WellPolymerProperties Well::getPolymerPropertiesCopy(size_t timeStep) const {
        return m_polymerProperties->get(timeStep);
    }

    const WellPolymerProperties& Well::getPolymerProperties(size_t timeStep) const {
        return m_polymerProperties->at(timeStep);
    }

    bool Well::setSolventFraction(size_t timeStep , const double fraction) {
        m_isProducer->update(timeStep , false);
        return m_solventFraction->update(timeStep, fraction);
    }

    bool Well::setEconProductionLimits(const size_t timeStep, const WellEconProductionLimits& productionlimits) {
        // not sure if this keyword turning a well to be producer.
        // not sure what will happen if we use this keyword to a injector.
        return m_econproductionlimits->update(timeStep, productionlimits);
    }

    const WellEconProductionLimits& Well::getEconProductionLimits(const size_t timeStep) const {
        return m_econproductionlimits->at(timeStep);
    }

    const double& Well::getSolventFraction(size_t timeStep) const {
        return m_solventFraction->at(timeStep);
    }

    bool Well::hasBeenDefined(size_t timeStep) const {
        if (timeStep < m_creationTimeStep)
            return false;
        else
            return true;
    }

    WellCommon::StatusEnum Well::getStatus(size_t timeStep) const {
        return m_status->get( timeStep );
    }

    bool Well::setStatus(size_t timeStep, WellCommon::StatusEnum status) {
        if ((WellCommon::StatusEnum::OPEN == status) && getCompletions(timeStep)->allCompletionsShut()) {
            m_messages.note("When handling keyword for well " + name() + ": Cannot open a well where all completions are shut");
            return false;
        } else
            return m_status->update( timeStep , status );
    }

    const MessageContainer& Well::getMessageContainer() const {
        return m_messages;
    }
    bool Well::isProducer(size_t timeStep) const {
        return bool( m_isProducer->get(timeStep) );
    }

    bool Well::isInjector(size_t timeStep) const {
        return !bool( isProducer(timeStep) );
    }

    bool Well::isAvailableForGroupControl(size_t timeStep) const {
        return m_isAvailableForGroupControl->get(timeStep);
    }

    void Well::setAvailableForGroupControl(size_t timeStep, bool isAvailableForGroupControl_) {
        m_isAvailableForGroupControl->update(timeStep, isAvailableForGroupControl_);
    }

    double Well::getGuideRate(size_t timeStep) const {
        return m_guideRate->get(timeStep);
    }

    void Well::setGuideRate(size_t timeStep, double guideRate) {
        m_guideRate->update(timeStep, guideRate);
    }

    GuideRate::GuideRatePhaseEnum Well::getGuideRatePhase(size_t timeStep) const {
        return m_guideRatePhase->get(timeStep);
    }

    void Well::setGuideRatePhase(size_t timeStep, GuideRate::GuideRatePhaseEnum phase) {
        m_guideRatePhase->update(timeStep, phase);
    }

    double Well::getGuideRateScalingFactor(size_t timeStep) const {
        return m_guideRateScalingFactor->get(timeStep);
    }

    void Well::setGuideRateScalingFactor(size_t timeStep, double scalingFactor) {
        m_guideRateScalingFactor->update(timeStep, scalingFactor);
    }

    /*****************************************************************/

    // WELSPECS

    int Well::getHeadI() const {
        return m_headI;
    }

    int Well::getHeadJ() const {
        return m_headJ;
    }


    double Well::getRefDepth() const{
        if (!m_refDepth.hasValue())
            setRefDepthFromCompletions();

        return m_refDepth.getValue();
    }


    void Well::setRefDepthFromCompletions() const {
        size_t timeStep = m_creationTimeStep;
        while (true) {
            auto completions = getCompletions( timeStep );
            if (completions->size() > 0) {
                auto firstCompletion = completions->get(0);
                m_refDepth.setValue( firstCompletion->getCenterDepth() );
                break;
            } else {
                timeStep++;
                if (timeStep >= m_timeMap->size())
                    throw std::invalid_argument("No completions defined for well: " + name() + " can not infer reference depth");
            }
        }
    }


    Phase::PhaseEnum Well::getPreferredPhase() const {
        return m_preferredPhase;
    }

    CompletionSetConstPtr Well::getCompletions(size_t timeStep) const {
        return m_completions->get( timeStep );
    }

    CompletionSetConstPtr Well::getCompletions() const {
        return m_completions->back();
    }

    void Well::addCompletions(size_t time_step , const std::vector<CompletionPtr>& newCompletions) {
        CompletionSetConstPtr currentCompletionSet = m_completions->get(time_step);
        CompletionSetPtr newCompletionSet = CompletionSetPtr( currentCompletionSet->shallowCopy() );

        for (size_t ic = 0; ic < newCompletions.size(); ic++) {
            newCompletions[ic]->fixDefaultIJ( m_headI , m_headJ );
            newCompletionSet->add( newCompletions[ic] );
        }

        addCompletionSet( time_step , newCompletionSet);
    }

    void Well::addCompletionSet(size_t time_step, const CompletionSetConstPtr newCompletionSet){
        CompletionSetPtr mutable_copy(newCompletionSet->shallowCopy());
        if (getWellCompletionOrdering() == WellCompletion::TRACK) {
            mutable_copy->orderCompletions(m_headI, m_headJ);
        }
        m_completions->update(time_step, mutable_copy);
    }

    const std::string Well::getGroupName(size_t time_step) const {
        return m_groupName->get(time_step);
    }


    void Well::setGroupName(size_t time_step, const std::string& groupName ) {
        m_groupName->update(time_step , groupName);
    }



    void Well::setRFTActive(size_t time_step, bool value){
        m_rft->update(time_step, value);
    }

    bool Well::getRFTActive(size_t time_step) const{
        return bool( m_rft->get(time_step) );
    }

    bool Well::getPLTActive(size_t time_step) const{
     return bool( m_plt->get(time_step) );
    }
    void Well::setPLTActive(size_t time_step, bool value){
        m_plt->update(time_step, value);
    }

    /*
      The first report step where *either* RFT or PLT output is active.
    */
    int Well::firstRFTOutput( ) const {
        int rft_output = m_rft->find( true );
        int plt_output = m_plt->find( true );

        if (rft_output < plt_output) {
            if (rft_output >= 0)
                return rft_output;
            else
                return plt_output;
        } else {
            if (plt_output >= 0)
                return plt_output;
            else
                return rft_output;
        }
    }


    int Well::findWellFirstOpen(int startTimeStep) const{
        int numberOfTimeSteps = m_timeMap->numTimesteps();
        for(int i = startTimeStep; i < numberOfTimeSteps;i++){
            if(getStatus(i)==WellCommon::StatusEnum::OPEN){
                return i;
            }
        }
        return -1;
    }

    void Well::setRFTForWellWhenFirstOpen(int numSteps,size_t currentStep){
        int time;
        if(getStatus(currentStep)==WellCommon::StatusEnum::OPEN ){
            time = currentStep;
        }else {
            time = findWellFirstOpen(currentStep);
        }
        if(time>-1){
            setRFTActive(time, true);
            if(time < numSteps){
                setRFTActive(time+1, false);
            }
        }
    }

    WellCompletion::CompletionOrderEnum Well::getWellCompletionOrdering() const {
        return m_comporder;
    }


    bool Well::wellNameInWellNamePattern(const std::string& wellName, const std::string& wellNamePattern) {
        bool wellNameInPattern = false;
        if (util_fnmatch( wellNamePattern.c_str() , wellName.c_str()) == 0) {
            wellNameInPattern = true;
        }
        return wellNameInPattern;
    }

    bool Well::getAllowCrossFlow() const {
        return m_allowCrossFlow;
    }

    bool Well::getAutomaticShutIn() const {
        return m_automaticShutIn;
    }

    bool Well::canOpen(size_t currentStep) const {
        if( getAllowCrossFlow() ) return true;

        if( isInjector( currentStep ) )
            return getInjectionProperties( currentStep ).surfaceInjectionRate != 0;

        const auto& prod = getProductionProperties( currentStep );
        return (prod.WaterRate + prod.OilRate + prod.GasRate) != 0;
    }


    SegmentSetConstPtr Well::getSegmentSet(size_t time_step) const {
        return m_segmentset->get(time_step);
    }

    bool Well::isMultiSegment(size_t time_step) const {
        return (getSegmentSet(time_step)->numberSegment() > 0);
    }

    void Well::addSegmentSet(size_t time_step, SegmentSetConstPtr new_segmentset_in) {
        // to see if it is the first time entering WELSEGS input to this well.
        // if the well is not multi-segment well, it will be the first time
        // not sure if a well can switch between mutli-segment well and other
        // type of well
        // Here, we assume not
        const bool first_time = !isMultiSegment(time_step);

        if (first_time) {
            // overwrite the BHP reference depth with the one from WELSEGS keyword
            const double ref_depth = new_segmentset_in->depthTopSegment();
            m_refDepth.setValue(ref_depth);
            SegmentSetPtr new_segmentset = SegmentSetPtr(new_segmentset_in->shallowCopy());
            if (new_segmentset->lengthDepthType() == WellSegment::ABS) {
                new_segmentset->processABS();
            } else if (new_segmentset->lengthDepthType() == WellSegment::INC) {
                new_segmentset->processINC(first_time);
            } else {
                throw std::logic_error(" unknown length_depth_type in the new_segmentset");
            }
            m_segmentset->update(time_step, new_segmentset);
        } else {
            // checking the consistency of the input WELSEGS information
            throw std::logic_error("re-entering WELSEGS for a well is not supported yet!!.");
        }
    }

}




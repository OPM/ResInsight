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



#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellSet.hpp>

#define INVALID_GROUP_RATE -999e100
#define INVALID_EFFICIENCY_FACTOR 0.0


namespace Opm {
    namespace GroupProduction {
        struct ProductionData {
            ProductionData(TimeMapConstPtr timeMap);

            std::shared_ptr<DynamicState<GroupProduction::ControlEnum> > controlMode;
            std::shared_ptr<DynamicState<GroupProductionExceedLimit::ActionEnum> > exceedAction;
            std::shared_ptr<DynamicState<double> > oilTarget;
            std::shared_ptr<DynamicState<double> > waterTarget;
            std::shared_ptr<DynamicState<double> > gasTarget;
            std::shared_ptr<DynamicState<double> > liquidTarget;
            std::shared_ptr<DynamicState<double> > reservoirVolumeTarget;
            std::shared_ptr<DynamicState<double> > efficiencyFactor;
            std::shared_ptr<DynamicState<int> >    transferEfficiencyFactor;

        };

        ProductionData::ProductionData(TimeMapConstPtr timeMap) :
            controlMode( new DynamicState<GroupProduction::ControlEnum>(timeMap , GroupProduction::NONE)),
            exceedAction( new DynamicState<GroupProductionExceedLimit::ActionEnum>(timeMap , GroupProductionExceedLimit::NONE)),
            oilTarget( new DynamicState<double>(timeMap , INVALID_GROUP_RATE)),
            waterTarget( new DynamicState<double>(timeMap , INVALID_GROUP_RATE)),
            gasTarget( new DynamicState<double>(timeMap , INVALID_GROUP_RATE)),
            liquidTarget( new DynamicState<double>(timeMap , INVALID_GROUP_RATE)),
            reservoirVolumeTarget( new DynamicState<double>(timeMap , INVALID_GROUP_RATE)),
            efficiencyFactor( new DynamicState<double>(timeMap, INVALID_EFFICIENCY_FACTOR)),
            transferEfficiencyFactor( new DynamicState<int>(timeMap, false))
        {

        }
    }




    namespace GroupInjection {
        struct InjectionData {
            InjectionData(TimeMapConstPtr timeMap);

            std::shared_ptr<DynamicState<Phase::PhaseEnum> > phase;
            std::shared_ptr<DynamicState<GroupInjection::ControlEnum> > controlMode;
            std::shared_ptr<DynamicState<double> > rate;
            std::shared_ptr<DynamicState<double> > surfaceFlowMaxRate;
            std::shared_ptr<DynamicState<double> > reservoirFlowMaxRate;
            std::shared_ptr<DynamicState<double> > targetReinjectFraction;
            std::shared_ptr<DynamicState<double> > targetVoidReplacementFraction;
        };

        InjectionData::InjectionData(TimeMapConstPtr timeMap) :
            phase( new DynamicState<Phase::PhaseEnum>( timeMap , Phase::WATER )),
            controlMode( new DynamicState<GroupInjection::ControlEnum>( timeMap , NONE )),
            rate( new DynamicState<double>( timeMap , 0 )),
            surfaceFlowMaxRate( new DynamicState<double>( timeMap , 0)),
            reservoirFlowMaxRate( new DynamicState<double>( timeMap , 0)),
            targetReinjectFraction( new DynamicState<double>( timeMap , 0)),
            targetVoidReplacementFraction( new DynamicState<double>( timeMap , 0))
        {

        }
    }




    /*****************************************************************/

    Group::Group(const std::string& name_, TimeMapConstPtr timeMap , size_t creationTimeStep) :
        m_injection( new GroupInjection::InjectionData(timeMap) ),
        m_production( new GroupProduction::ProductionData( timeMap )),
        m_wells( new DynamicState< std::shared_ptr< const WellSet > >( timeMap , std::make_shared< const WellSet >() ) ),
        m_isProductionGroup( timeMap, false),
        m_isInjectionGroup( timeMap, false)
    {
        m_name = name_;
        m_creationTimeStep = creationTimeStep;
    }


    const std::string& Group::name() const {
        return m_name;
    }


    bool Group::hasBeenDefined(size_t timeStep) const {
        if (timeStep < m_creationTimeStep)
            return false;
        else
            return true;
    }

    bool Group::isProductionGroup(size_t timeStep) const {
        return bool( m_isProductionGroup.get(timeStep) );
    }

    bool Group::isInjectionGroup(size_t timeStep) const {
        return bool( m_isInjectionGroup.get(timeStep) );
    }

    void Group::setProductionGroup(size_t timeStep, bool isProductionGroup_) {
        m_isProductionGroup.update(timeStep, isProductionGroup_);
    }

    void Group::setInjectionGroup(size_t timeStep, bool isInjectionGroup_) {
        m_isInjectionGroup.update(timeStep, isInjectionGroup_);
    }


    /**********************************************************************/


    void Group::setInjectionPhase(size_t time_step , Phase::PhaseEnum phase){
        if (m_injection->phase->size() == time_step + 1) {
            Phase::PhaseEnum currentPhase = m_injection->phase->get(time_step);
            /*
              The ECLIPSE documentation of the GCONINJE keyword seems
              to indicate that a group can inject more than one phase
              simultaneously. This should be implemented in the input
              file as:

              GCONINJE
                 'GROUP'   'PHASE1'    'RATE'   ... /
                 'GROUP'   'PHASE2'    'RATE'   ... /
                 ...
              /

              I.e. the same group occurs more than once at the same
              time step, with different phases. This seems quite
              weird, and we do currently not support it. Changing the
              injected phase from one time step to the next is
              supported.
            */
            if (phase != currentPhase)
                throw std::invalid_argument("Sorry - we currently do not support injecting multiple phases at the same time.");
        }
        m_injection->phase->update( time_step , phase );
    }

    Phase::PhaseEnum Group::getInjectionPhase( size_t time_step ) const {
        return m_injection->phase->get( time_step );
    }

    void Group::setInjectionRate( size_t time_step , double rate) {
        m_injection->rate->update( time_step , rate);
    }

    double Group::getInjectionRate( size_t time_step ) const {
        return m_injection->rate->get( time_step );
    }

    void Group::setInjectionControlMode(size_t time_step , GroupInjection::ControlEnum controlMode) {
        m_injection->controlMode->update( time_step , controlMode );
    }

    GroupInjection::ControlEnum Group::getInjectionControlMode( size_t time_step) const {
        return m_injection->controlMode->get( time_step );
    }

    void Group::setSurfaceMaxRate( size_t time_step , double rate) {
        m_injection->surfaceFlowMaxRate->update( time_step , rate);
    }

    double Group::getSurfaceMaxRate( size_t time_step ) const {
        return m_injection->surfaceFlowMaxRate->get( time_step );
    }

    void Group::setReservoirMaxRate( size_t time_step , double rate) {
        m_injection->reservoirFlowMaxRate->update( time_step , rate);
    }

    double Group::getReservoirMaxRate( size_t time_step ) const {
        return m_injection->reservoirFlowMaxRate->get( time_step );
    }

    void Group::setTargetReinjectFraction( size_t time_step , double rate) {
        m_injection->targetReinjectFraction->update( time_step , rate);
    }

    double Group::getTargetReinjectFraction( size_t time_step ) const {
        return m_injection->targetReinjectFraction->get( time_step );
    }

    void Group::setTargetVoidReplacementFraction( size_t time_step , double rate) {
        m_injection->targetVoidReplacementFraction->update( time_step , rate);
    }

    double Group::getTargetVoidReplacementFraction( size_t time_step ) const {
        return m_injection->targetVoidReplacementFraction->get( time_step );
    }


    /*****************************************************************/

    void Group::setProductionControlMode( size_t time_step , GroupProduction::ControlEnum controlMode) {
        m_production->controlMode->update(time_step , controlMode );
    }

    GroupProduction::ControlEnum Group::getProductionControlMode( size_t time_step ) const {
        return m_production->controlMode->get(time_step);
    }


    GroupProductionExceedLimit::ActionEnum Group::getProductionExceedLimitAction( size_t time_step ) const  {
        return m_production->exceedAction->get(time_step);
    }


    void Group::setProductionExceedLimitAction( size_t time_step , GroupProductionExceedLimit::ActionEnum action) {
        m_production->exceedAction->update(time_step , action);
    }


    void Group::setOilTargetRate(size_t time_step , double oilTargetRate) {
        m_production->oilTarget->update(time_step , oilTargetRate);
    }


    double Group::getOilTargetRate(size_t time_step) const {
        return m_production->oilTarget->get(time_step);
    }


    void Group::setGasTargetRate(size_t time_step , double gasTargetRate) {
        m_production->gasTarget->update(time_step , gasTargetRate);
    }


    double Group::getGasTargetRate(size_t time_step) const {
        return m_production->gasTarget->get(time_step);
    }


    void Group::setWaterTargetRate(size_t time_step , double waterTargetRate) {
        m_production->waterTarget->update(time_step , waterTargetRate);
    }


    double Group::getWaterTargetRate(size_t time_step) const {
        return m_production->waterTarget->get(time_step);
    }


    void Group::setLiquidTargetRate(size_t time_step , double liquidTargetRate) {
        m_production->liquidTarget->update(time_step , liquidTargetRate);
    }


    double Group::getLiquidTargetRate(size_t time_step) const {
        return m_production->liquidTarget->get(time_step);
    }


    void Group::setReservoirVolumeTargetRate(size_t time_step , double reservoirVolumeTargetRate) {
        m_production->reservoirVolumeTarget->update(time_step , reservoirVolumeTargetRate);
    }


    double Group::getReservoirVolumeTargetRate(size_t time_step) const {
        return m_production->reservoirVolumeTarget->get(time_step);
    }


    void Group::setGroupEfficiencyFactor(size_t time_step, double factor) {
        m_production->efficiencyFactor->update(time_step , factor);
    }

    double Group::getGroupEfficiencyFactor(size_t time_step) const {
        return m_production->efficiencyFactor->get(time_step);
    }

    void  Group::setTransferGroupEfficiencyFactor(size_t time_step, bool transfer) {
        m_production->transferEfficiencyFactor->update(time_step , transfer);
    }


    bool  Group::getTransferGroupEfficiencyFactor(size_t time_step) const {
        return m_production->transferEfficiencyFactor->get(time_step);
    }

    /*****************************************************************/

    std::shared_ptr< const WellSet > Group::wellMap(size_t time_step) const {
        return m_wells->get(time_step);
    }


    bool Group::hasWell(const std::string& wellName , size_t time_step) const {
        return this->wellMap(time_step)->hasWell( wellName );
    }


    const Well* Group::getWell(const std::string& wellName , size_t time_step) const {
        return this->wellMap( time_step )->getWell( wellName );
    }

    const WellSet& Group::getWells( size_t time_step ) const {
        return *this->wellMap( time_step );
    }

    size_t Group::numWells(size_t time_step) const {
        return wellMap(time_step)->size();
    }

    void Group::addWell(size_t time_step, Well* well ) {
        auto wellSet = wellMap(time_step);
        std::shared_ptr< WellSet > newWellSet( wellSet->shallowCopy() );

        newWellSet->addWell(well);
        m_wells->update(time_step , newWellSet);
    }


    void Group::delWell(size_t time_step, const std::string& wellName) {
        auto wellSet = wellMap(time_step);
        std::shared_ptr< WellSet > newWellSet( wellSet->shallowCopy() );

        newWellSet->delWell(wellName);
        m_wells->update(time_step , newWellSet);
    }

}




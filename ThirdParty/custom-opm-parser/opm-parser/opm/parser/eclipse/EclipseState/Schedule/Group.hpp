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


#ifndef GROUP_HPP_
#define GROUP_HPP_

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>

#include <memory>
#include <string>

namespace Opm {


    class TimeMap;
    class Well;
    class WellSet;

    namespace GroupInjection {
        struct InjectionData;
    }


    namespace GroupProduction {
        struct ProductionData;
    }

    class Group {
    public:
        Group(const std::string& name, std::shared_ptr< const TimeMap > timeMap , size_t creationTimeStep);
        bool hasBeenDefined(size_t timeStep) const;
        const std::string& name() const;
        bool isProductionGroup(size_t timeStep) const;
        bool isInjectionGroup(size_t timeStep) const;
        void setProductionGroup(size_t timeStep, bool isProductionGroup);
        void setInjectionGroup(size_t timeStep, bool isInjectionGroup_);

        /******************************************************************/
        void             setInjectionPhase(size_t time_step , Phase::PhaseEnum phase);
        Phase::PhaseEnum getInjectionPhase(size_t time_step) const;

        void                      setInjectionControlMode(size_t time_step , GroupInjection::ControlEnum ControlMode);
        GroupInjection::ControlEnum getInjectionControlMode( size_t time_step) const;

        void   setInjectionRate(size_t time_step , double rate);
        double getInjectionRate( size_t time_step) const;

        void   setSurfaceMaxRate( size_t time_step , double rate);
        double getSurfaceMaxRate( size_t time_step ) const;

        void   setReservoirMaxRate( size_t time_step , double rate);
        double getReservoirMaxRate( size_t time_step ) const;

        void   setTargetReinjectFraction( size_t time_step , double rate);
        double getTargetReinjectFraction( size_t time_step ) const;

        void   setTargetVoidReplacementFraction( size_t time_step , double rate);
        double getTargetVoidReplacementFraction( size_t time_step ) const;

        /******************************************************************/

        void   setProductionControlMode( size_t time_step , GroupProduction::ControlEnum controlMode);
        GroupProduction::ControlEnum getProductionControlMode( size_t time_step ) const;

        GroupProductionExceedLimit::ActionEnum getProductionExceedLimitAction(size_t time_step) const;
        void setProductionExceedLimitAction(size_t time_step , GroupProductionExceedLimit::ActionEnum action);

        void   setOilTargetRate(size_t time_step , double oilTargetRate);
        double getOilTargetRate(size_t time_step) const;
        void   setGasTargetRate(size_t time_step , double gasTargetRate);
        double getGasTargetRate(size_t time_step) const;
        void   setWaterTargetRate(size_t time_step , double waterTargetRate);
        double getWaterTargetRate(size_t time_step) const;
        void   setLiquidTargetRate(size_t time_step , double liquidTargetRate);
        double getLiquidTargetRate(size_t time_step) const;
        void   setReservoirVolumeTargetRate(size_t time_step , double reservoirVolumeTargetRate);
        double getReservoirVolumeTargetRate(size_t time_step) const;
        void   setGroupEfficiencyFactor(size_t time_step, double factor);
        double getGroupEfficiencyFactor(size_t time_step) const;
        void   setTransferGroupEfficiencyFactor(size_t time_step, bool transfer);
        bool   getTransferGroupEfficiencyFactor(size_t time_step) const;

        /*****************************************************************/

        bool hasWell(const std::string& wellName , size_t time_step) const;
        const Well* getWell(const std::string& wellName , size_t time_step) const;
        const WellSet& getWells( size_t time_step ) const;
        size_t numWells(size_t time_step) const;
        void addWell(size_t time_step , Well* well);
        void delWell(size_t time_step, const std::string& wellName );
    private:
        std::shared_ptr< const WellSet > wellMap(size_t time_step) const;

        size_t m_creationTimeStep;
        std::string m_name;
        std::shared_ptr<GroupInjection::InjectionData> m_injection;
        std::shared_ptr<GroupProduction::ProductionData> m_production;
        std::shared_ptr<DynamicState<std::shared_ptr< const WellSet >> > m_wells;
        DynamicState<int> m_isProductionGroup;
        DynamicState<int> m_isInjectionGroup;
    };
    typedef std::shared_ptr<Group> GroupPtr;
    typedef std::shared_ptr<const Group> GroupConstPtr;
}



#endif /* WELL_HPP_ */

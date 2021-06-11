/*
  Copyright 2019 Equinor ASA.

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

#ifndef GROUP2_HPP
#define GROUP2_HPP


#include <map>
#include <optional>
#include <string>

#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/EclipseState/Util/IOrderSet.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

namespace Opm {

class SummaryState;
class Group {
public:

// A group can have both injection controls and production controls set at
// the same time, i.e. this enum is used as a bitmask.
enum class GroupType : unsigned {
    NONE = 0,
    PRODUCTION = 1,
    INJECTION = 2,
    MIXED = 3
};



enum class ExceedAction {
    NONE = 0,
    CON = 1,
    CON_PLUS = 2,   // String: "+CON"
    WELL = 3,
    PLUG = 4,
    RATE = 5
};
static const std::string ExceedAction2String( ExceedAction enumValue );
static ExceedAction ExceedActionFromString( const std::string& stringValue );


enum class InjectionCMode  : int {
    NONE = 0,
    RATE = 1,
    RESV = 2,
    REIN = 4,
    VREP = 8,
    FLD  = 16,
    SALE = 32
};
static const std::string InjectionCMode2String( InjectionCMode enumValue );
static InjectionCMode InjectionCModeFromString( const std::string& stringValue );


enum class ProductionCMode : int {
    NONE = 0,
    ORAT = 1,
    WRAT = 2,
    GRAT = 4,
    LRAT = 8,
    CRAT = 16,
    RESV = 32,
    PRBL = 64,
    FLD  = 128
};
static const std::string ProductionCMode2String( ProductionCMode enumValue );
static ProductionCMode ProductionCModeFromString( const std::string& stringValue );


enum class GuideRateTarget {
    OIL = 0,
    WAT = 1,
    GAS = 2,
    LIQ = 3,
    RES = 4,
    COMB = 5,
    WGA =  6,
    CVAL = 7,
    INJV = 8,
    POTN = 9,
    FORM = 10,
    NO_GUIDE_RATE = 11
};
static GuideRateTarget GuideRateTargetFromString( const std::string& stringValue );



struct GroupInjectionProperties {
    Phase phase = Phase::WATER;
    InjectionCMode cmode = InjectionCMode::NONE;
    UDAValue surface_max_rate;
    UDAValue resv_max_rate;
    UDAValue target_reinj_fraction;
    UDAValue target_void_fraction;
    std::string reinj_group;
    std::string voidage_group;
    bool available_group_control;

    static GroupInjectionProperties serializeObject();

    int injection_controls = 0;
    bool operator==(const GroupInjectionProperties& other) const;
    bool operator!=(const GroupInjectionProperties& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(phase);
        serializer(cmode);
        surface_max_rate.serializeOp(serializer);
        resv_max_rate.serializeOp(serializer);
        target_reinj_fraction.serializeOp(serializer);
        target_void_fraction.serializeOp(serializer);
        serializer(reinj_group);
        serializer(voidage_group);
        serializer(injection_controls);
        serializer(available_group_control);
    }
};

struct InjectionControls {
    Phase phase;
    InjectionCMode cmode;
    double surface_max_rate;
    double resv_max_rate;
    double target_reinj_fraction;
    double target_void_fraction;
    int injection_controls = 0;
    std::string reinj_group;
    std::string voidage_group;
    bool has_control(InjectionCMode control) const;
};

struct GroupProductionProperties {
    ProductionCMode cmode = ProductionCMode::NONE;
    ExceedAction exceed_action = ExceedAction::NONE;
    UDAValue oil_target;
    UDAValue water_target;
    UDAValue gas_target;
    UDAValue liquid_target;
    double guide_rate;
    GuideRateTarget guide_rate_def;
    double resv_target = 0;
    bool available_group_control = true;
    static GroupProductionProperties serializeObject();

    int production_controls = 0;
    bool operator==(const GroupProductionProperties& other) const;
    bool operator!=(const GroupProductionProperties& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(cmode);
        serializer(exceed_action);
        oil_target.serializeOp(serializer);
        water_target.serializeOp(serializer);
        gas_target.serializeOp(serializer);
        liquid_target.serializeOp(serializer);
        serializer(guide_rate);
        serializer(guide_rate_def);
        serializer(resv_target);
        serializer(available_group_control);
        serializer(production_controls);
    }
};


struct ProductionControls {
    ProductionCMode cmode;
    ExceedAction exceed_action;
    double oil_target;
    double water_target;
    double gas_target;
    double liquid_target;
    double guide_rate;
    GuideRateTarget guide_rate_def;
    double resv_target = 0;
    int production_controls = 0;
    bool has_control(ProductionCMode control) const;
};


    Group();
    Group(const std::string& group_name, std::size_t insert_index_arg, std::size_t init_step_arg, double udq_undefined_arg, const UnitSystem& unit_system);

    static Group serializeObject();

    bool defined(std::size_t timeStep) const;
    std::size_t insert_index() const;
    const std::string& name() const;
    int getGroupNetVFPTable() const;

    bool updateNetVFPTable(int vfp_arg);
    bool update_gefac(double gefac, bool transfer_gefac);

    // [[deprecated("use Group::control_group() or Group::flow_group()")]]
    const std::string& parent() const;
    std::optional<std::string> control_group() const;
    std::optional<std::string> flow_group() const;

    bool updateParent(const std::string& parent);
    bool updateInjection(const GroupInjectionProperties& injection);
    bool updateProduction(const GroupProductionProperties& production);
    bool isProductionGroup() const;
    bool isInjectionGroup() const;
    void setProductionGroup();
    void setInjectionGroup();
    double getGroupEfficiencyFactor() const;
    bool   getTransferGroupEfficiencyFactor() const;

    std::size_t numWells() const;
    bool addGroup(const std::string& group_name);
    bool hasGroup(const std::string& group_name) const;
    void delGroup(const std::string& group_name);
    bool addWell(const std::string& well_name);
    bool hasWell(const std::string& well_name) const;
    void delWell(const std::string& well_name);

    const std::vector<std::string>& wells() const;
    const std::vector<std::string>& groups() const;
    bool wellgroup() const;
    ProductionControls productionControls(const SummaryState& st) const;
    InjectionControls injectionControls(Phase phase, const SummaryState& st) const;
    bool hasInjectionControl(Phase phase) const;
    const GroupProductionProperties& productionProperties() const;
    const std::map<Phase , GroupInjectionProperties>& injectionProperties() const;
    const GroupType& getGroupType() const;
    ProductionCMode production_cmode() const;
    InjectionCMode injection_cmode() const;
    Phase injection_phase() const;
    bool has_control(ProductionCMode control) const;
    bool has_control(InjectionCMode control) const;
    bool productionGroupControlAvailable() const;
    bool injectionGroupControlAvailable(const Phase phase) const;

    bool operator==(const Group& data) const;
    const Phase& topup_phase() const;
    bool has_topup_phase() const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_name);
        serializer(m_insert_index);
        serializer(init_step);
        serializer(udq_undefined);
        unit_system.serializeOp(serializer);
        serializer(group_type);
        serializer(gefac);
        serializer(transfer_gefac);
        serializer(vfp_table);
        serializer(parent_group);
        m_wells.serializeOp(serializer);
        m_groups.serializeOp(serializer);
        serializer.map(injection_properties);
        production_properties.serializeOp(serializer);
        serializer(m_topup_phase);
    }

private:
    bool hasType(GroupType gtype) const;
    void addType(GroupType new_gtype);

    std::string m_name;
    std::size_t m_insert_index;
    std::size_t init_step;
    double udq_undefined;
    UnitSystem unit_system;
    GroupType group_type;
    double gefac;
    bool transfer_gefac;
    int vfp_table;

    std::string parent_group;
    IOrderSet<std::string> m_wells;
    IOrderSet<std::string> m_groups;

    std::map<Phase, GroupInjectionProperties> injection_properties;
    GroupProductionProperties production_properties{};
    std::pair<Phase, bool> m_topup_phase{Phase::WATER, false};
};

Group::GroupType operator |(Group::GroupType lhs, Group::GroupType rhs);
Group::GroupType operator &(Group::GroupType lhs, Group::GroupType rhs);

}

#endif

/*
  Copyright 2015 Statoil ASA.

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
#include <fmt/format.h>

#include <opm/input/eclipse/Schedule/Events.hpp>

namespace Opm {


    Events Events::serializeObject()
    {
        Events result;
        result.m_events = 12345;
        return result;
    }

    bool Events::hasEvent(uint64_t eventMask) const {
        return (this->m_events & eventMask);
    }

    void Events::addEvent(ScheduleEvents::Events event) {
        this->m_events |= event;
    }

    bool Events::operator==(const Events& data) const {
        return this->m_events == data.m_events;
    }

    void Events::clearEvent(uint64_t eventMask) {
        auto diff = this->m_events & eventMask;
        this->m_events -= diff;
    }

    void Events::reset() {
        this->m_events = 0;
    }


    WellGroupEvents WellGroupEvents::serializeObject() {
        WellGroupEvents wg;
        wg.addWell("WG1");
        wg.addGroup("GG1");
        return wg;
    }

    void WellGroupEvents::addWell(const std::string& wname) {
        Events events;
        events.addEvent( ScheduleEvents::NEW_WELL );
        this->m_wellgroup_events.insert( std::make_pair( wname, events ));
    }

    void WellGroupEvents::addGroup(const std::string& gname) {
        Events events;
        events.addEvent( ScheduleEvents::NEW_GROUP );
        this->m_wellgroup_events.insert( std::make_pair( gname, events ));
    }

    bool WellGroupEvents::hasEvent(const std::string& wgname, uint64_t eventMask) const {
        const auto events_iter = this->m_wellgroup_events.find(wgname);
        if (events_iter == this->m_wellgroup_events.end())
            return false;
        return events_iter->second.hasEvent(eventMask);
    }

    void WellGroupEvents::clearEvent(const std::string& wgname, uint64_t eventMask) {
        const auto events_iter = this->m_wellgroup_events.find(wgname);
        if (events_iter != this->m_wellgroup_events.end())
            events_iter->second.clearEvent(eventMask);
    }

    void WellGroupEvents::addEvent(const std::string& wgname, ScheduleEvents::Events event) {
        const auto events_iter = this->m_wellgroup_events.find(wgname);
        if (events_iter == this->m_wellgroup_events.end())
            throw std::logic_error(fmt::format("Adding event for unknown well/group: {}", wgname));
        events_iter->second.addEvent(event);
    }

    void WellGroupEvents::reset() {
        for (auto& [_, events] : this->m_wellgroup_events) {
            (void)_;
            events.reset();
        }
    }

    bool WellGroupEvents::operator==(const WellGroupEvents& data) const {
        return this->m_wellgroup_events == data.m_wellgroup_events;
    }


    const Events& WellGroupEvents::at(const std::string& wgname) const {
        return this->m_wellgroup_events.at(wgname);
    }


    bool WellGroupEvents::has(const std::string& wgname) const {
        return this->m_wellgroup_events.count(wgname);
    }

}

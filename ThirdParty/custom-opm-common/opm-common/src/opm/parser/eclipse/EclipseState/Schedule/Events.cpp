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
#include <cstddef>

#include <opm/parser/eclipse/EclipseState/Schedule/Events.hpp>

namespace Opm {

    Events::Events(const TimeMap& timeMap) :
        m_events( DynamicVector<uint64_t>( timeMap , 0 ) )
    { }


    Events Events::serializeObject()
    {
        Events result;
        result.m_events = DynamicVector<uint64_t>({1,2,3,4,5});

        return result;
    }


    bool Events::hasEvent(uint64_t eventMask , size_t reportStep) const {
        uint64_t eventSum = m_events[reportStep];
        if (eventSum & eventMask)
            return true;
        else
            return false;
    }


    void Events::addEvent(ScheduleEvents::Events event, size_t reportStep) {
        m_events[reportStep] |= event;
    }

    bool Events::operator==(const Events& data) const {
        return this->m_events == data.m_events;
    }

}


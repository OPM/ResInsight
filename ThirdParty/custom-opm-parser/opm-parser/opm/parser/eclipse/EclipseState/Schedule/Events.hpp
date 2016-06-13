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
#ifndef SCHEDULE_EVENTS_HPP
#define SCHEDULE_EVENTS_HPP

#include <cstdint>

#include <opm/parser/eclipse/EclipseState/Schedule/DynamicVector.hpp>

namespace Opm
{
    namespace ScheduleEvents {
        // These values are used as bitmask - 2^n structure is essential.
        enum Events {
            /* The NEW_WELL event is triggered by the WELSPECS
               keyword. */
            NEW_WELL = 1,

            /*
               The NEW_GROUP event is triggered by the WELSPECS and
               GRUPTREE keywords.
            */
            NEW_GROUP = 2,

            /*
               The PRODUCTION_UPDATE event is triggered by the
               WCONPROD and WCONHIST keywords. The event will be
               triggered if *any* of the elements in one of keywords
               is changed. Quite simlar for INJECTION_UPDATE and
               POLYMER_UPDATE.
            */
            PRODUCTION_UPDATE = 4,
            INJECTION_UPDATE = 8,
            POLYMER_UPDATES = 16,

            /*
              This event is triggered if the well status is changed
              between {OPEN,SHUT,STOP,AUTO}. There are many keywords
              which can trigger a well status change.
            */
            WELL_STATUS_CHANGE = 32,

            /*
              COMPDAT and WELOPEN
            */
            COMPLETION_CHANGE = 64,

            /*
              The well group topolyg has changed.
            */
            GROUP_CHANGE = 128,


            /*
              Geology modifier.
            */
            GEO_MODIFIER = 256
        };
    }

    /*
      This class implements a simple system for recording when various
      events happen in the Schedule file. The purpose of the class is
      that downstream code can query this system whether a certain a
      event has taken place, and then perform potentially expensive
      calculations conditionally:

        auto events = schedule->getEvents();
        if (events.hasEvent(SchedulEvents::NEW_WELL , reportStep))
           // Perform expensive calculation which must be performed
           // when a new well is introduced.
           ...

    */

    class Events {
    public:
        Events(std::shared_ptr<const TimeMap> timeMap);
        void addEvent(ScheduleEvents::Events event, size_t reportStep);
        bool hasEvent(uint64_t eventMask, size_t reportStep) const;
    private:
        DynamicVector<uint64_t> m_events;
    };
}

#endif

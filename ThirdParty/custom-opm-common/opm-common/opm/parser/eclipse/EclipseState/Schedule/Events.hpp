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
            /*
               The NEW_WELL event is triggered by the WELSPECS
               keyword. For wells the event is triggered the first
               time the well is mentioned in the WELSPECS keyword, for
               the Schedule object the NEW_WELL event is triggered
               every time a WELSPECS keyword is encountered.
            */
            NEW_WELL = 1,

            /*
              WHen the well data is updated with the WELSPECS keyword
              this event is triggered. Only applies to individual
              wells, and not the global Schedule object.
            */
            WELL_WELSPECS_UPDATE = 2,


            //WELL_POLYMER_UPDATE = 4,
            /*
               The NEW_GROUP event is triggered by the WELSPECS and
               GRUPTREE keywords.
            */
            NEW_GROUP = 8,

            /*
               The PRODUCTION_UPDATE event is triggered by the
               WCONPROD and WCONHIST keywords. The event will be
               triggered if *any* of the elements in one of keywords
               is changed. Quite simlar for INJECTION_UPDATE and
               POLYMER_UPDATE.
            */
            PRODUCTION_UPDATE = 16,
            INJECTION_UPDATE = 32,
            //POLYMER_UPDATES = 64,

            /*
              This event is triggered if the well status is changed
              between {OPEN,SHUT,STOP,AUTO}. There are many keywords
              which can trigger a well status change.
            */
            WELL_STATUS_CHANGE = 128,

            /*
              COMPDAT and WELOPEN
            */
            COMPLETION_CHANGE = 256,

            /*
              The well group topolyg has changed.
            */
            GROUP_CHANGE = 512,


            /*
              Geology modifier.
            */
            GEO_MODIFIER = 1024,

            /*
              TUNING has changed
            */
            TUNING_CHANGE = 2048,

            /* The VFP tables have changed */
            VFPINJ_UPDATE = 4096,
            VFPPROD_UPDATE = 8192,


            /*
              GROUP production or injection targets has changed
            */
            GROUP_PRODUCTION_UPDATE = 16384,
            GROUP_INJECTION_UPDATE = 32768
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
        Events() = default;
        explicit Events(const TimeMap& timeMap);

        static Events serializeObject();

        void addEvent(ScheduleEvents::Events event, size_t reportStep);
        bool hasEvent(uint64_t eventMask, size_t reportStep) const;

        bool operator==(const Events& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            m_events.template serializeOp<Serializer,false>(serializer);
        }

    private:
        DynamicVector<uint64_t> m_events;
    };
}

#endif

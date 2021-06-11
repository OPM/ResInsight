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


#ifndef TIMEMAP_HPP_
#define TIMEMAP_HPP_

#include <opm/common/utility/TimeService.hpp>

#include <vector>
#include <ctime>
#include <map>
#include <utility>
#include <iostream>

#include <stddef.h>

namespace Opm {

    class Deck;
    class DeckKeyword;
    class DeckRecord;

    class TimeMap {
    public:
        TimeMap() = default;
        explicit TimeMap(const Deck& deck, const std::pair<std::time_t, std::size_t>& restart = std::make_pair(std::time_t{0}, std::size_t{0}));
        explicit TimeMap(const std::vector<std::time_t>& time_points);

        static TimeMap serializeObject();

        size_t size() const;
        size_t last() const;
        size_t numTimesteps() const;
        double getTotalTime() const;
        double seconds(size_t timeStep) const;
        std::size_t restart_offset() const;

        std::time_t operator[] (size_t index) const;
        /// Return the date and time where a given time step starts.
        std::time_t getStartTime(size_t tStepIdx) const;
        std::time_t getEndTime() const;
        bool skiprest() const;
        /// Return the period of time in seconds which passed between the start of the simulation and a given point in time.
        double getTimePassedUntil(size_t tLevelIdx) const;
        /// Return the length of a given time step in seconds.
        double getTimeStepLength(size_t tStepIdx) const;

        const std::vector<std::time_t>& timeList() const;
        bool operator==(const TimeMap& data) const;

        /// Return true if the given timestep is the first one of a new month or year, or if frequency > 1,
        /// return true if the step is the first of each n-month or n-month period, starting from start_timestep - 1.
        bool isTimestepInFirstOfMonthsYearsSequence(size_t timestep, bool years = true, size_t start_timestep = 1, size_t frequency = 1) const;

        static std::time_t timeFromEclipse(const DeckRecord &dateRecord);

        static std::time_t forward(std::time_t t0, int64_t hours, int64_t minutes, int64_t seconds);
        static std::time_t forward(std::time_t t0, int64_t seconds);
        static std::time_t mkdate(int year, int month, int day);
        static std::time_t mkdatetime(int year, int month, int day, int hour, int minute, int second);
        static const std::map<std::string, int>& eclipseMonthIndices();

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_timeList);
            serializer.vector(m_first_timestep_years);
            serializer.vector(m_first_timestep_months);
            serializer(m_skiprest);
            serializer(m_restart_offset);
        }

    private:
        struct StepData
        {
            size_t stepnumber;
            TimeStampUTC timestamp;

            bool operator==(const StepData& data) const
            {
                return stepnumber == data.stepnumber &&
                       timestamp == data.timestamp;
            }

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(stepnumber);
                timestamp.serializeOp(serializer);
            }
        };

        bool isTimestepInFreqSequence (size_t timestep, size_t start_timestep, size_t frequency, bool years) const;
        size_t closest(const std::vector<size_t> & vec, size_t value) const;
        void addTStep(int64_t step);
        void addTime(std::time_t newTime);
        void addFromTSTEPKeyword( const DeckKeyword& TSTEPKeyword );
        void init_start(std::time_t start_time);

        std::vector<std::time_t> m_timeList;
        std::vector<StepData> m_first_timestep_years;   // A list of the first timestep of every year
        std::vector<StepData> m_first_timestep_months;  // A list of the first timestep of every month
        bool m_skiprest = false;
        std::size_t m_restart_offset = 0;
    };

std::ostream& operator<<(std::ostream& stream, const TimeMap& tm);

}



#endif /* TIMEMAP_HPP_ */

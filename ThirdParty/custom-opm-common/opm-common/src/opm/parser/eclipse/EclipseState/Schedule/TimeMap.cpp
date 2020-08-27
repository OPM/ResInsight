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

#include <cassert>
#include <ctime>
#include <stddef.h>
#include <iomanip>

#include <opm/common/utility/TimeService.hpp>

#include <opm/parser/eclipse/Parser/ParserKeywords/S.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckSection.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/common/utility/String.hpp>


constexpr const std::time_t invalid_time = -1;

namespace Opm {

namespace {
    const std::map<std::string, int> month_indices = {{"JAN", 1},
                                                      {"FEB", 2},
                                                      {"MAR", 3},
                                                      {"APR", 4},
                                                      {"MAI", 5},
                                                      {"MAY", 5},
                                                      {"JUN", 6},
                                                      {"JUL", 7},
                                                      {"JLY", 7},
                                                      {"AUG", 8},
                                                      {"SEP", 9},
                                                      {"OCT", 10},
                                                      {"OKT", 10},
                                                      {"NOV", 11},
                                                      {"DEC", 12},
                                                      {"DES", 12}};

}

    void TimeMap::init_start(std::time_t start_time) {
        auto timestamp = TimeStampUTC{start_time};

        this->m_timeList.push_back(start_time);
        this->m_first_timestep_months.push_back({0, timestamp});
        this->m_first_timestep_years.push_back({0, timestamp});
    }


    TimeMap::TimeMap(const std::vector<std::time_t>& time_points) {
        if (time_points.empty())
            throw std::invalid_argument("Can not initialize with empty list of time points");

        this->init_start(time_points[0]);
        for (std::size_t ti = 1; ti < time_points.size(); ti++) {
            if (time_points[ti] == invalid_time) {
                this->m_timeList.push_back(invalid_time);
                this->m_restart_offset += 1;
            }
            else
                this->addTime( time_points[ti] );
        }
        if (this->m_restart_offset > 0)
            this->m_restart_offset += 1;
    }

    TimeMap::TimeMap( const Deck& deck, const std::pair<std::time_t, std::size_t>& restart) {
        bool skiprest = deck.hasKeyword<ParserKeywords::SKIPREST>();
        {
            std::time_t start_time;
            if (deck.hasKeyword("START")) {
                // Use the 'START' keyword to find out the start date (if the
                // keyword was specified)
                const auto& keyword = deck.getKeyword("START");
                start_time = timeFromEclipse(keyword.getRecord(0));
            } else {
                // The default start date is not specified in the Eclipse
                // reference manual. We hence just assume it is same as for
                // the START keyword for Eclipse R100, i.e., January 1st,
                // 1983...
                start_time = mkdate(1983, 1, 1);
            }
            this->init_start(start_time);
        }

        auto restart_time = restart.first;
        this->m_restart_offset = restart.second;
        bool skip = false;

        for (std::size_t it = 1; it < this->m_restart_offset; it++)
            this->m_timeList.push_back(invalid_time);

        if (this->m_restart_offset > 0) {
            if (skiprest)
                skip = true;
            else {
                this->m_timeList.push_back(restart_time);
                skip = false;
            }
        }

        for( const auto& keyword : SCHEDULESection(deck)) {
            // We're only interested in "TSTEP" and "DATES" keywords,
            // so we ignore everything else here...
            if (keyword.name() != "TSTEP" && keyword.name() != "DATES")
                continue;

            if (keyword.name() == "DATES") {
                for (size_t recordIndex = 0; recordIndex < keyword.size(); recordIndex++) {
                    const auto &record = keyword.getRecord(recordIndex);
                    const std::time_t nextTime = TimeMap::timeFromEclipse(record);
                    if (nextTime == restart_time)
                        skip = false;

                    if (!skip)
                        addTime(nextTime);
                }

                continue;
            }

            if (skip)
                continue;

            addFromTSTEPKeyword(keyword);
        }

        /*
          There is a coupling between the presence of the SKIPREST keyword and
          the restart argument: The restart argument indicates whether this is
          deck should be parsed as restarted deck. If m_restart_offset == 0 we do
          not interpret this as restart situation and the presence of SKIPREST
          is ignored. In the opposite case we verify - post loading - that we
          have actually located the restart date - otherwise "something is
          broken".
        */
        if (this->m_restart_offset != 0) {
            if (skiprest) {
                const auto iter = std::find(this->m_timeList.begin(), this->m_timeList.end(), restart_time);
                if (iter == this->m_timeList.end()) {
                    TimeStampUTC ts(restart_time);
                    throw std::invalid_argument("Could not find restart date " + std::to_string(ts.year()) + "-" + std::to_string(ts.month()) + "-" + std::to_string(ts.day()));
                }
                this->m_skiprest = true;
            }
        }
    }

    TimeMap TimeMap::serializeObject()
    {
        TimeMap result({123});
        result.m_skiprest = true;
        result.m_restart_offset = 4;

        return result;
    }


    size_t TimeMap::numTimesteps() const {
        return m_timeList.size() - 1;
    }


    std::time_t TimeMap::getStartTime(size_t tStepIdx) const {
        return this->operator[](tStepIdx);
    }

    std::time_t TimeMap::getEndTime() const {
        return this->operator[]( this->size( ) - 1);
    }


    double TimeMap::seconds(size_t timeStep) const {
        return std::difftime( this->operator[](timeStep), this->operator[](0));
    }

    double TimeMap::getTotalTime() const
    {
        if (m_timeList.size() < 2)
            return 0.0;

        return std::difftime(this->m_timeList.back(),
                             this->m_timeList.front());
    }

    void TimeMap::addTime(std::time_t newTime) {
        const std::time_t lastTime = m_timeList.back();
        const size_t step = m_timeList.size();
        if (newTime > lastTime) {
            const auto nw   = TimeStampUTC{ newTime };
            const auto last = TimeStampUTC{ lastTime };

            const auto new_month  = nw  .month();
            const auto last_month = last.month();

            const auto new_year  = nw  .year();
            const auto last_year = last.year();

            if (new_month != last_month || new_year != last_year)
                m_first_timestep_months.push_back({step, nw});

            if (new_year != last_year)
                m_first_timestep_years.push_back({step, nw});

            m_timeList.push_back(newTime);
        } else
            throw std::invalid_argument("Times added must be in strictly increasing order.");
    }

    void TimeMap::addTStep(int64_t step) {
        addTime(forward(m_timeList.back(), step));
    }

    size_t TimeMap::size() const {
        return m_timeList.size();
    }

    size_t TimeMap::last() const {
        return this->numTimesteps();
    }

    const std::map<std::string , int>& TimeMap::eclipseMonthIndices() {
        return month_indices;
    }


    std::time_t TimeMap::timeFromEclipse(const DeckRecord &dateRecord) {
        const auto &dayItem = dateRecord.getItem(0);
        const auto &monthItem = dateRecord.getItem(1);
        const auto &yearItem = dateRecord.getItem(2);
        const auto &timeItem = dateRecord.getItem(3);

        int hour = 0, min = 0, second = 0;
        if (timeItem.hasValue(0)) {
            if (sscanf(timeItem.get<std::string>(0).c_str(), "%d:%d:%d" , &hour,&min,&second) != 3) {
                hour = min = second = 0;
            }
        }

        // Accept lower- and mixed-case month names.
        std::string monthname = uppercase(monthItem.get<std::string>(0));

        std::time_t date = mkdatetime(yearItem.get<int>(0),
                                      eclipseMonthIndices().at(monthname),
                                      dayItem.get<int>(0),
                                      hour,
                                      min,
                                      second);
        return date;
    }


    void TimeMap::addFromTSTEPKeyword(const DeckKeyword &TSTEPKeyword) {
        if (TSTEPKeyword.name() != "TSTEP")
            throw std::invalid_argument("Method requires TSTEP keyword input.");
        {
            const auto &item = TSTEPKeyword.getRecord(0).getItem(0);

            for (size_t itemIndex = 0; itemIndex < item.data_size(); itemIndex++) {
                const int64_t seconds = static_cast<int64_t>(item.getSIDouble(itemIndex));
                addTStep(seconds);
            }
        }
    }

    double TimeMap::getTimeStepLength(size_t tStepIdx) const
    {
        assert(tStepIdx < numTimesteps());

        return std::difftime(this->m_timeList[tStepIdx + 1],
                             this->m_timeList[tStepIdx + 0]);
    }

    double TimeMap::getTimePassedUntil(size_t tLevelIdx) const
    {
        assert(tLevelIdx < m_timeList.size());

        return std::difftime(this->m_timeList[tLevelIdx],
                             this->m_timeList.front());
    }

    const std::vector<std::time_t>& TimeMap::timeList() const
    {
        return m_timeList;
    }


    bool TimeMap::operator==(const TimeMap& data) const
    {
        return this->m_timeList == data.m_timeList &&
               this->m_first_timestep_months == data.m_first_timestep_months &&
               this->m_first_timestep_years == data.m_first_timestep_years &&
               this->m_restart_offset == data.m_restart_offset;
    }

    bool TimeMap::isTimestepInFirstOfMonthsYearsSequence(size_t timestep, bool years, size_t start_timestep, size_t frequency) const {
        bool timestep_first_of_month_year = false;
        const auto& timesteps = (years) ? m_first_timestep_years : m_first_timestep_months;

        auto same_step = [timestep](const StepData& sd) { return sd.stepnumber == timestep; };
        auto ci_timestep = std::find_if(timesteps.begin(), timesteps.end(), same_step);
        if (ci_timestep != timesteps.end() && ci_timestep != timesteps.begin()) {
            if (1 >= frequency) {
                timestep_first_of_month_year = true;
            } else { //Frequency given
                timestep_first_of_month_year = isTimestepInFreqSequence(timestep, start_timestep, frequency, years);
            }
        }
        return timestep_first_of_month_year;
    }


    // Return true if the step is the first of each n-month or n-month
    // period, starting from start_timestep - 1, with n = frequency.
    bool TimeMap::isTimestepInFreqSequence (size_t timestep, size_t start_timestep, size_t frequency, bool years) const {
        // Find iterator to data for 'start_timestep' or first
        // in-sequence step following it, set start_year and
        // start_month.
        const auto& timesteps = (years) ? m_first_timestep_years : m_first_timestep_months;
        auto compare_stepnumber = [](const StepData& sd, size_t value) { return sd.stepnumber < value; };
        auto ci_start_timestep = std::lower_bound(timesteps.begin(), timesteps.end(), start_timestep - 1, compare_stepnumber);
        if (ci_start_timestep == timesteps.end()) {
            // We are after the end of the sequence.
            return false;
        }
        const int start_year = ci_start_timestep->timestamp.year();
        const int start_month = ci_start_timestep->timestamp.month() - 1; // For 0-indexing.

        // Find iterator to data for 'timestep'.
        auto same_step = [timestep](const StepData& sd) { return sd.stepnumber == timestep; };
        auto ci_timestep = std::find_if(timesteps.begin(), timesteps.end(), same_step);
        // The ci_timestep can be assumed to be different from
        // timesteps.end(), or we would not be in this function.
        // If, however, it is at or before the first timestep we should
        // always return false.
        if (ci_timestep <= ci_start_timestep) {
            return false;
        }

        if (years) {
            // Year logic.
            const int my_year = ci_timestep->timestamp.year();
            if ((my_year - start_year) % frequency == 0) {
                return true;
            } else {
                // Check if we are in a new (frequency-year) period.
                const auto prev_it = ci_timestep - 1;
                const int prev_year = prev_it->timestamp.year();
                return (my_year - start_year)/frequency > (prev_year - start_year)/frequency;
            }
        } else {
            // Month logic.
            const int my_year = ci_timestep->timestamp.year();
            const int my_month = (my_year - start_year) * 12 + ci_timestep->timestamp.month() - 1;
            // my_month is now the count of months since start_month.
            assert(my_month > start_month);
            if ((my_month - start_month) % frequency == 0) {
                return true;
            } else {
                // Check if we are in a new (frequency-month) period.
                const auto prev_it = ci_timestep - 1;
                const int prev_year = prev_it->timestamp.year();
                const int prev_month = (prev_year - start_year) * 12 + prev_it->timestamp.month() - 1;
                return (my_month - start_month)/frequency > (prev_month - start_month)/frequency;
            }
        }
    }


    // vec is assumed to be sorted
    size_t TimeMap::closest(const std::vector<size_t> & vec, size_t value) const
    {
        std::vector<size_t>::const_iterator ci =
            std::lower_bound(vec.begin(), vec.end(), value);
        if (ci != vec.end()) {
            return *ci;
        }
        return 0;
    }


    std::time_t TimeMap::operator[] (size_t index) const {
        if (index >= m_timeList.size())
            throw std::invalid_argument("Index out of range");

        if (index > 0 && index < this->m_restart_offset)
            throw std::invalid_argument("Tried to get time information from the base case in restarted run");

        return m_timeList[index];
    }

    std::time_t TimeMap::mkdate(int in_year, int in_month, int in_day) {
        return mkdatetime(in_year , in_month , in_day, 0,0,0);
    }

    std::time_t TimeMap::mkdatetime(int in_year, int in_month, int in_day, int hour, int minute, int second) {
        const auto tp = TimeStampUTC{ TimeStampUTC::YMD { in_year, in_month, in_day } }
            .hour(hour).minutes(minute).seconds(second);

        std::time_t t = asTimeT(tp);
        {
            /*
               The underlying mktime( ) function will happily wrap
               around dates like January 33, this function will check
               that no such wrap-around has taken place.
            */
            const auto check = TimeStampUTC{ t };
            if ((in_day != check.day()) || (in_month != check.month()) || (in_year != check.year()))
                throw std::invalid_argument("Invalid input arguments for date.");
        }
        return t;
    }

    std::time_t TimeMap::forward(std::time_t t, int64_t seconds) {
        return t + seconds;
    }

    std::time_t TimeMap::forward(std::time_t t, int64_t hours, int64_t minutes, int64_t seconds) {
        return t + seconds + minutes * 60 + hours * 3600;
    }

    std::size_t TimeMap::restart_offset() const {
        return this->m_restart_offset;
    }

    bool TimeMap::skiprest() const {
        return this->m_skiprest;
    }

std::ostream& operator<<(std::ostream& stream, const TimeMap& tm) {
    std::stringstream ss;
    ss << "{";
    std::size_t index = 0;
    for (const auto& tp : tm.timeList()) {
        auto ts = TimeStampUTC(tp);
        ss << ts.year() << "-" << std::setfill('0') << std::setw(2) << ts.month() << "-" << std::setfill('0') << std::setw(2) << ts.day();
        index += 1;
        if (index < tm.timeList().size())
            ss << ", ";
        if (index % 12 == 0)
            ss << std::endl;
    }
    ss << "}";
    return stream << ss.str();
}
}



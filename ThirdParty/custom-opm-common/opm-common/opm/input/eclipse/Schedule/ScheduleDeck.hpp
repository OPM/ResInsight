/*
  Copyright 2021 Equinor ASA.

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
#ifndef SCHEDULE_DECK_HPP
#define SCHEDULE_DECK_HPP

#include <chrono>
#include <cstddef>
#include <optional>
#include <ostream>
#include <vector>

#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/io/eclipse/rst/state.hpp>

namespace Opm {

    enum class ScheduleTimeType {
        START = 0,
        DATES = 1,
        TSTEP = 2,
        RESTART = 3
    };


    class Deck;
    class DeckOutput;
    struct ScheduleDeckContext;
    class Runspec;

    /*
      The ScheduleBlock is collection of all the Schedule keywords from one
      report step.
    */

    class ScheduleBlock {
    public:
        ScheduleBlock() = default;
        ScheduleBlock(const KeywordLocation& location, ScheduleTimeType time_type, const time_point& start_time);
        std::size_t size() const;
        void push_back(const DeckKeyword& keyword);
        std::optional<DeckKeyword> get(const std::string& kw) const;
        const time_point& start_time() const;
        const std::optional<time_point>& end_time() const;
        void end_time(const time_point& t);
        ScheduleTimeType time_type() const;
        const KeywordLocation& location() const;
        const DeckKeyword& operator[](const std::size_t index) const;
        std::vector<DeckKeyword>::const_iterator begin() const;
        std::vector<DeckKeyword>::const_iterator end() const;

        bool operator==(const ScheduleBlock& other) const;
        static ScheduleBlock serializeObject();
        template<class Serializer>
        void serializeOp(Serializer& serializer) {
            serializer(m_time_type);
            serializer(m_start_time);
            serializer(m_end_time);
            serializer.vector(m_keywords);
            m_location.serializeOp(serializer);
        }

        void dump_time(time_point current_time, DeckOutput& output) const;
        void dump_deck(DeckOutput& output, time_point& current_time) const;
    private:
        ScheduleTimeType m_time_type;
        time_point m_start_time;
        std::optional<time_point> m_end_time;
        KeywordLocation m_location;
        std::vector<DeckKeyword> m_keywords;
    };


    struct ScheduleRestartInfo {
        std::time_t time{0};
        std::size_t report_step{0};
        bool skiprest{false};

        ScheduleRestartInfo() = default;

        ScheduleRestartInfo(const RestartIO::RstState * rst, const Deck& deck);
        bool operator==(const ScheduleRestartInfo& other) const;
        static ScheduleRestartInfo serializeObject();

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(this->time);
            serializer(this->report_step);
            serializer(this->skiprest);
        }
    };




    /*
      The purpose of the ScheduleDeck class is to serve as a container holding
      all the keywords of the SCHEDULE section, when the Schedule class is
      assembled that is done by iterating over the contents of the ScheduleDeck.
      The ScheduleDeck class can be indexed with report step through operator[].
      Internally the ScheduleDeck class is a vector of ScheduleBlock instances -
      one for each report step.
    */

    class ScheduleDeck {
    public:
        explicit ScheduleDeck(time_point start_time, const Deck& deck, const ScheduleRestartInfo& rst_info);
        ScheduleDeck();
        void add_block(ScheduleTimeType time_type, const time_point& t, ScheduleDeckContext& context, const KeywordLocation& location);
        void add_TSTEP(const DeckKeyword& TSTEPKeyword, ScheduleDeckContext& context);
        ScheduleBlock& operator[](const std::size_t index);
        const ScheduleBlock& operator[](const std::size_t index) const;
        std::vector<ScheduleBlock>::const_iterator begin() const;
        std::vector<ScheduleBlock>::const_iterator end() const;
        std::size_t size() const;
        std::size_t restart_offset() const;
        const KeywordLocation& location() const;
        double seconds(std::size_t timeStep) const;

        bool operator==(const ScheduleDeck& other) const;
        static ScheduleDeck serializeObject();
        template<class Serializer>
        void serializeOp(Serializer& serializer) {
            serializer(m_restart_time);
            serializer(m_restart_offset);
            serializer(skiprest);
            serializer.vector(m_blocks);
            m_location.serializeOp(serializer);
        }

        void dump_deck(std::ostream& os) const;

    private:
        time_point m_restart_time;
        std::size_t m_restart_offset;
        bool skiprest;
        KeywordLocation m_location;
        std::vector<ScheduleBlock> m_blocks;
    };
}

#endif

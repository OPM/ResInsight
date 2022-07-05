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

#include <opm/input/eclipse/Schedule/ScheduleDeck.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/utility/String.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Deck/DeckOutput.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>

#include <chrono>
#include <ctime>
#include <unordered_set>

#include <fmt/format.h>
#include <fmt/chrono.h>

namespace Opm {

ScheduleRestartInfo::ScheduleRestartInfo(const RestartIO::RstState * rst, const Deck& deck) {
    if (rst) {
        const auto& [t,r] = rst->header.restart_info();
        this->time = t;
        this->report_step = r;
        this->skiprest = deck.hasKeyword<ParserKeywords::SKIPREST>();
    }
}


bool ScheduleRestartInfo::operator==(const ScheduleRestartInfo& other) const {
    return this->time == other.time &&
        this->report_step == other.report_step &&
        this->skiprest == other.skiprest;
}


ScheduleRestartInfo ScheduleRestartInfo::serializeObject() {
    ScheduleRestartInfo rst_info;
    rst_info.report_step = 12345;
    rst_info.skiprest = false;
    return rst_info;
}




ScheduleBlock::ScheduleBlock(const KeywordLocation& location, ScheduleTimeType time_type, const time_point& start_time) :
    m_time_type(time_type),
    m_start_time(start_time),
    m_location(location)
{
}

std::size_t ScheduleBlock::size() const {
    return this->m_keywords.size();
}

void ScheduleBlock::push_back(const DeckKeyword& keyword) {
    this->m_keywords.push_back(keyword);
}

std::vector<DeckKeyword>::const_iterator ScheduleBlock::begin() const {
    return this->m_keywords.begin();
}

std::vector<DeckKeyword>::const_iterator ScheduleBlock::end() const {
    return this->m_keywords.end();
}

const DeckKeyword& ScheduleBlock::operator[](const std::size_t index) const {
    return this->m_keywords.at(index);
}

const time_point& ScheduleBlock::start_time() const {
    return this->m_start_time;
}

const std::optional<time_point>& ScheduleBlock::end_time() const {
    return this->m_end_time;
}

ScheduleTimeType ScheduleBlock::time_type() const {
    return this->m_time_type;
}

void ScheduleBlock::end_time(const time_point& t) {
    this->m_end_time = t;
}

bool ScheduleBlock::operator==(const ScheduleBlock& other) const {
    return this->m_start_time == other.m_start_time &&
           this->m_end_time == other.m_end_time &&
           this->m_location == other.m_location &&
           this->m_time_type == other.m_time_type &&
           this->m_keywords == other.m_keywords;
}

void ScheduleBlock::dump_time(time_point current_time, DeckOutput& output) const {
    if (this->m_time_type == ScheduleTimeType::START)
        return;

    if (this->m_time_type == ScheduleTimeType::DATES) {
        TimeStampUTC ts(TimeService::to_time_t(this->start_time()));
        auto ecl_month = TimeService::eclipseMonthNames().at(ts.month());
        std::string dates_string = fmt::format(R"(
DATES
   {} '{}' {} /
/
)", ts.day(), ecl_month, ts.year());
        output.write_string(dates_string);
    } else {
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(this->start_time() - current_time);
        double days = seconds.count() / 86400.0;
        std::string tstep_string = fmt::format(R"(
TSTEP
   {} /
)", days);
        output.write_string(tstep_string);
    }
}


void ScheduleBlock::dump_deck(DeckOutput& output, time_point& current_time) const {
    this->dump_time(current_time, output);
    if (!this->end_time().has_value())
        return;

    for (const auto& keyword : this->m_keywords)
        keyword.write(output);

    current_time = this->end_time().value();
}


const KeywordLocation& ScheduleBlock::location() const {
    return this->m_location;
}


ScheduleBlock ScheduleBlock::serializeObject() {
    ScheduleBlock block;
    block.m_time_type = ScheduleTimeType::TSTEP;
    block.m_start_time = TimeService::from_time_t( asTimeT( TimeStampUTC( 2003, 10, 10 )));
    block.m_end_time = TimeService::from_time_t( asTimeT( TimeStampUTC( 1993, 07, 06 )));
    block.m_location = KeywordLocation::serializeObject();
    block.m_keywords = {DeckKeyword::serializeObject()};
    return block;
}

std::optional<DeckKeyword> ScheduleBlock::get(const std::string& kw) const {
    for (const auto& keyword : this->m_keywords) {
        if (keyword.name() == kw)
            return keyword;
    }
    return {};
}

/*****************************************************************************/

struct ScheduleDeckContext {
    bool rst_skip;
    time_point last_time;

    ScheduleDeckContext(bool skip, time_point t) :
        rst_skip(skip),
        last_time(t)
    {}
};


const KeywordLocation& ScheduleDeck::location() const {
    return this->m_location;
}


std::size_t ScheduleDeck::restart_offset() const {
    return this->m_restart_offset;
}


ScheduleDeck::ScheduleDeck(time_point start_time, const Deck& deck, const ScheduleRestartInfo& rst_info) {
    const std::unordered_set<std::string> skiprest_include = {"VFPPROD", "VFPINJ", "RPTSCHED", "RPTRST", "TUNING", "MESSAGES"};

    this->m_restart_time = TimeService::from_time_t(rst_info.time);
    this->m_restart_offset = rst_info.report_step;
    this->skiprest = rst_info.skiprest;
    if (this->m_restart_offset > 0) {
        for (std::size_t it = 0; it < this->m_restart_offset; it++) {
            if (it == 0)
                this->m_blocks.emplace_back(KeywordLocation{}, ScheduleTimeType::START, start_time);
            else
                this->m_blocks.emplace_back(KeywordLocation{}, ScheduleTimeType::RESTART, start_time);
            this->m_blocks.back().end_time(start_time);
        }
        if (!this->skiprest) {
            this->m_blocks.back().end_time(this->m_restart_time);
            this->m_blocks.emplace_back(KeywordLocation{}, ScheduleTimeType::RESTART, this->m_restart_time);
        }
    } else
        this->m_blocks.emplace_back(KeywordLocation{}, ScheduleTimeType::START, start_time);

    ScheduleDeckContext context(this->skiprest, this->m_blocks.back().start_time());
    for( const auto& keyword : SCHEDULESection(deck)) {
        if (keyword.name() == "DATES") {
            for (size_t recordIndex = 0; recordIndex < keyword.size(); recordIndex++) {
                const auto &record = keyword.getRecord(recordIndex);
                std::time_t nextTime;

                try {
                    nextTime = TimeService::timeFromEclipse(record);
                } catch (const std::exception& e) {
                    const OpmInputError opm_error { e, keyword.location() } ;
                    OpmLog::error(opm_error.what());
                    std::throw_with_nested(opm_error);
                }

                this->add_block(ScheduleTimeType::DATES, TimeService::from_time_t( nextTime ), context, keyword.location());
            }
            continue;
        }
        if (keyword.name() == "TSTEP") {
            this->add_TSTEP(keyword, context);
            continue;
        }

        if (keyword.name() == "SCHEDULE") {
            this->m_location = keyword.location();
            continue;
        }

        if (context.rst_skip) {
            if (skiprest_include.count(keyword.name()) != 0)
                this->m_blocks[0].push_back(keyword);
        } else
            this->m_blocks.back().push_back(keyword);
    }
}

namespace {

    std::string
    format_skiprest_error(const ScheduleTimeType time_type,
                          const time_point&      restart_time,
                          const time_point&      t)
    {
        const auto rst = TimeStampUTC {
            TimeService::to_time_t(restart_time)
        };

        const auto current = TimeStampUTC {
            TimeService::to_time_t(t)
        };

        auto rst_tm = std::tm{};
        rst_tm.tm_year = rst.year()  - 1900;
        rst_tm.tm_mon  = rst.month() -    1;
        rst_tm.tm_mday = rst.day();

        rst_tm.tm_hour = rst.hour();
        rst_tm.tm_min  = rst.minutes();
        rst_tm.tm_sec  = rst.seconds();

        auto current_tm = std::tm{};
        current_tm.tm_year = current.year()  - 1900;
        current_tm.tm_mon  = current.month() -    1;
        current_tm.tm_mday = current.day();

        current_tm.tm_hour = current.hour();
        current_tm.tm_min  = current.minutes();
        current_tm.tm_sec  = current.seconds();

        const auto* keyword = (time_type == ScheduleTimeType::DATES)
            ? "DATES" : "TSTEP";
        const auto* record = (time_type == ScheduleTimeType::DATES)
            ? "record" : "report step";

        return fmt::format("In a restarted simulation using SKIPREST, the {0} keyword must have\n"
                           "a {1} corresponding to the RESTART time {2:%d-%b-%Y %H:%M:%S}.\n"
                           "Reached time {3:%d-%b-%Y %H:%M:%S} without an intervening {1}.",
                           keyword, record, rst_tm, current_tm);
    }
}

void ScheduleDeck::add_block(ScheduleTimeType time_type, const time_point& t, ScheduleDeckContext& context, const KeywordLocation& location) {
    context.last_time = t;
    if (context.rst_skip) {
        if (t < this->m_restart_time)
            return;

        if (t == this->m_restart_time)
            context.rst_skip = false;

        if (t > this->m_restart_time) {
            if (this->skiprest) {
                const auto reason =
                    format_skiprest_error(time_type, this->m_restart_time, t);

                throw OpmInputError(reason, location);
            }
            context.rst_skip = false;
        }
    }
    this->m_blocks.back().end_time(t);
    this->m_blocks.emplace_back( location, time_type, t );
}


void ScheduleDeck::add_TSTEP(const DeckKeyword& TSTEPKeyword, ScheduleDeckContext& context) {
    const auto &item = TSTEPKeyword.getRecord(0).getItem(0);
    for (size_t itemIndex = 0; itemIndex < item.data_size(); itemIndex++) {
        auto next_time = context.last_time + std::chrono::duration_cast<time_point::duration>(std::chrono::duration<double>(item.getSIDouble(itemIndex)));
        this->add_block(ScheduleTimeType::TSTEP, next_time, context, TSTEPKeyword.location());
    }
}


double ScheduleDeck::seconds(std::size_t timeStep) const {
    if (this->m_blocks.empty())
        return 0;

    if (timeStep >= this->m_blocks.size())
        throw std::logic_error(fmt::format("seconds({}) - invalid timeStep. Valid range [0,{}>", timeStep, this->m_blocks.size()));

    std::chrono::duration<double> elapsed = this->m_blocks[timeStep].start_time() - this->m_blocks[0].start_time();
    return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
}


ScheduleDeck::ScheduleDeck() {
    time_point start_time;
    this->m_blocks.emplace_back(KeywordLocation{}, ScheduleTimeType::START, start_time);
}


ScheduleBlock& ScheduleDeck::operator[](const std::size_t index) {
    return this->m_blocks.at(index);
}

const ScheduleBlock& ScheduleDeck::operator[](const std::size_t index) const {
    return this->m_blocks.at(index);
}

std::size_t ScheduleDeck::size() const {
    return this->m_blocks.size();
}

std::vector<ScheduleBlock>::const_iterator ScheduleDeck::begin() const {
    return this->m_blocks.begin();
}

std::vector<ScheduleBlock>::const_iterator ScheduleDeck::end() const {
    return this->m_blocks.end();
}


bool ScheduleDeck::operator==(const ScheduleDeck& other) const {
    return this->m_restart_time == other.m_restart_time &&
           this->m_restart_offset == other.m_restart_offset &&
           this->m_blocks == other.m_blocks;
}

ScheduleDeck ScheduleDeck::serializeObject() {
    ScheduleDeck deck;
    deck.m_restart_time = TimeService::from_time_t( asTimeT( TimeStampUTC( 2013, 12, 12 )));
    deck.m_restart_offset = 123;
    deck.m_location = KeywordLocation::serializeObject();
    deck.m_blocks = { ScheduleBlock::serializeObject(), ScheduleBlock::serializeObject() };
    return deck;
}

void ScheduleDeck::dump_deck(std::ostream& os) const {
    DeckOutput output(os);

    output.write_string("SCHEDULE\n");
    auto current_time = this->m_blocks[0].start_time();
    for (const auto& block : this->m_blocks)
        block.dump_deck(output, current_time);
}


}

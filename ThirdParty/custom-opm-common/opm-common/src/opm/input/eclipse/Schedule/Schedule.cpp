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

#include <algorithm>
#include <ctime>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/utility/numeric/cmp.hpp>
#include <opm/common/utility/String.hpp>
#include <opm/common/utility/shmatch.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include <opm/io/eclipse/rst/state.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/L.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>

#include <opm/input/eclipse/EclipseState/TracerConfig.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionResult.hpp>
#include <opm/input/eclipse/Schedule/MSW/SICD.hpp>
#include <opm/input/eclipse/Schedule/MSW/Valve.hpp>
#include <opm/input/eclipse/Schedule/MSW/WellSegments.hpp>
#include <opm/input/eclipse/Schedule/Group/GConSump.hpp>
#include <opm/input/eclipse/Schedule/Group/GConSale.hpp>

#include <opm/input/eclipse/Schedule/OilVaporizationProperties.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/RPTConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/ScheduleGrid.hpp>
#include <opm/input/eclipse/Schedule/Tuning.hpp>
#include <opm/input/eclipse/Schedule/Network/Node.hpp>
#include <opm/input/eclipse/Schedule/Well/WList.hpp>
#include <opm/input/eclipse/Schedule/Well/WListManager.hpp>
#include <opm/input/eclipse/Schedule/Well/WellFoamProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellMICPProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellPolymerProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellBrineProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#include "Well/injection.hpp"
#include "MSW/Compsegs.hpp"

namespace {

    bool name_match(const std::string& pattern, const std::string& name) {
        return Opm::shmatch(pattern, name);
    }

    double sumthin_summary_section(const Opm::SUMMARYSection& section) {
        const auto entries = section.getKeywordList<Opm::ParserKeywords::SUMTHIN>();

        // Care only about the last SUMTHIN entry in the SUMMARY
        // section if keyword is present here at all.
        return entries.empty()
            ? -1.0 // (<= 0.0)
            : entries.back()->getRecord(0).getItem(0).getSIDouble(0);
    }

    bool rptonly_summary_section(const Opm::SUMMARYSection& section) {
        auto rptonly = false;

        using On = Opm::ParserKeywords::RPTONLY;
        using Off = Opm::ParserKeywords::RPTONLYO;

        // Last on/off keyword entry "wins".
        for (const auto& keyword : section) {
            if (keyword.is<On>())
                rptonly = true;
            else if (keyword.is<Off>())
                rptonly = false;
        }

        return rptonly;
    }
}

namespace Opm {

    ScheduleStatic::ScheduleStatic(std::shared_ptr<const Python> python_handle,
                                   const ScheduleRestartInfo& restart_info,
                                   const Deck& deck,
                                   const Runspec& runspec,
                                   const std::optional<int>& output_interval_,
                                   const ParseContext& parseContext,
                                   ErrorGuard& errors) :
        m_python_handle(python_handle),
        m_input_path(deck.getInputPath()),
        rst_info(restart_info),
        m_deck_message_limits( deck ),
        m_unit_system( deck.getActiveUnitSystem() ),
        m_runspec( runspec ),
        rst_config( SOLUTIONSection(deck), parseContext, errors ),
        output_interval(output_interval_),
        sumthin(sumthin_summary_section(SUMMARYSection{ deck })),
        rptonly(rptonly_summary_section(SUMMARYSection{ deck })),
        gaslift_opt_active(deck.hasKeyword<ParserKeywords::LIFTOPT>())
    {
    }

    Schedule::Schedule( const Deck& deck,
                        const EclipseGrid& ecl_grid,
                        const FieldPropsManager& fp,
                        const Runspec &runspec,
                        const ParseContext& parseContext,
                        ErrorGuard& errors,
                        std::shared_ptr<const Python> python,
                        const std::optional<int>& output_interval,
                        const RestartIO::RstState * rst,
                        const TracerConfig * tracer_config)
    try :
        m_static( python, ScheduleRestartInfo(rst, deck), deck, runspec, output_interval, parseContext, errors ),
        m_sched_deck(TimeService::from_time_t(runspec.start_time()), deck, m_static.rst_info ),
        completed_cells(ecl_grid.getNX(), ecl_grid.getNY(), ecl_grid.getNZ())
    {
        this->restart_output.resize(this->m_sched_deck.size());
        this->restart_output.clearRemainingEvents(0);

        //const ScheduleGridWrapper gridWrapper { grid } ;
        ScheduleGrid grid(ecl_grid, fp, this->completed_cells);

        if (rst) {
            if (!tracer_config)
                throw std::logic_error("Bug: when loading from restart a valid TracerConfig object must be supplied");

            auto restart_step = this->m_static.rst_info.report_step;
            this->iterateScheduleSection( 0, restart_step, parseContext, errors, grid, nullptr, "");
            this->load_rst(*rst, *tracer_config, grid, fp);
            if (! this->restart_output.writeRestartFile(restart_step))
                this->restart_output.addRestartOutput(restart_step);
            this->iterateScheduleSection( restart_step, this->m_sched_deck.size(), parseContext, errors, grid, nullptr, "");
        } else {
            this->iterateScheduleSection( 0, this->m_sched_deck.size(), parseContext, errors, grid, nullptr, "");
        }

        //m_grid = std::make_shared<SparseScheduleGrid>(grid, gridWrapper.getHitKeys());
    }
    catch (const OpmInputError& opm_error) {
        OpmLog::error(opm_error.what());
        throw;
    }
    catch (const std::exception& std_error) {
        OpmLog::error(fmt::format("An error occurred while creating the reservoir schedule\n"
                                  "Internal error: {}", std_error.what()));
        throw;
    }




    template <typename T>
    Schedule::Schedule( const Deck& deck,
                        const EclipseGrid& grid,
                        const FieldPropsManager& fp,
                        const Runspec &runspec,
                        const ParseContext& parseContext,
                        T&& errors,
                        std::shared_ptr<const Python> python,
                        const std::optional<int>& output_interval,
                        const RestartIO::RstState * rst,
                        const TracerConfig* tracer_config) :
        Schedule(deck, grid, fp, runspec, parseContext, errors, python, output_interval, rst, tracer_config)
    {}


    Schedule::Schedule( const Deck& deck,
                        const EclipseGrid& grid,
                        const FieldPropsManager& fp,
                        const Runspec &runspec,
                        std::shared_ptr<const Python> python,
                        const std::optional<int>& output_interval,
                        const RestartIO::RstState * rst,
                        const TracerConfig* tracer_config) :
        Schedule(deck, grid, fp, runspec, ParseContext(), ErrorGuard(), python, output_interval, rst, tracer_config)
    {}


    Schedule::Schedule(const Deck& deck, const EclipseState& es, const ParseContext& parse_context, ErrorGuard& errors, std::shared_ptr<const Python> python, const std::optional<int>& output_interval, const RestartIO::RstState * rst) :
        Schedule(deck,
                 es.getInputGrid(),
                 es.fieldProps(),
                 es.runspec(),
                 parse_context,
                 errors,
                 python,
                 output_interval,
                 rst,
                 &es.tracer())
    {}


    template <typename T>
    Schedule::Schedule(const Deck& deck, const EclipseState& es, const ParseContext& parse_context, T&& errors, std::shared_ptr<const Python> python, const std::optional<int>& output_interval, const RestartIO::RstState * rst) :
        Schedule(deck,
                 es.getInputGrid(),
                 es.fieldProps(),
                 es.runspec(),
                 parse_context,
                 errors,
                 python,
                 output_interval,
                 rst,
                 &es.tracer())
    {}


Schedule::Schedule(const Deck& deck, const EclipseState& es, std::shared_ptr<const Python> python, const std::optional<int>& output_interval, const RestartIO::RstState * rst) :
    Schedule(deck, es, ParseContext(), ErrorGuard(), python, output_interval, rst)
{}


Schedule::Schedule(const Deck& deck, const EclipseState& es, const std::optional<int>& output_interval, const RestartIO::RstState * rst) :
    Schedule(deck, es, ParseContext(), ErrorGuard(), std::make_shared<const Python>(), output_interval, rst)
    {}

    Schedule::Schedule(std::shared_ptr<const Python> python_handle) :
        m_static( python_handle )
    {
    }

    /*
      In general the serializeObject() instances are used as targets for
      deserialization, i.e. the serialized buffer is unpacked into this
      instance. However the Schedule object is a top level object, and the
      simulator will instantiate and manage a Schedule object to unpack into, so
      the instance created here is only for testing.
    */
    Schedule Schedule::serializeObject()
    {
        Schedule result;

        result.m_static = ScheduleStatic::serializeObject();
        result.snapshots = { ScheduleState::serializeObject() };
        result.m_sched_deck = ScheduleDeck::serializeObject();
        result.restart_output = WriteRestartFileEvents::serializeObject();

        return result;
    }

    std::time_t Schedule::getStartTime() const {
        return this->posixStartTime( );
    }

    time_t Schedule::posixStartTime() const {
        return std::chrono::system_clock::to_time_t(this->m_sched_deck[0].start_time());
    }

    time_t Schedule::posixEndTime() const {
        // This should indeed access the start_time() property of the last
        // snapshot.
        return std::chrono::system_clock::to_time_t(this->snapshots.back().start_time());
    }


    void Schedule::handleKeyword(std::size_t currentStep,
                                 const ScheduleBlock& block,
                                 const DeckKeyword& keyword,
                                 const ParseContext& parseContext,
                                 ErrorGuard& errors,
                                 const ScheduleGrid& grid,
                                 const std::vector<std::string>& matching_wells,
                                 bool actionx_mode,
                                 SimulatorUpdate * sim_update,
                                 const std::unordered_map<std::string, double> * target_wellpi) {

        static const std::unordered_set<std::string> require_grid = {
            "COMPDAT",
            "COMPSEGS"
        };


        HandlerContext handlerContext { block, keyword, grid, currentStep, matching_wells, actionx_mode, parseContext, errors, sim_update, target_wellpi};
        /*
          The grid and fieldProps members create problems for reiterating the
          Schedule section. We therefor single them out very clearly here.
        */
        if (handleNormalKeyword(handlerContext))
            return;

        if (keyword.is<ParserKeywords::PYACTION>())
            handlePYACTION(keyword);
    }

namespace {

class ScheduleLogger {
public:
    ScheduleLogger(bool restart_skip, const std::string& prefix_, const KeywordLocation& location)
        : prefix(prefix_)
        , current_file(location.filename)
    {
        if (restart_skip)
            this->log_function = &OpmLog::note;
        else
            this->log_function = &OpmLog::info;
    }

    void operator()(const std::string& msg) {
        this->log_function(this->prefix + msg);
    }

    void info(const std::string& msg) {
        OpmLog::info(this->prefix + msg);
    }

    void info(const std::vector<std::string>& msg_list) {
        for (const auto& msg : msg_list)
            this->info(msg);
    }

    void complete_step(const std::string& msg) {
        this->step_count += 1;
        if (this->step_count == this->max_print) {
            this->log_function(this->prefix + msg);
            this->info(std::vector<std::string>{"Report limit reached, see PRT-file for remaining Schedule initialization.", ""});
            this->log_function = &OpmLog::note;
        } else {
            this->log_function( this->prefix + msg );
            this->log_function( this->prefix );
        }
    };

    void restart() {
        this->step_count = 0;
        this->log_function = &OpmLog::info;
    }

    void location(const KeywordLocation& location) {
        if (this->current_file == location.filename)
            return;

        this->operator()( fmt::format("Reading from: {} line {}", location.filename, location.lineno) );
        this->current_file = location.filename;
    }


private:
    std::size_t step_count = 0;
    std::size_t max_print  = 5;
    std::string prefix;
    std::string current_file;
    void (*log_function)(const std::string&);
};

}

void Schedule::iterateScheduleSection(std::size_t load_start, std::size_t load_end,
                                      const ParseContext& parseContext ,
                                      ErrorGuard& errors,
                                      const ScheduleGrid& grid,
                                      const std::unordered_map<std::string, double> * target_wellpi,
                                      const std::string& prefix) {

        std::vector<std::pair< const DeckKeyword* , std::size_t> > rftProperties;
        std::string time_unit = this->m_static.m_unit_system.name(UnitSystem::measure::time);
        auto deck_time = [this](double seconds) { return this->m_static.m_unit_system.from_si(UnitSystem::measure::time, seconds); };
        /*
          The keywords in the skiprest_whitelist set are loaded from the
          SCHEDULE section even though the SKIPREST keyword is in action. The
          full list includes some additional keywords which we do not support at
          all.
        */
        std::unordered_set<std::string> skiprest_whitelist = {"VFPPROD", "VFPINJ", "RPTSCHED", "RPTRST", "TUNING", "MESSAGES"};
        /*
          The behavior of variable restart_skip is more lenient than the
          SKIPREST keyword. If this is a restarted[1] run the loop iterating
          over keywords will skip the all keywords[2] until DATES keyword with
          the restart date is encountered - irrespective of whether the SKIPREST
          keyword is present in the deck or not.

          [1]: opm/flow can restart in a mode where all the keywords from the
               historical part of the Schedule section is internalized, and only
               the solution fields are read from the restart file. In this case
               we will have Schedule::restart_offset() == 0.

          [2]: With the exception of the keywords in the skiprest_whitelist;
               these keywords will be assigned to report step 0.
        */

        auto restart_skip = load_start < this->m_static.rst_info.report_step;
        ScheduleLogger logger(restart_skip, prefix, this->m_sched_deck.location());
        {
            const auto& location = this->m_sched_deck.location();
            logger.info({"", "Processing dynamic information from", fmt::format("{} line {}", location.filename, location.lineno)});
            if (restart_skip)
                logger.info(fmt::format("This is a restarted run - skipping until report step {} at {}", this->m_static.rst_info.report_step, Schedule::formatDate(this->m_static.rst_info.time)));

            logger(fmt::format("Initializing report step {}/{} at {} {} {} line {}",
                               load_start,
                               this->m_sched_deck.size() - 1,
                               Schedule::formatDate(this->getStartTime()),
                               deck_time(this->m_sched_deck.seconds(load_start)),
                               time_unit,
                               location.lineno));
        }

        for (auto report_step = load_start; report_step < load_end; report_step++) {
            std::size_t keyword_index = 0;
            auto& block = this->m_sched_deck[report_step];
            auto time_type = block.time_type();
            if (time_type == ScheduleTimeType::DATES || time_type == ScheduleTimeType::TSTEP) {
                const auto& start_date = Schedule::formatDate(std::chrono::system_clock::to_time_t(block.start_time()));
                const auto& days = deck_time(this->stepLength(report_step - 1));
                const auto& days_total = deck_time(this->seconds(report_step - 1));
                logger.complete_step(fmt::format("Complete report step {0} ({1} {2}) at {3} ({4} {2})",
                                                 report_step,
                                                 days,
                                                 time_unit,
                                                 start_date,
                                                 days_total));

                if (report_step < (load_end - 1)) {
                    logger.location(block.location());
                    logger(fmt::format("Initializing report step {}/{} at {} ({} {}) line {}",
                                       report_step + 1,
                                       this->m_sched_deck.size() - 1,
                                       start_date,
                                       days_total,
                                       time_unit,
                                       block.location().lineno));
                }
            }
            this->create_next(block);

            while (true) {
                if (keyword_index == block.size())
                    break;

                const auto& keyword = block[keyword_index];
                const auto& location = keyword.location();
                logger.location(keyword.location());

                if (keyword.is<ParserKeywords::ACTIONX>()) {
                    Action::ActionX action(keyword,
                                           this->m_static.m_runspec.actdims(),
                                           std::chrono::system_clock::to_time_t(this->snapshots[report_step].start_time()));
                    while (true) {
                        keyword_index++;
                        if (keyword_index == block.size())
                            throw OpmInputError("Missing keyword ENDACTIO", keyword.location());

                        const auto& action_keyword = block[keyword_index];
                        if (action_keyword.is<ParserKeywords::ENDACTIO>())
                            break;

                        if (Action::ActionX::valid_keyword(action_keyword.name())){
                            action.addKeyword(action_keyword);
                            this->prefetch_cell_properties(grid, action_keyword);
                            this->store_wgnames(action_keyword);
                        }
                        else {
                            std::string msg_fmt = fmt::format("The keyword {} is not supported in the ACTIONX block", action_keyword.name());
                            parseContext.handleError( ParseContext::ACTIONX_ILLEGAL_KEYWORD, msg_fmt, action_keyword.location(), errors);
                        }
                    }
                    this->addACTIONX(action);
                    keyword_index++;
                    continue;
                }

                logger(fmt::format("Processing keyword {} at line {}", location.keyword, location.lineno));
                this->handleKeyword(report_step,
                                    block,
                                    keyword,
                                    parseContext,
                                    errors,
                                    grid,
                                    {},
                                    false,
                                    nullptr,
                                    target_wellpi);
                keyword_index++;
            }

            this->end_report(report_step);

            if (this->must_write_rst_file(report_step)) {
                this->restart_output.addRestartOutput(report_step);
            }
        }
    }

    void Schedule::addACTIONX(const Action::ActionX& action) {
        auto new_actions = this->snapshots.back().actions.get();
        new_actions.add( action );
        this->snapshots.back().actions.update( std::move(new_actions) );
    }


    void Schedule::store_wgnames(const DeckKeyword& keyword) {
        if(keyword.is<ParserKeywords::WELSPECS>()) {
            for (const auto& record : keyword) {
                const auto& wname = record.getItem<ParserKeywords::WELSPECS::WELL>().get<std::string>(0);
                const auto& gname = record.getItem<ParserKeywords::WELSPECS::GROUP>().get<std::string>(0);
                this->action_wgnames.add_well(wname);
                this->action_wgnames.add_group(gname);
            }
        }
    }


    void Schedule::prefetch_cell_properties(const ScheduleGrid& grid, const DeckKeyword& keyword){
        if(keyword.is<ParserKeywords::COMPDAT>()){
            for (auto record : keyword){
                const auto& itemI = record.getItem("I");
                const auto& itemJ = record.getItem("J");
                bool defaulted_I = itemI.defaultApplied(0) || itemI.get<int>(0) == 0;
                bool defaulted_J = itemJ.defaultApplied(0) || itemJ.get<int>(0) == 0;

                if (defaulted_I || defaulted_J)
                    throw std::logic_error(fmt::format("Defaulted grid coordinates is not allowed for COMPDAT as part of ACTIONX"));

                const int I = itemI.get<int>(0) - 1;
                const int J = itemJ.get<int>(0) - 1;
                int K1 = record.getItem("K1").get<int>(0) - 1;
                int K2 = record.getItem("K2").get<int>(0) - 1;

                for (int k = K1; k <= K2; k++){
                    auto cell = grid.get_cell(I, J, k);
                    (void) cell;
                    //Only interested in activating the cells.
                }
            }
            return;
        }

        if (keyword.is<ParserKeywords::COMPSEGS>()) {
            bool first_record = true;
            for (auto record : keyword){
                if (first_record) {
                    first_record = false;
                    continue;
                }
                const auto& itemI = record.getItem("I");
                const auto& itemJ = record.getItem("J");
                const auto& itemK = record.getItem("K");

                const int I = itemI.get<int>(0) - 1;
                const int J = itemJ.get<int>(0) - 1;
                const int K = itemK.get<int>(0) - 1;

                auto cell = grid.get_cell(I, J, K);
                (void) cell;
            }
        }
    }

    void Schedule::handlePYACTION(const DeckKeyword& keyword) {
        if (!this->m_static.m_python_handle->enabled()) {
            //Must have a real Python instance here - to ensure that IMPORT works
            const auto& loc = keyword.location();
            OpmLog::warning("This version of flow is built without support for Python. Keyword PYACTION in file: " + loc.filename + " line: " + std::to_string(loc.lineno) + " is ignored.");
            return;
        }

        const auto& name = keyword.getRecord(0).getItem<ParserKeywords::PYACTION::NAME>().get<std::string>(0);
        const auto& run_count = Action::PyAction::from_string( keyword.getRecord(0).getItem<ParserKeywords::PYACTION::RUN_COUNT>().get<std::string>(0) );
        const auto& module_arg = keyword.getRecord(1).getItem<ParserKeywords::PYACTION::FILENAME>().get<std::string>(0);
        std::string module;
        if (this->m_static.m_input_path.empty())
            module = module_arg;
        else
            module = this->m_static.m_input_path + "/" + module_arg;

        Action::PyAction pyaction(this->m_static.m_python_handle, name, run_count, module);
        auto new_actions = this->snapshots.back().actions.get();
        new_actions.add(pyaction);
        this->snapshots.back().actions.update( std::move(new_actions) );
    }

    void Schedule::applyEXIT(const DeckKeyword& keyword, std::size_t report_step) {
        int status = keyword.getRecord(0).getItem<ParserKeywords::EXIT::STATUS_CODE>().get<int>(0);
        OpmLog::info("Simulation exit with status: " + std::to_string(status) + " requested as part of ACTIONX at report_step: " + std::to_string(report_step));
        this->exit_status = status;
    }

    void Schedule::shut_well(const std::string& well_name, std::size_t report_step) {
        this->updateWellStatus(well_name, report_step, Well::Status::SHUT);
    }

    void Schedule::open_well(const std::string& well_name, std::size_t report_step) {
        this->updateWellStatus(well_name, report_step, Well::Status::OPEN);
    }

    void Schedule::stop_well(const std::string& well_name, std::size_t report_step) {
        this->updateWellStatus(well_name, report_step, Well::Status::STOP);
    }

    /*
      Function is quite dangerous - because if this is called while holding a
      Well pointer that will go stale and needs to be refreshed.
    */
    bool Schedule::updateWellStatus( const std::string& well_name, std::size_t reportStep , Well::Status status, std::optional<KeywordLocation> location) {
        auto well2 = this->snapshots[reportStep].wells.get(well_name);
        if (well2.getConnections().empty() && status == Well::Status::OPEN) {
            if (location) {
                auto msg = fmt::format("Problem with{}\n",
                                       "In {} line{}\n"
                                       "Well {} has no connections to grid and will remain SHUT", location->keyword, location->filename, location->lineno, well_name);
                OpmLog::warning(msg);
            } else
                OpmLog::warning(fmt::format("Well {} has no connections to grid and will remain SHUT", well_name));
            return false;
        }

        auto old_status = well2.getStatus();
        bool update = false;
        if (well2.updateStatus(status)) {
            if (status == Well::Status::OPEN) {
                auto new_rft = this->snapshots.back().rft_config().well_open(well_name);
                if (new_rft.has_value())
                    this->snapshots.back().rft_config.update( std::move(*new_rft) );
            }

            /*
              The Well::updateStatus() will always return true because a new
              WellStatus object should be created. But the new object might have
              the same value as the previous object; therefor we need to check
              for an actual status change before we emit a WELL_STATUS_CHANGE
              event.
            */
            if (old_status != status) {
                this->snapshots.back().events().addEvent( ScheduleEvents::WELL_STATUS_CHANGE);
                this->snapshots.back().wellgroup_events().addEvent( well2.name(), ScheduleEvents::WELL_STATUS_CHANGE);
            }
            this->snapshots[reportStep].wells.update( std::move(well2) );
            update = true;
        }
        return update;
    }


    bool Schedule::updateWPAVE(const std::string& wname, std::size_t report_step, const PAvg& pavg) {
        const auto& well = this->getWell(wname, report_step);
        if (well.pavg() != pavg) {
            auto new_well = this->snapshots[report_step].wells.get(wname);
            new_well.updateWPAVE( pavg );
            this->snapshots[report_step].wells.update( std::move(new_well) );
            return true;
        }
        return false;
    }




    std::optional<std::size_t> Schedule::first_RFT() const {
        for (std::size_t report_step = 0; report_step < this->snapshots.size(); report_step++) {
            if (this->snapshots[report_step].rft_config().active())
                return report_step;
        }
        return {};
    }

    void Schedule::invalidNamePattern( const std::string& namePattern, const HandlerContext& context) const {
        std::string msg_fmt = fmt::format("No wells/groups match the pattern: \'{}\'", namePattern);
        if (namePattern == "?") {
            /*
              In particular when an ACTIONX keyword is called via PYACTION
              coming in here with an empty list of matching wells is not
              entirely unheard of. It is probably not what the user wanted and
              we give a warning, but the simulation continues.
            */
            auto msg = OpmInputError::format("No matching wells for ACTIONX {keyword} in {file} line {line}.", context.keyword.location());
            OpmLog::warning(msg);
        } else
            context.parseContext.handleError(ParseContext::SCHEDULE_INVALID_NAME, msg_fmt, context.keyword.location(), context.errors);
    }

    GTNode Schedule::groupTree(const std::string& root_node, std::size_t report_step, std::size_t level, const std::optional<std::string>& parent_name) const {
        auto root_group = this->getGroup(root_node, report_step);
        GTNode tree(root_group, level, parent_name);

        for (const auto& wname : root_group.wells()) {
            const auto& well = this->getWell(wname, report_step);
            tree.add_well(well);
        }

        for (const auto& gname : root_group.groups()) {
            auto child_group = this->groupTree(gname, report_step, level + 1, root_node);
            tree.add_group(child_group);
        }

        return tree;
    }

    GTNode Schedule::groupTree(const std::string& root_node, std::size_t report_step) const {
        return this->groupTree(root_node, report_step, 0, {});
    }

    GTNode Schedule::groupTree(std::size_t report_step) const {
        return this->groupTree("FIELD", report_step);
    }

    void Schedule::addWell(const std::string& wellName,
                           const DeckRecord& record,
                           std::size_t timeStep,
                           Connection::Order wellConnectionOrder)
    {
        // We change from eclipse's 1 - n, to a 0 - n-1 solution
        int headI = record.getItem("HEAD_I").get< int >(0) - 1;
        int headJ = record.getItem("HEAD_J").get< int >(0) - 1;
        Phase preferredPhase;
        {
            const std::string phaseStr = record.getItem("PHASE").getTrimmedString(0);
            if (phaseStr == "LIQ") {
                // We need a workaround in case the preferred phase is "LIQ",
                // which is not proper phase and will cause the get_phase()
                // function to throw. In that case we choose to treat it as OIL.
                preferredPhase = Phase::OIL;
                OpmLog::warning("LIQ_PREFERRED_PHASE",
                                "LIQ preferred phase not supported for well " + wellName + ", using OIL instead");
            } else {
                preferredPhase = get_phase(phaseStr);
            }
        }
        const auto& refDepthItem = record.getItem("REF_DEPTH");
        std::optional<double> ref_depth;
        if (refDepthItem.hasValue( 0 ))
            ref_depth = refDepthItem.getSIDouble( 0 );

        double drainageRadius = record.getItem( "D_RADIUS" ).getSIDouble(0);

        bool allowCrossFlow = true;
        const std::string& allowCrossFlowStr = record.getItem<ParserKeywords::WELSPECS::CROSSFLOW>().getTrimmedString(0);
        if (allowCrossFlowStr == "NO")
            allowCrossFlow = false;

        bool automaticShutIn = true;
        const std::string& automaticShutInStr = record.getItem<ParserKeywords::WELSPECS::AUTO_SHUTIN>().getTrimmedString(0);
        if (automaticShutInStr == "STOP") {
            automaticShutIn = false;
        }

        const std::string& group = record.getItem<ParserKeywords::WELSPECS::GROUP>().getTrimmedString(0);
        auto pvt_table = record.getItem<ParserKeywords::WELSPECS::P_TABLE>().get<int>(0);
        auto gas_inflow = Well::GasInflowEquationFromString( record.getItem<ParserKeywords::WELSPECS::INFLOW_EQ>().get<std::string>(0) );

        this->addWell(wellName,
                      group,
                      headI,
                      headJ,
                      preferredPhase,
                      ref_depth,
                      drainageRadius,
                      allowCrossFlow,
                      automaticShutIn,
                      pvt_table,
                      gas_inflow,
                      timeStep,
                      wellConnectionOrder);
    }

    void Schedule::addWell(Well well) {
        const std::string wname = well.name();
        auto& sched_state = this->snapshots.back();

        sched_state.events().addEvent( ScheduleEvents::NEW_WELL );
        sched_state.wellgroup_events().addWell( wname );
        {
            auto wo = sched_state.well_order.get();
            wo.add( wname );
            sched_state.well_order.update( std::move(wo) );
        }
        well.setInsertIndex(sched_state.wells.size());
        sched_state.wells.update( std::move(well) );
    }

    void Schedule::addWell(const std::string& wellName,
                           const std::string& group,
                           int headI,
                           int headJ,
                           Phase preferredPhase,
                           const std::optional<double>& ref_depth,
                           double drainageRadius,
                           bool allowCrossFlow,
                           bool automaticShutIn,
                           int pvt_table,
                           Well::GasInflowEquation gas_inflow,
                           std::size_t timeStep,
                           Connection::Order wellConnectionOrder) {

        const auto& sched_state = this->operator[](timeStep);
        Well well(wellName,
                  group,
                  timeStep,
                  0,
                  headI, headJ,
                  ref_depth,
                  WellType(preferredPhase),
                  sched_state.whistctl(),
                  wellConnectionOrder,
                  this->m_static.m_unit_system,
                  this->getUDQConfig(timeStep).params().undefinedValue(),
                  drainageRadius,
                  allowCrossFlow,
                  automaticShutIn,
                  pvt_table,
                  gas_inflow);

        this->addWell( std::move(well) );

        const auto& ts = this->operator[](timeStep);
        this->updateWPAVE( wellName, timeStep, ts.pavg.get() );
    }


    std::size_t Schedule::numWells() const {
        return this->snapshots.back().wells.size();
    }

    std::size_t Schedule::numWells(std::size_t timestep) const {
        auto well_names = this->wellNames(timestep);
        return well_names.size();
    }

    bool Schedule::hasWell(const std::string& wellName) const {
        return this->snapshots.back().wells.has(wellName);
    }

    bool Schedule::hasWell(const std::string& wellName, std::size_t timeStep) const {
        return this->snapshots[timeStep].wells.has(wellName);
    }

    bool Schedule::hasGroup(const std::string& groupName, std::size_t timeStep) const {
        return this->snapshots[timeStep].groups.has(groupName);
    }

    std::vector< const Group* > Schedule::getChildGroups2(const std::string& group_name, std::size_t timeStep) const {
        const auto& sched_state = this->snapshots[timeStep];
        const auto& group = sched_state.groups.get(group_name);

        std::vector<const Group*> child_groups;
        for (const auto& child_name : group.groups())
            child_groups.push_back( std::addressof(this->getGroup(child_name, timeStep)));

        return child_groups;
    }

    std::vector< Well > Schedule::getChildWells2(const std::string& group_name, std::size_t timeStep) const {
        const auto& sched_state = this->snapshots[timeStep];
        const auto& group = sched_state.groups.get(group_name);

        std::vector<Well> wells;

        if (group.groups().size()) {
            for (const auto& child_name : group.groups()) {
                const auto& child_wells = this->getChildWells2(child_name, timeStep);
                wells.insert(wells.end(), child_wells.begin(), child_wells.end());
            }
        } else {
            for (const auto& well_name : group.wells()) {
                wells.push_back( this->getWell(well_name, timeStep));
            }
        }
        return wells;
    }

    /*
      This function will return a list of wells which have changed
      *structurally* in the last report_step; wells where only production
      settings have changed will not be included.
    */
    std::vector<std::string> Schedule::changed_wells(std::size_t report_step) const {
        std::vector<std::string> wells;
        const auto& state = this->snapshots[report_step];
        const auto& all_wells = state.wells();

        if (report_step == 0)
            std::transform( all_wells.begin(), all_wells.end(), std::back_inserter(wells), [] (const auto& well_ref) { return well_ref.get().name(); });
        else {
            const auto& prev_state = this->snapshots[report_step - 1];
            for (const auto& well_ref : all_wells) {
                const auto& wname = well_ref.get().name();
                if (prev_state.wells.has(wname)) {
                    const auto& prev_well = prev_state.wells.get( wname );
                    if (!prev_well.cmp_structure(well_ref.get()))
                        wells.push_back( wname );
                } else
                    wells.push_back( wname );
            }
        }

        return wells;
    }


    std::vector<Well> Schedule::getWells(std::size_t timeStep) const {
        std::vector<Well> wells;
        if (timeStep >= this->snapshots.size())
            throw std::invalid_argument("timeStep argument beyond the length of the simulation");

        const auto& well_order = this->snapshots[timeStep].well_order();
        for (const auto& wname : well_order)
            wells.push_back( this->snapshots[timeStep].wells.get(wname) );

        return wells;
    }

    std::vector<Well> Schedule::getWellsatEnd() const {
        return this->getWells(this->snapshots.size() - 1);
    }

    const Well& Schedule::getWellatEnd(const std::string& well_name) const {
        return this->getWell(well_name, this->snapshots.size() - 1);
    }

    const Well& Schedule::getWell(const std::string& wellName, std::size_t timeStep) const {
        return this->snapshots[timeStep].wells.get(wellName);
    }

    const Well& Schedule::getWell(std::size_t well_index, std::size_t timeStep) const {
        const auto find_pred = [well_index] (const auto& well_pair) -> bool
        {
            return well_pair.second->seqIndex() == well_index;
        };

        auto well_ptr = this->snapshots[timeStep].wells.find( find_pred );
        if (well_ptr == nullptr)
            throw std::invalid_argument(fmt::format("There is no well with well_index:{} at report_step:{}", well_index, timeStep));

        return *well_ptr;
    }

    const Group& Schedule::getGroup(const std::string& groupName, std::size_t timeStep) const {
        return this->snapshots[timeStep].groups.get(groupName);
    }

    void Schedule::updateGuideRateModel(const GuideRateModel& new_model, std::size_t report_step) {
        auto new_config = this->snapshots[report_step].guide_rate();
        if (new_config.update_model(new_model))
            this->snapshots[report_step].guide_rate.update( std::move(new_config) );
    }

    /*
      There are many SCHEDULE keyword which take a wellname as argument. In
      addition to giving a fully qualified name like 'W1' you can also specify
      shell wildcard patterns like like 'W*', you can get all the wells in the
      well-list '*WL'[1] and the wellname '?' is used to get all the wells which
      already have matched a condition in a ACTIONX keyword. This function
      should be one-stop function to get all well names according to a input
      pattern. The timestep argument is used to check that the wells have
      indeed been defined at the point in time we are considering.

      [1]: The leading '*' in a WLIST name should not be interpreted as a shell
           wildcard!
    */


    std::vector<std::string> Schedule::wellNames(const std::string& pattern, std::size_t timeStep, const std::vector<std::string>& matching_wells) const {
        const auto wm = this->wellMatcher(timeStep);

        return (pattern == "?")
            ? wm.sort(matching_wells) // ACTIONX handler
            : wm.wells(pattern);      // Normal well name pattern matching
    }



    std::vector<std::string> Schedule::wellNames(const std::string& pattern, const HandlerContext& context) {
        std::vector<std::string> valid_names;
        const auto& report_step = context.currentStep;
        auto names = this->wellNames(pattern, report_step, context.matching_wells);
        if (names.empty()) {
            const auto& location = context.keyword.location();
            if (this->action_wgnames.has_well(pattern)) {
                std::string msg = fmt::format(R"(Well: {} not yet defined for keyword {}.
Expecting well to be defined with WELSPECS in ACTIONX before actual use.
File {} line {}.)", pattern, location.keyword, location.filename, location.lineno);
                OpmLog::warning(msg);
            } else
                this->invalidNamePattern(pattern, context);
        }
        return names;
    }

    WellMatcher Schedule::wellMatcher(std::size_t report_step) const {
        const ScheduleState * sched_state;

        if (report_step < this->snapshots.size())
            sched_state = &this->snapshots[report_step];
        else
            sched_state = &this->snapshots.back();

        return WellMatcher(sched_state->well_order.get(), sched_state->wlist_manager.get());
    }


    std::vector<std::string> Schedule::wellNames(const std::string& pattern) const {
        return this->wellNames(pattern, this->size() - 1);
    }

    std::vector<std::string> Schedule::wellNames(std::size_t timeStep) const {
        const auto& well_order = this->snapshots[timeStep].well_order();
        return well_order.names();
    }

    std::vector<std::string> Schedule::wellNames() const {
        const auto& well_order = this->snapshots.back().well_order();
        return well_order.names();
    }

    std::vector<std::string> Schedule::groupNames(const std::string& pattern, std::size_t timeStep) const {
        if (pattern.size() == 0)
            return {};

        const auto& group_order = this->snapshots[timeStep].group_order();

        // Normal pattern matching
        auto star_pos = pattern.find('*');
        if (star_pos != std::string::npos) {
            std::vector<std::string> names;
            for (const auto& gname : group_order) {
                if (name_match(pattern, gname))
                    names.push_back(gname);
            }
            return names;
        }

        // Normal group name without any special characters
        if (group_order.has(pattern))
            return { pattern };

        return {};
    }

    std::vector<std::string> Schedule::groupNames(std::size_t timeStep) const {
        const auto& group_order = this->snapshots[timeStep].group_order();
        return group_order.names();
    }

    std::vector<std::string> Schedule::groupNames(const std::string& pattern) const {
        return this->groupNames(pattern, this->snapshots.size() - 1);
    }

    std::vector<std::string> Schedule::groupNames() const {
        const auto& group_order = this->snapshots.back().group_order();
        return group_order.names();
    }

    std::vector<const Group*> Schedule::restart_groups(std::size_t timeStep) const {
        const auto& restart_groups = this->snapshots[timeStep].group_order().restart_groups();
        std::vector<const Group*> rst_groups(restart_groups.size() , nullptr );
        for (std::size_t restart_index = 0; restart_index < restart_groups.size(); restart_index++) {
            const auto& group_name = restart_groups[restart_index];
            if (group_name.has_value())
                rst_groups[restart_index] = &this->getGroup(group_name.value(), timeStep);
        }
        return rst_groups;
    }


    void Schedule::addGroup(Group group) {
        std::string group_name = group.name();
        auto& sched_state = this->snapshots.back();
        sched_state.groups.update(std::move(group) );
        sched_state.events().addEvent( ScheduleEvents::NEW_GROUP );
        sched_state.wellgroup_events().addGroup(group_name);
        {
            auto go = sched_state.group_order.get();
            go.add( group_name );
            sched_state.group_order.update( std::move(go) );
        }

        // All newly created groups are attached to the field group,
        // can then be relocated with the GRUPTREE keyword.
        if (group_name != "FIELD")
            this->addGroupToGroup("FIELD", group_name);
    }


    void Schedule::addGroup(const std::string& groupName, std::size_t timeStep) {
        auto udq_undefined = this->getUDQConfig(timeStep).params().undefinedValue();
        const auto& sched_state = this->snapshots.back();
        auto insert_index = sched_state.groups.size();
        this->addGroup( Group(groupName, insert_index, udq_undefined, this->m_static.m_unit_system) );
    }


    void Schedule::addGroup(const RestartIO::RstGroup& rst_group, std::size_t timeStep) {
        auto udq_undefined = this->getUDQConfig(timeStep).params().undefinedValue();

        const auto insert_index = this->snapshots.back().groups.size();
        auto new_group = Group(rst_group, insert_index, udq_undefined, this->m_static.m_unit_system);
        if (rst_group.name != "FIELD") {
            // Common case.  Add new group.
            this->addGroup( std::move(new_group) );
            return;
        }

        // If we get here we're updating the FIELD group to incorporate any
        // applicable field-wide GCONPROD and/or GCONINJE settings stored in
        // the restart file.  Happens at most once per run.

        auto& field = this->snapshots.back().groups.get("FIELD");
        if (new_group.isProductionGroup())
            // Initialise field-wide GCONPROD settings from restart.
            field.updateProduction(new_group.productionProperties());
        for (const auto phase : { Phase::GAS, Phase::WATER })
            if (new_group.hasInjectionControl(phase))
                // Initialise field-wide GCONINJE settings (phase) from restart.
                field.updateInjection(new_group.injectionProperties(phase));
    }


    void Schedule::addGroupToGroup( const std::string& parent_name, const std::string& child_name) {
        auto parent_group = this->snapshots.back().groups.get(parent_name);
        if (parent_group.addGroup(child_name))
            this->snapshots.back().groups.update( std::move(parent_group) );

        // Check and update backreference in child
        const auto& child_group = this->snapshots.back().groups.get(child_name);
        if (child_group.parent() != parent_name) {
            auto old_parent = this->snapshots.back().groups.get(child_group.parent());
            old_parent.delGroup(child_group.name());
            this->snapshots.back().groups.update( std::move(old_parent) );

            auto new_child_group = Group{ child_group };
            new_child_group.updateParent(parent_name);
            this->snapshots.back().groups.update( std::move(new_child_group) );
        }
    }

    void Schedule::addWellToGroup( const std::string& group_name, const std::string& well_name , std::size_t timeStep) {
        auto well = this->getWell(well_name, timeStep);
        const auto old_gname = well.groupName();
        if (old_gname != group_name) {
            well.updateGroup(group_name);
            this->snapshots.back().wells.update( std::move(well) );
            this->snapshots.back().wellgroup_events().addEvent( well_name, ScheduleEvents::WELL_WELSPECS_UPDATE );

            // Remove well child reference from previous group
            auto group = this->snapshots.back().groups.get( old_gname );
            group.delWell(well_name);
            this->snapshots.back().groups.update( std::move(group) );
        }

        // Add well child reference to new group
        auto group = this->snapshots.back().groups.get( group_name );
        group.addWell(well_name);
        this->snapshots.back().groups.update( std::move(group) );
        this->snapshots.back().events().addEvent( ScheduleEvents::GROUP_CHANGE );
    }



    Well::ProducerCMode Schedule::getGlobalWhistctlMmode(std::size_t timestep) const {
        return this->operator[](timestep).whistctl();
    }



    void Schedule::checkIfAllConnectionsIsShut(std::size_t timeStep) {
        const auto& well_names = this->wellNames(timeStep);
        for (const auto& wname : well_names) {
            const auto& well = this->getWell(wname, timeStep);
            const auto& connections = well.getConnections();
            if (connections.allConnectionsShut() && well.getStatus() != Well::Status::SHUT) {
                auto elapsed = this->snapshots[timeStep].start_time() - this->snapshots[0].start_time();
                auto days = std::chrono::duration_cast<std::chrono::hours>(elapsed).count() / 24.0;
                auto msg = fmt::format("All completions in well {} is shut at {} days\n"
                                       "The well is therefore also shut", well.name(), days);
                OpmLog::note(msg);
                this->updateWellStatus( well.name(), timeStep, Well::Status::SHUT);
            }
        }
    }

    void Schedule::end_report(std::size_t report_step) {
        this->checkIfAllConnectionsIsShut(report_step);
    }


    void Schedule::filterConnections(const ActiveGridCells& grid) {
        for (auto& sched_state : this->snapshots) {
            for (auto& well : sched_state.wells()) {
                well.get().filterConnections(grid);
            }
        }
    }


    const UDQConfig& Schedule::getUDQConfig(std::size_t timeStep) const {
        return this->snapshots[timeStep].udq.get();
    }

    std::optional<int> Schedule::exitStatus() const {
        return this->exit_status;
    }

    std::size_t Schedule::size() const {
        return this->snapshots.size();
    }


    double Schedule::seconds(std::size_t timeStep) const {
        if (this->snapshots.empty())
            return 0;

        if (timeStep >= this->snapshots.size())
            throw std::logic_error(fmt::format("seconds({}) - invalid timeStep. Valid range [0,{}>", timeStep, this->snapshots.size()));

        auto elapsed = this->snapshots[timeStep].start_time() - this->snapshots[0].start_time();
        return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    }

    time_t Schedule::simTime(std::size_t timeStep) const {
        return std::chrono::system_clock::to_time_t( this->snapshots[timeStep].start_time() );
    }

    double Schedule::stepLength(std::size_t timeStep) const {
        auto elapsed = this->snapshots[timeStep].end_time() - this->snapshots[timeStep].start_time();
        return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    }

    void Schedule::applyKeywords(
             std::vector<DeckKeyword*>& keywords, std::size_t reportStep) {
        ParseContext parseContext;
        ErrorGuard errors;
        ScheduleGrid grid(this->completed_cells);
        SimulatorUpdate sim_update;
        std::unordered_map<std::string, double> target_wellpi;
        std::vector<std::string> matching_wells;
        const std::string prefix = "| "; /* logger prefix string */
        this->snapshots.resize(reportStep + 1);
        auto& input_block = this->m_sched_deck[reportStep];
        for (auto keyword : keywords) {
            input_block.push_back(*keyword);
            this->handleKeyword(reportStep,
                                input_block,
                                *keyword,
                                parseContext,
                                errors,
                                grid,
                                matching_wells,
                                /*actionx_mode=*/false,
                                &sim_update,
                                &target_wellpi);
        }
        this->end_report(reportStep);
        if (reportStep < this->m_sched_deck.size() - 1) {
            iterateScheduleSection(
                reportStep + 1,
                this->m_sched_deck.size(),
                parseContext,
                errors,
                grid,
                &target_wellpi,
                prefix);
        }
    }


    SimulatorUpdate Schedule::applyAction(std::size_t reportStep, const Action::ActionX& action, const std::vector<std::string>& matching_wells, const std::unordered_map<std::string, double>& target_wellpi) {
        const std::string prefix = "| ";
        ParseContext parseContext;
        ErrorGuard errors;
        SimulatorUpdate sim_update;
        ScheduleGrid grid(this->completed_cells);

        OpmLog::info("/----------------------------------------------------------------------");
        OpmLog::info(fmt::format("{0}Action {1} evaluated to true. Will add action keywords and\n{0}rerun Schedule section.\n{0}", prefix, action.name()));
        this->snapshots.resize(reportStep + 1);
        auto& input_block = this->m_sched_deck[reportStep];
        for (const auto& keyword : action) {
            input_block.push_back(keyword);
            const auto& location = keyword.location();
            OpmLog::info(fmt::format("{}Processing keyword {} from {} line {}", prefix, location.keyword, location.filename, location.lineno));
            this->handleKeyword(reportStep,
                                input_block,
                                keyword,
                                parseContext,
                                errors,
                                grid,
                                matching_wells,
                                true,
                                &sim_update,
                                &target_wellpi);
        }
        this->end_report(reportStep);
        if (!sim_update.affected_wells.empty()) {
            this->snapshots.back().events().addEvent( ScheduleEvents::ACTIONX_WELL_EVENT );
            for (const auto& well: sim_update.affected_wells)
                this->snapshots.back().wellgroup_events().addEvent(well, ScheduleEvents::ACTIONX_WELL_EVENT);
        }

        if (reportStep < this->m_sched_deck.size() - 1)
            iterateScheduleSection(reportStep + 1, this->m_sched_deck.size(), parseContext, errors, grid, &target_wellpi, prefix);
        OpmLog::info("\\----------------------------------------------------------------------");

        return sim_update;
    }


    /*
      This function will typically be called from the apply_action_callback()
      which is invoked in a PYACTION plugin, i.e. the arguments here are
      supplied by the user in a script - can very well be wrong.
    */
    SimulatorUpdate Schedule::applyAction(std::size_t reportStep, const std::string& action_name, const std::vector<std::string>& matching_wells) {
        const auto& actions = this->snapshots[reportStep].actions();
        if (actions.has(action_name)) {
            const auto& action = this->snapshots[reportStep].actions()[action_name];

            std::vector<std::string> well_names;
            for (const auto& wname : matching_wells) {
                if (this->hasWell(wname, reportStep))
                    well_names.push_back(wname);
                else
                    OpmLog::error(fmt::format("Tried to apply action: {} on non existing well: {}", action_name, wname));
            }

            return this->applyAction(reportStep, action, well_names, {});
        } else {
            OpmLog::error(fmt::format("Tried to apply action unknown action: {}", action_name));
            return {};
        }
    }


    /*
      The runPyAction() method is a utility to run PYACTION keywords. The
      PYACTION keywords contain a link to file with Python code and a function
      run() which will eventually be invoked.

      In principal the python code can do "anything" - but when it comes to
      modifications of the Schedule information - e.g. by opening or closing
      wells, the recommended way to do it is unfortunately to utilize the normal
      ACTIONX machinery and apply a ACTIONX keyword in order to invoke the
      keywords in the ACTIONX block. In order to set this up correctly we need
      to align three different systems:

         1. The Python code executes normally, and will possibly decide to
            apply an ACTIONX keyword.

         2. When an AXTIONX keyword is applied the Schedule implementation will
            need to add the new keywords to the correct ScheduleBlock and
            reiterate the Schedule section.

         3. As part of the Schedule iteration we record which changes which must
            be taken into account in the simulator afterwards. These changes are
            recorded in a Action::SimulatorUpdate instance.

      An important part of the implementation is the lambda
      'apply_action_callback' which is used from Python to call back in to C++
      in order to run the ACTIONX keywords. The sequence of calls goes like
      this:

         1. The simulator calls the method Schedule::runPyAction()

         2. The Schedule::runPyAction() method creates a SimulatorUpdate
            instance and captures a reference to that in the lambda
            apply_action_callback. When calling the pyaction.run() method the
            apply_action_callback is passed as a callable all the way down to
            the python run() method.

         3. In python the apply_action_callback comes in as the parameter
            'actionx_callback' in the run() function. If the python code decides
            to invoke the keywords from an actionx it will be like:

            def run(ecl_state, schedule, report_step, summary_state, actionx_callback):
                ...
                ...
                wells = ["W1", "W2"]
                actionx_callback("ACTION_NAME", wells)

            Observe that the wells argument must be a Python lvalue (otherwise
            hard crash???)

         4. The callable will go back into C++ and eventually reach the
            Schedule::applyAction() which will invoke the
            Schedule::iterateScheduleSection() and return an update
            SimulatorUpdate which will be assigned to the instance in the
            Schedule::runPyAction().

         5. When the pyaction.run() method returns the Schedule structure and
            the sim_update variable have been correctly updated, and the
            sim_update is returned to the simulator.
    */


    SimulatorUpdate Schedule::runPyAction(std::size_t reportStep, const Action::PyAction& pyaction, Action::State& action_state, EclipseState& ecl_state, SummaryState& summary_state) {
        SimulatorUpdate sim_update;

        auto apply_action_callback = [&sim_update, &reportStep, this](const std::string& action_name, const std::vector<std::string>& matching_wells) {
            sim_update = this->applyAction(reportStep, action_name, matching_wells);
        };

        auto result = pyaction.run(ecl_state, *this, reportStep, summary_state, apply_action_callback);
        action_state.add_run(pyaction, result);
        return sim_update;
    }

    void Schedule::applyWellProdIndexScaling(const std::string& well_name, const std::size_t reportStep, const double newWellPI) {
        if (reportStep >= this->snapshots.size())
            return;

        if (!this->snapshots[reportStep].wells.has(well_name))
            return;

        std::vector<Well *> unique_wells;
        for (std::size_t step = reportStep; step < this->snapshots.size(); step++) {
            auto& well = this->snapshots[step].wells.get(well_name);
            if (unique_wells.empty() || (!(*unique_wells.back() == well)))
                unique_wells.push_back( &well );
        }

        std::vector<bool> scalingApplicable;
        const auto targetPI = this->snapshots[reportStep].target_wellpi.at(well_name);
        auto prev_well = unique_wells[0];
        auto scalingFactor = prev_well->convertDeckPI(targetPI) / newWellPI;
        prev_well->applyWellProdIndexScaling(scalingFactor, scalingApplicable);

        for (std::size_t well_index = 1; well_index < unique_wells.size(); well_index++) {
            auto wellPtr = unique_wells[well_index];
            if (! wellPtr->hasSameConnectionsPointers(*prev_well)) {
                wellPtr->applyWellProdIndexScaling(scalingFactor, scalingApplicable);
                prev_well = wellPtr;
            }
        }
    }

    bool Schedule::write_rst_file(const std::size_t report_step) const
    {
        return this->restart_output.writeRestartFile(report_step) || this->operator[](report_step).save();
    }

    bool Schedule::must_write_rst_file(const std::size_t report_step) const
    {
        if (this->m_static.output_interval.has_value())
            return this->m_static.output_interval.value() % report_step;

        if (report_step == 0)
            return this->m_static.rst_config.write_rst_file.value();

        const auto previous_restart_output_step =
            this->restart_output.lastRestartEventBefore(report_step);

        // Previous output event time or start of simulation if no previous
        // event recorded
        const auto previous_output = previous_restart_output_step.has_value()
            ? this->snapshots[previous_restart_output_step.value()].start_time()
            : this->snapshots[0].start_time();

        const auto& rst_config = this->snapshots[report_step - 1].rst_config();
        return this->snapshots[report_step].rst_file(rst_config, previous_output);
    }


    const std::map< std::string, int >& Schedule::rst_keywords( size_t report_step ) const {
        if (report_step == 0)
            return this->m_static.rst_config.keywords;

        const auto& keywords = this->snapshots[report_step - 1].rst_config().keywords;
        return keywords;
    }

    bool Schedule::operator==(const Schedule& data) const {
        return this->m_static == data.m_static &&
               this->m_sched_deck == data.m_sched_deck &&
               this->snapshots == data.snapshots &&
               this->restart_output == data.restart_output &&
               this->action_wgnames == data.action_wgnames &&
               this->completed_cells == data.completed_cells;
     }


    std::string Schedule::formatDate(std::time_t t) {
        const auto ts { TimeStampUTC(t) } ;
        return fmt::format("{:04d}-{:02d}-{:02d}" , ts.year(), ts.month(), ts.day());
    }

    std::string Schedule::simulationDays(std::size_t currentStep) const {
        const double sim_time { this->m_static.m_unit_system.from_si(UnitSystem::measure::time, simTime(currentStep)) } ;
        return fmt::format("{} {}", sim_time, this->m_static.m_unit_system.name(UnitSystem::measure::time));
    }

namespace {

    // Duplicated from Well.cpp
    Connection::Order order_from_int(int int_value) {
        switch(int_value) {
        case 0:
            return Connection::Order::TRACK;
        case 1:
            return Connection::Order::DEPTH;
        case 2:
            return Connection::Order::INPUT;
        default:
            throw std::invalid_argument("Invalid integer value: " + std::to_string(int_value) + " encountered when determining connection ordering");
        }
    }
}

    void Schedule::load_rst(const RestartIO::RstState& rst_state, const TracerConfig& tracer_config, const ScheduleGrid& grid, const FieldPropsManager& fp)
    {
        const auto report_step = rst_state.header.report_step - 1;
        double udq_undefined = 0;
        for (const auto& rst_group : rst_state.groups) {
            this->addGroup(rst_group, report_step);
            const auto& group = this->snapshots.back().groups.get( rst_group.name );
            if (group.isProductionGroup()) {
                // Was originally at report_step + 1
                this->snapshots.back().events().addEvent(ScheduleEvents::GROUP_PRODUCTION_UPDATE );
                this->snapshots.back().wellgroup_events().addEvent(rst_group.name, ScheduleEvents::GROUP_PRODUCTION_UPDATE);
            }

            if (group.isInjectionGroup()) {
                // Was originally at report_step + 1
                this->snapshots.back().events().addEvent(ScheduleEvents::GROUP_INJECTION_UPDATE );
                this->snapshots.back().wellgroup_events().addEvent(rst_group.name, ScheduleEvents::GROUP_INJECTION_UPDATE);
            }

            OpmLog::info(fmt::format("Adding group {} from restart file", rst_group.name));
        }

        auto glo = this->snapshots.back().glo();
        glo.all_newton(rst_state.header.glift_all_nupcol);
        glo.min_wait(rst_state.header.glift_min_wait);
        glo.min_eco_gradient(rst_state.header.glift_min_eco_grad);
        glo.gaslift_increment(rst_state.header.glift_rate_delta);

        for (std::size_t group_index = 0; group_index < rst_state.groups.size(); group_index++) {
            const auto& rst_group = rst_state.groups[group_index];

            if (rst_group.parent_group == 0)
                continue;

            if (rst_group.parent_group == rst_state.header.max_groups_in_field)
                continue;

            const auto& parent_group = rst_state.groups[rst_group.parent_group - 1];
            this->addGroupToGroup(parent_group.name, rst_group.name);

            if (GasLiftOpt::Group::active(rst_group))
                glo.add_group(GasLiftOpt::Group(rst_group));
        }

        for (const auto& rst_well : rst_state.wells) {
            Opm::Well well(rst_well, report_step, tracer_config, this->m_static.m_unit_system, udq_undefined);
            std::vector<Opm::Connection> rst_connections;

            for (const auto& rst_conn : rst_well.connections)
                rst_connections.emplace_back(rst_conn, grid, fp);

            if (rst_well.segments.empty()) {
                Opm::WellConnections connections(order_from_int(rst_well.completion_ordering),
                                                 rst_well.ij[0],
                                                 rst_well.ij[1],
                                                 rst_connections);
                well.updateConnections( std::make_shared<WellConnections>( std::move(connections) ), grid);
            } else {
                std::unordered_map<int, Opm::Segment> rst_segments;
                for (const auto& rst_segment : rst_well.segments) {
                    Opm::Segment segment(rst_segment);
                    rst_segments.insert(std::make_pair(rst_segment.segment, std::move(segment)));
                }

                auto [connections, segments] = Compsegs::rstUpdate(rst_well, rst_connections, rst_segments);
                well.updateConnections( std::make_shared<WellConnections>(std::move(connections)), grid);
                well.updateSegments( std::make_shared<WellSegments>(std::move(segments) ));
            }

            this->addWell(well);
            this->addWellToGroup(well.groupName(), well.name(), report_step);

            OpmLog::info(fmt::format("Adding well {} from restart file", rst_well.name));

            if (GasLiftOpt::Well::active(rst_well))
                glo.add_well(GasLiftOpt::Well(rst_well));
        }
        this->snapshots.back().glo.update( std::move(glo) );
        this->snapshots.back().update_tuning(rst_state.tuning);
        this->snapshots.back().events().addEvent( ScheduleEvents::TUNING_CHANGE );

        {
            const auto& header = rst_state.header;
            if (GuideRateModel::rst_valid(header.guide_rate_delay,
                                          header.guide_rate_a,
                                          header.guide_rate_b,
                                          header.guide_rate_c,
                                          header.guide_rate_d,
                                          header.guide_rate_e,
                                          header.guide_rate_f,
                                          header.guide_rate_damping))
            {
                const bool allow_increase = true;
                const bool use_free_gas = false;

                const auto guide_rate_model = GuideRateModel {
                    header.guide_rate_delay,
                    GuideRateModel::TargetFromRestart(header.guide_rate_nominated_phase),
                    header.guide_rate_a,
                    header.guide_rate_b,
                    header.guide_rate_c,
                    header.guide_rate_d,
                    header.guide_rate_e,
                    header.guide_rate_f,
                    allow_increase,
                    header.guide_rate_damping,
                    use_free_gas
                };

                this->updateGuideRateModel(guide_rate_model, report_step);
            }
        }

        for (const auto& rst_group : rst_state.groups) {
            const auto& group = this->snapshots.back().groups.get( rst_group.name );
            if (group.isProductionGroup()) {
                auto new_config = this->snapshots.back().guide_rate();
                new_config.update_production_group(group);
                this->snapshots.back().guide_rate.update(std::move(new_config));
            }
        }

        this->snapshots.back().udq.update( UDQConfig(this->m_static.m_runspec.udqParams(), rst_state) );
        const auto& uda_records = UDQActive::load_rst( this->m_static.m_unit_system, this->snapshots.back().udq(), rst_state, this->wellNames(report_step), this->groupNames(report_step));
        if (!uda_records.empty()) {
            const auto& udq_config = this->snapshots.back().udq();
            auto udq_active = this->snapshots.back().udq_active();

            for (const auto& [control, value, wgname, ig_phase] : uda_records) {
                if (UDQ::well_control(control)) {
                    auto& well = this->snapshots.back().wells.get(wgname);

                    if (UDQ::is_well_injection_control(control, well.isInjector())) {
                        auto injection_properties = std::make_shared<Well::WellInjectionProperties>(well.getInjectionProperties());
                        injection_properties->update_uda(udq_config, udq_active, control, value);
                        well.updateInjection(std::move(injection_properties));
                    }

                    if (UDQ::is_well_production_control(control, well.isProducer())) {
                        auto production_properties = std::make_shared<Well::WellProductionProperties>(well.getProductionProperties());
                        production_properties->update_uda(udq_config, udq_active, control, value);
                        well.updateProduction(std::move(production_properties));
                    }
                } else {
                    auto& group = this->snapshots.back().groups.get(wgname);
                    if (UDQ::is_group_injection_control(control)) {
                        auto injection_properties = group.injectionProperties(ig_phase.value());
                        injection_properties.update_uda(udq_config, udq_active, control, value);
                        group.updateInjection(injection_properties);
                    }

                    if (UDQ::is_group_production_control(control)) {
                        auto production_properties = group.productionProperties();
                        production_properties.update_uda(udq_config, udq_active, control, value);
                        group.updateProduction(production_properties);
                    }
                }
            }
            this->snapshots.back().udq_active.update( std::move(udq_active) );
        }

        if (!rst_state.actions.empty()) {
            auto actions = this->snapshots.back().actions();
            for (const auto& rst_action : rst_state.actions)
                actions.add( Action::ActionX(rst_action) );
            this->snapshots.back().actions.update( std::move(actions) );
        }
        this->snapshots.back().wtest_config.update( WellTestConfig{rst_state, report_step});


        if (!rst_state.wlists.empty())
            this->snapshots.back().wlist_manager.update( WListManager(rst_state) );

        if (rst_state.network.isActive()) {
            auto network = this->snapshots.back().network();

            // Note: We presently support only the default value of BRANPROP(4).
            const auto alq_value =
                ParserKeywords::BRANPROP::ALQ::defaultValue;

            const auto& rst_nodes = rst_state.network.nodes();
            for (const auto& rst_branch : rst_state.network.branches()) {
                if ((rst_branch.down < 0) || (rst_branch.up < 0)) {
                    // Prune branches to non-existent nodes.
                    continue;
                }

                const auto& downtree_node = rst_nodes[rst_branch.down].name;
                const auto& uptree_node = rst_nodes[rst_branch.up].name;

                network.add_branch({ downtree_node, uptree_node, rst_branch.vfp, alq_value });
            }

            for (const auto& rst_node : rst_nodes) {
                auto node = Network::Node { rst_node.name };

                if (rst_node.terminal_pressure.has_value()) {
                    node.terminal_pressure(rst_node.terminal_pressure.value());
                }

                if (rst_node.as_choke.has_value()) {
                    node.as_choke(rst_node.as_choke.value());
                }

                node.add_gas_lift_gas(rst_node.add_lift_gas);

                network.add_node(std::move(node));
            }

            this->snapshots.back().network.update(std::move(network));
        }
    }

    std::shared_ptr<const Python> Schedule::python() const
    {
        return this->m_static.m_python_handle;
    }


    const GasLiftOpt& Schedule::glo(std::size_t report_step) const {
        return this->snapshots[report_step].glo();
    }

namespace {
/*
  The insane trickery here (thank you Stackoverflow!) is to be able to provide a
  simple templated comparison function

     template <typename T>
     int not_equal(const T& arg1, const T& arg2, const std::string& msg);

  which will print arg1 and arg2 on stderr *if* T supports operator<<, otherwise
  it will just print the typename of T.
*/


template<typename T, typename = int>
struct cmpx
{
    int neq(const T& arg1, const T& arg2, const std::string& msg) {
        if (arg1 == arg2)
            return 0;

        std::cerr << "Error when comparing <" << typeid(arg1).name() << ">: " << msg << std::endl;
        return 1;
    }
};

template <typename T>
struct cmpx<T, decltype(std::cout << T(), 0)>
{
    int neq(const T& arg1, const T& arg2, const std::string& msg) {
        if (arg1 == arg2)
            return 0;

        std::cerr << "Error when comparing: " << msg << " " << arg1 << " != " << arg2 << std::endl;
        return 1;
    }
};


template <typename T>
int not_equal(const T& arg1, const T& arg2, const std::string& msg) {
    return cmpx<T>().neq(arg1, arg2, msg);
}


template <>
int not_equal(const double& arg1, const double& arg2, const std::string& msg) {
    if (Opm::cmp::scalar_equal(arg1, arg2))
        return 0;

    std::cerr << "Error when comparing: " << msg << " " << arg1 << " != " << arg2 << std::endl;
    return 1;
}

template <>
int not_equal(const UDAValue& arg1, const UDAValue& arg2, const std::string& msg) {
    if (arg1.is<double>())
        return not_equal( arg1.get<double>(), arg2.get<double>(), msg);
    else
        return not_equal( arg1.get<std::string>(), arg2.get<std::string>(), msg);
}


std::string well_msg(const std::string& well, const std::string& msg) {
    return "Well: " + well + " " + msg;
}

std::string well_segment_msg(const std::string& well, int segment_number, const std::string& msg) {
    return "Well: " + well + " Segment: " + std::to_string(segment_number) + " " + msg;
}

std::string well_connection_msg(const std::string& well, const Connection& conn, const std::string& msg) {
    return "Well: " + well + " Connection: " + std::to_string(conn.getI()) + ", " + std::to_string(conn.getJ()) + ", " + std::to_string(conn.getK()) + "  " + msg;
}

}

bool Schedule::cmp(const Schedule& sched1, const Schedule& sched2, std::size_t report_step) {
    int count = not_equal(sched1.wellNames(report_step), sched2.wellNames(report_step), "Wellnames");
    if (count != 0)
        return false;
    {
        //if (sched1.size() != sched2.size())
        //    return false;

        //for (std::size_t step=0; step < sched1.size(); step++) {
        //    auto start1 = sched1[step].start_time();
        //    auto start2 = sched2[step].start_time();
        //    if (start1 != start2)
        //        return false;

        //    if (step < sched1.size() - 1) {
        //        auto end1 = sched1[step].end_time();
        //        auto end2 = sched2[step].end_time();
        //        if (end1 != end2)
        //            return false;
        //    }
        //}
    }

    for (const auto& wname : sched1.wellNames(report_step)) {
        const auto& well1 = sched1.getWell(wname, report_step);
        const auto& well2 = sched2.getWell(wname, report_step);
        int well_count = 0;
        {
            const auto& connections2 = well2.getConnections();
            const auto& connections1 = well1.getConnections();

            well_count += not_equal( connections1.ordering(), connections2.ordering(), well_msg(well1.name(), "Connection: ordering"));
            for (std::size_t icon = 0; icon < connections1.size(); icon++) {
                const auto& conn1 = connections1[icon];
                const auto& conn2 = connections2[icon];
                well_count += not_equal( conn1.getI(), conn2.getI(), well_connection_msg(well1.name(), conn1, "I"));
                well_count += not_equal( conn1.getJ() , conn2.getJ() , well_connection_msg(well1.name(), conn1, "J"));
                well_count += not_equal( conn1.getK() , conn2.getK() , well_connection_msg(well1.name(), conn1, "K"));
                well_count += not_equal( conn1.state() , conn2.state(), well_connection_msg(well1.name(), conn1, "State"));
                well_count += not_equal( conn1.dir() , conn2.dir(), well_connection_msg(well1.name(), conn1, "dir"));
                well_count += not_equal( conn1.complnum() , conn2.complnum(), well_connection_msg(well1.name(), conn1, "complnum"));
                well_count += not_equal( conn1.segment() , conn2.segment(), well_connection_msg(well1.name(), conn1, "segment"));
                well_count += not_equal( conn1.kind() , conn2.kind(), well_connection_msg(well1.name(), conn1, "CFKind"));
                well_count += not_equal( conn1.sort_value(), conn2.sort_value(), well_connection_msg(well1.name(), conn1, "sort_value"));


                well_count += not_equal( conn1.CF(), conn2.CF(), well_connection_msg(well1.name(), conn1, "CF"));
                well_count += not_equal( conn1.Kh(), conn2.Kh(), well_connection_msg(well1.name(), conn1, "Kh"));
                well_count += not_equal( conn1.rw(), conn2.rw(), well_connection_msg(well1.name(), conn1, "rw"));
                well_count += not_equal( conn1.depth(), conn2.depth(), well_connection_msg(well1.name(), conn1, "depth"));

                //well_count += not_equal( conn1.r0(), conn2.r0(), well_connection_msg(well1.name(), conn1, "r0"));
                well_count += not_equal( conn1.skinFactor(), conn2.skinFactor(), well_connection_msg(well1.name(), conn1, "skinFactor"));

            }
        }

        if (not_equal(well1.isMultiSegment(), well2.isMultiSegment(), well_msg(well1.name(), "Is MSW")))
            return false;

        if (well1.isMultiSegment()) {
            const auto& segments1 = well1.getSegments();
            const auto& segments2 = well2.getSegments();
            if (not_equal(segments1.size(), segments2.size(), "Segments: size"))
                return false;

            for (std::size_t iseg=0; iseg < segments1.size(); iseg++) {
                const auto& segment1 = segments1[iseg];
                const auto& segment2 = segments2[iseg];
                //const auto& segment2 = segments2.getFromSegmentNumber(segment1.segmentNumber());
                well_count += not_equal(segment1.segmentNumber(), segment2.segmentNumber(), well_segment_msg(well1.name(), segment1.segmentNumber(), "segmentNumber"));
                well_count += not_equal(segment1.branchNumber(), segment2.branchNumber(), well_segment_msg(well1.name(), segment1.segmentNumber(), "branchNumber"));
                well_count += not_equal(segment1.outletSegment(), segment2.outletSegment(), well_segment_msg(well1.name(), segment1.segmentNumber(), "outletSegment"));
                well_count += not_equal(segment1.totalLength(), segment2.totalLength(), well_segment_msg(well1.name(), segment1.segmentNumber(), "totalLength"));
                well_count += not_equal(segment1.depth(), segment2.depth(), well_segment_msg(well1.name(), segment1.segmentNumber(), "depth"));
                well_count += not_equal(segment1.internalDiameter(), segment2.internalDiameter(), well_segment_msg(well1.name(), segment1.segmentNumber(), "internalDiameter"));
                well_count += not_equal(segment1.roughness(), segment2.roughness(), well_segment_msg(well1.name(), segment1.segmentNumber(), "roughness"));
                well_count += not_equal(segment1.crossArea(), segment2.crossArea(), well_segment_msg(well1.name(), segment1.segmentNumber(), "crossArea"));
                well_count += not_equal(segment1.volume(), segment2.volume(), well_segment_msg(well1.name(), segment1.segmentNumber(), "volume"));
            }
        }

        well_count += not_equal(well1.getStatus(), well2.getStatus(), well_msg(well1.name(), "status"));
        {
            const auto& prod1 = well1.getProductionProperties();
            const auto& prod2 = well2.getProductionProperties();
            well_count += not_equal(prod1.name, prod2.name , well_msg(well1.name(), "Prod: name"));
            well_count += not_equal(prod1.OilRate, prod2.OilRate, well_msg(well1.name(), "Prod: OilRate"));
            well_count += not_equal(prod1.GasRate, prod2.GasRate, well_msg(well1.name(), "Prod: GasRate"));
            well_count += not_equal(prod1.WaterRate, prod2.WaterRate, well_msg(well1.name(), "Prod: WaterRate"));
            well_count += not_equal(prod1.LiquidRate, prod2.LiquidRate, well_msg(well1.name(), "Prod: LiquidRate"));
            well_count += not_equal(prod1.ResVRate, prod2.ResVRate, well_msg(well1.name(), "Prod: ResVRate"));
            well_count += not_equal(prod1.BHPTarget, prod2.BHPTarget, well_msg(well1.name(), "Prod: BHPTarget"));
            well_count += not_equal(prod1.THPTarget, prod2.THPTarget, well_msg(well1.name(), "Prod: THPTarget"));
            well_count += not_equal(prod1.VFPTableNumber, prod2.VFPTableNumber, well_msg(well1.name(), "Prod: VFPTableNumber"));
            well_count += not_equal(prod1.ALQValue, prod2.ALQValue, well_msg(well1.name(), "Prod: ALQValue"));
            well_count += not_equal(prod1.predictionMode, prod2.predictionMode, well_msg(well1.name(), "Prod: predictionMode"));
            if (!prod1.predictionMode) {
                well_count += not_equal(prod1.bhp_hist_limit, prod2.bhp_hist_limit, well_msg(well1.name(), "Prod: bhp_hist_limit"));
                well_count += not_equal(prod1.thp_hist_limit, prod2.thp_hist_limit, well_msg(well1.name(), "Prod: thp_hist_limit"));
                well_count += not_equal(prod1.BHPH, prod2.BHPH, well_msg(well1.name(), "Prod: BHPH"));
                well_count += not_equal(prod1.THPH, prod2.THPH, well_msg(well1.name(), "Prod: THPH"));
            }
            well_count += not_equal(prod1.productionControls(), prod2.productionControls(), well_msg(well1.name(), "Prod: productionControls"));
            if (well1.getStatus() == Well::Status::OPEN)
                well_count += not_equal(prod1.controlMode, prod2.controlMode, well_msg(well1.name(), "Prod: controlMode"));
            well_count += not_equal(prod1.whistctl_cmode, prod2.whistctl_cmode, well_msg(well1.name(), "Prod: whistctl_cmode"));
        }
        {
            const auto& inj1 = well1.getInjectionProperties();
            const auto& inj2 = well2.getInjectionProperties();

            well_count += not_equal(inj1.name, inj2.name, well_msg(well1.name(), "Well::Inj: name"));
            well_count += not_equal(inj1.surfaceInjectionRate, inj2.surfaceInjectionRate, well_msg(well1.name(), "Well::Inj: surfaceInjectionRate"));
            well_count += not_equal(inj1.reservoirInjectionRate, inj2.reservoirInjectionRate, well_msg(well1.name(), "Well::Inj: reservoirInjectionRate"));
            well_count += not_equal(inj1.BHPTarget, inj2.BHPTarget, well_msg(well1.name(), "Well::Inj: BHPTarget"));
            well_count += not_equal(inj1.THPTarget, inj2.THPTarget, well_msg(well1.name(), "Well::Inj: THPTarget"));
            well_count += not_equal(inj1.bhp_hist_limit, inj2.bhp_hist_limit, well_msg(well1.name(), "Well::Inj: bhp_hist_limit"));
            well_count += not_equal(inj1.thp_hist_limit, inj2.thp_hist_limit, well_msg(well1.name(), "Well::Inj: thp_hist_limit"));
            well_count += not_equal(inj1.BHPH, inj2.BHPH, well_msg(well1.name(), "Well::Inj: BHPH"));
            well_count += not_equal(inj1.THPH, inj2.THPH, well_msg(well1.name(), "Well::Inj: THPH"));
            well_count += not_equal(inj1.VFPTableNumber, inj2.VFPTableNumber, well_msg(well1.name(), "Well::Inj: VFPTableNumber"));
            well_count += not_equal(inj1.predictionMode, inj2.predictionMode, well_msg(well1.name(), "Well::Inj: predictionMode"));
            well_count += not_equal(inj1.injectionControls, inj2.injectionControls, well_msg(well1.name(), "Well::Inj: injectionControls"));
            well_count += not_equal(inj1.injectorType, inj2.injectorType, well_msg(well1.name(), "Well::Inj: injectorType"));
            well_count += not_equal(inj1.controlMode, inj2.controlMode, well_msg(well1.name(), "Well::Inj: controlMode"));
        }

        {
            well_count += well2.firstTimeStep() > report_step;
            well_count += not_equal( well1.groupName(), well2.groupName(), well_msg(well1.name(), "Well: groupName"));
            well_count += not_equal( well1.getHeadI(), well2.getHeadI(), well_msg(well1.name(), "Well: getHeadI"));
            well_count += not_equal( well1.getHeadJ(), well2.getHeadJ(), well_msg(well1.name(), "Well: getHeadJ"));
            well_count += not_equal( well1.getRefDepth(), well2.getRefDepth(), well_msg(well1.name(), "Well: getRefDepth"));
            well_count += not_equal( well1.isMultiSegment(), well2.isMultiSegment() , well_msg(well1.name(), "Well: isMultiSegment"));
            well_count += not_equal( well1.isAvailableForGroupControl(), well2.isAvailableForGroupControl() , well_msg(well1.name(), "Well: isAvailableForGroupControl"));
            well_count += not_equal( well1.getGuideRate(), well2.getGuideRate(), well_msg(well1.name(), "Well: getGuideRate"));
            well_count += not_equal( well1.getGuideRatePhase(), well2.getGuideRatePhase(), well_msg(well1.name(), "Well: getGuideRatePhase"));
            well_count += not_equal( well1.getGuideRateScalingFactor(), well2.getGuideRateScalingFactor(), well_msg(well1.name(), "Well: getGuideRateScalingFactor"));
            well_count += not_equal( well1.predictionMode(), well2.predictionMode(), well_msg(well1.name(), "Well: predictionMode"));
            well_count += not_equal( well1.canOpen(), well2.canOpen(), well_msg(well1.name(), "Well: canOpen"));
            well_count += not_equal( well1.isProducer(), well2.isProducer(), well_msg(well1.name(), "Well: isProducer"));
            well_count += not_equal( well1.isInjector(), well2.isInjector(), well_msg(well1.name(), "Well: isInjector"));
            if (well1.isInjector())
                well_count += not_equal( well1.injectorType(), well2.injectorType(), well_msg(well1.name(), "Well1: injectorType"));
            well_count += not_equal( well1.seqIndex(), well2.seqIndex(), well_msg(well1.name(), "Well: seqIndex"));
            well_count += not_equal( well1.getAutomaticShutIn(), well2.getAutomaticShutIn(), well_msg(well1.name(), "Well: getAutomaticShutIn"));
            well_count += not_equal( well1.getAllowCrossFlow(), well2.getAllowCrossFlow(), well_msg(well1.name(), "Well: getAllowCrossFlow"));
            well_count += not_equal( well1.getSolventFraction(), well2.getSolventFraction(), well_msg(well1.name(), "Well: getSolventFraction"));
            well_count += not_equal( well1.getStatus(), well2.getStatus(), well_msg(well1.name(), "Well: getStatus"));
            //well_count += not_equal( well1.getInjectionProperties(), well2.getInjectionProperties(), "Well: getInjectionProperties");


            if (well1.isProducer())
                well_count += not_equal( well1.getPreferredPhase(), well2.getPreferredPhase(), well_msg(well1.name(), "Well: getPreferredPhase"));
            well_count += not_equal( well1.getDrainageRadius(), well2.getDrainageRadius(), well_msg(well1.name(), "Well: getDrainageRadius"));
            well_count += not_equal( well1.getEfficiencyFactor(), well2.getEfficiencyFactor(), well_msg(well1.name(), "Well: getEfficiencyFactor"));
        }
        count += well_count;
        if (well_count > 0)
            std::cerr << std::endl;
    }
    return (count == 0);
}

const ScheduleState& Schedule::back() const {
    return this->snapshots.back();
}

const ScheduleState& Schedule::operator[](std::size_t index) const {
    return this->snapshots.at(index);
}

std::vector<ScheduleState>::const_iterator Schedule::begin() const {
    return this->snapshots.begin();
}

std::vector<ScheduleState>::const_iterator Schedule::end() const {
    return this->snapshots.end();
}

void Schedule::create_first(const time_point& start_time, const std::optional<time_point>& end_time) {
    if (end_time.has_value())
        this->snapshots.emplace_back( start_time, end_time.value() );
    else
        this->snapshots.emplace_back(start_time);

    const auto& runspec = this->m_static.m_runspec;
    auto& sched_state = snapshots.back();
    sched_state.init_nupcol( runspec.nupcol() );
    sched_state.update_oilvap( OilVaporizationProperties( runspec.tabdims().getNumPVTTables() ));
    sched_state.update_message_limits( this->m_static.m_deck_message_limits );
    sched_state.pavg.update( PAvg() );
    sched_state.wtest_config.update( WellTestConfig() );
    sched_state.gconsale.update( GConSale() );
    sched_state.gconsump.update( GConSump() );
    sched_state.wlist_manager.update( WListManager() );
    sched_state.network.update( Network::ExtNetwork() );
    sched_state.rpt_config.update( RPTConfig() );
    sched_state.actions.update( Action::Actions() );
    sched_state.udq_active.update( UDQActive() );
    sched_state.well_order.update( NameOrder() );
    sched_state.group_order.update( GroupOrder( runspec.wellDimensions().maxGroupsInField()) );
    sched_state.udq.update( UDQConfig( runspec.udqParams() ));
    sched_state.glo.update( GasLiftOpt() );
    sched_state.guide_rate.update( GuideRateConfig() );
    sched_state.rft_config.update( RFTConfig() );
    sched_state.rst_config.update( RSTConfig::first( this->m_static.rst_config ) );
    sched_state.network_balance.update(Network::Balance{ runspec.networkDimensions().active() });
    sched_state.update_sumthin(this->m_static.sumthin);
    sched_state.rptonly(this->m_static.rptonly);
    //sched_state.update_date( start_time );
    this->addGroup("FIELD", 0);
}

void Schedule::create_next(const time_point& start_time, const std::optional<time_point>& end_time) {
    if (this->snapshots.empty())
        this->create_first(start_time, end_time);
    else {
        const auto& last = this->snapshots.back();
        if (end_time.has_value())
            this->snapshots.emplace_back( last, start_time, end_time.value() );
        else
            this->snapshots.emplace_back( last, start_time );
    }
}


void Schedule::create_next(const ScheduleBlock& block) {
    const auto& start_time = block.start_time();
    const auto& end_time = block.end_time();
    this->create_next(start_time, end_time);
}

void Schedule::dump_deck(std::ostream& os) const {
    this->m_sched_deck.dump_deck(os);
}

std::ostream& operator<<(std::ostream& os, const Schedule& sched)
{
    sched.dump_deck(os);
    return os;
}

}

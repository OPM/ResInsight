#ifndef ISIM_MAIN_HPP
#define ISIM_MAIN_HPP

#include <chrono>
#include <functional>
#include <map>
#include <string>

#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>

#include <opm/output/data/Solution.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>
#include <opm/input/eclipse/Deck/UDAValue.hpp>


namespace Opm {

class EclipseIO;
class ParseContext;
class Parser;
class Python;
class SummaryState;
class UDQState;
class WellTestState;

class msim {

public:
    using well_rate_function = double(const EclipseState&, const Schedule&, const SummaryState& st, const data::Solution&, size_t report_step, double seconds_elapsed);
    using solution_function = void(const EclipseState&, const Schedule&, data::Solution&, size_t report_step, double seconds_elapsed);

    msim(const EclipseState& state, const Schedule& schedule_arg);

    Opm::UDAValue uda_val();

    void well_rate(const std::string& well, data::Rates::opt rate, std::function<well_rate_function> func);
    void solution(const std::string& field, std::function<solution_function> func);
    void run(EclipseIO& io, bool report_only);
    void post_step(data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& group_nwrk_data, size_t report_step, const time_point& sim_time);

private:

    void run_step(WellTestState& wtest_state, UDQState& udq_state, data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& group_nwrk_data, size_t report_step, EclipseIO& io);
    void run_step(WellTestState& wtest_state, UDQState& udq_state, data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& group_nwrk_data, size_t report_step, double dt, EclipseIO& io);
    void output(WellTestState& wtest_state, const UDQState& udq_state, size_t report_step, bool substep, double seconds_elapsed, const data::Solution& sol, const data::Wells& well_data, const data::GroupAndNetworkValues& group_data, EclipseIO& io);
    void simulate(data::Solution& sol, data::Wells& well_data, data::GroupAndNetworkValues& group_nwrk_data, size_t report_step, double seconds_elapsed, double time_step);

    EclipseState state;
    std::map<std::string, std::map<data::Rates::opt, std::function<well_rate_function>>> well_rates;
    std::map<std::string, std::function<solution_function>> solutions;

public:
    Schedule schedule;
    Action::State action_state;
    SummaryState st;
};
}


#endif

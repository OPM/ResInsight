/*
  Copyright 2019 Equinor ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#include <opm/common/utility/TimeService.hpp>

#include <chrono>
#include <ctime>
#include <utility>

namespace Opm {
namespace TimeService {

namespace {
    const std::unordered_map<std::string, int> month_indices = {
        {"JAN", 1},
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


const time_t system_clock_epoch = std::chrono::system_clock::to_time_t({});

time_point from_time_t(std::time_t t) {
    auto diff = std::difftime(t, system_clock_epoch);
    return time_point(std::chrono::seconds(static_cast<std::chrono::seconds::rep>(diff)));
}

std::time_t to_time_t(const time_point& tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count() + system_clock_epoch;
}


time_point now() {
    time_point epoch;
    auto default_now = std::chrono::system_clock::now();
    return epoch + std::chrono::duration_cast<Opm::time_point::duration>(default_now.time_since_epoch());
}

std::time_t advance(const std::time_t tp, const double sec)
{
    const auto t = Opm::TimeService::from_time_t(tp) + std::chrono::duration_cast<Opm::time_point::duration>(std::chrono::duration<double>(sec));
    return Opm::TimeService::to_time_t(t);
}

std::time_t makeUTCTime(std::tm timePoint)
{
    const auto ltime =  std::mktime(&timePoint);
    auto       tmval = *std::gmtime(&ltime); // Mutable.

    // offset =  ltime - tmval
    //        == #seconds by which 'ltime' is AHEAD of tmval.
    const auto offset =
        std::difftime(ltime, std::mktime(&tmval));

    // Advance 'ltime' by 'offset' so that std::gmtime(return value) will
    // have the same broken-down elements as 'tp'.
    return advance(ltime, offset);
}

const std::unordered_map<std::string , int>& eclipseMonthIndices() {
    return month_indices;
}

bool valid_month(const std::string& month_name) {
    return (month_indices.count(month_name) != 0);
}


}
}

namespace {



    std::tm makeTm(const Opm::TimeStampUTC& tp) {
        auto timePoint = std::tm{};

        timePoint.tm_year = tp.year()  - 1900;
        timePoint.tm_mon  = tp.month() -    1;
        timePoint.tm_mday = tp.day();
        timePoint.tm_hour = tp.hour();
        timePoint.tm_min  = tp.minutes();
        timePoint.tm_sec  = tp.seconds();

        return timePoint;
    }


}

Opm::TimeStampUTC::TimeStampUTC(const std::time_t tp)
{
    auto t = tp;
    const auto tm = *std::gmtime(&t);

    this->ymd_ = YMD { tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday };

    this->hour(tm.tm_hour).minutes(tm.tm_min).seconds(tm.tm_sec);
}

Opm::TimeStampUTC::TimeStampUTC(const Opm::TimeStampUTC::YMD& ymd,
                                int hour, int minutes, int seconds, int usec)
    : ymd_(ymd)
    , hour_(hour)
    , minutes_(minutes)
    , seconds_(seconds)
    , usec_(usec)
{}

Opm::TimeStampUTC& Opm::TimeStampUTC::operator=(const std::time_t tp)
{
    auto t = tp;
    const auto tm = *std::gmtime(&t);

    this->ymd_ = YMD { tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday };

    this->hour(tm.tm_hour).minutes(tm.tm_min).seconds(tm.tm_sec);

    return *this;
}

bool Opm::TimeStampUTC::operator==(const TimeStampUTC& data) const
{
    return ymd_ == data.ymd_ &&
           hour_ == data.hour_ &&
           minutes_ == data.minutes_ &&
           seconds_ == data.seconds_ &&
           usec_ == data.usec_;
}

Opm::TimeStampUTC::TimeStampUTC(const YMD& ymd)
    : ymd_{ std::move(ymd) }
{}

Opm::TimeStampUTC::TimeStampUTC(int year, int month, int day)
    : ymd_{ year, month, day }
{}

Opm::TimeStampUTC& Opm::TimeStampUTC::hour(const int h)
{
    this->hour_ = h;
    return *this;
}

Opm::TimeStampUTC& Opm::TimeStampUTC::minutes(const int m)
{
    this->minutes_ = m;
    return *this;
}

Opm::TimeStampUTC& Opm::TimeStampUTC::seconds(const int s)
{
    this->seconds_ = s;
    return *this;
}

Opm::TimeStampUTC& Opm::TimeStampUTC::microseconds(const int us)
{
    this->usec_ = us;
    return *this;
}


std::time_t Opm::asTimeT(const TimeStampUTC& tp)
{
    return Opm::TimeService::makeUTCTime(makeTm(tp));
}

std::time_t Opm::asLocalTimeT(const TimeStampUTC& tp)
{
    auto tm = makeTm(tp);
    return std::mktime(&tm);
}

Opm::TimeStampUTC Opm::operator+(const Opm::TimeStampUTC& lhs, std::chrono::duration<double> delta) {
    return Opm::TimeStampUTC( Opm::TimeService::advance(Opm::asTimeT(lhs) , delta.count()) );
}



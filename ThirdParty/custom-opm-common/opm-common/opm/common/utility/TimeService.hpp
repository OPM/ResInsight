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

#ifndef OPM_TIMESERVICE_HEADER_INCLUDED
#define OPM_TIMESERVICE_HEADER_INCLUDED

#include <chrono>
#include <ctime>

namespace Opm {

    class TimeStampUTC
    {
    public:
        struct YMD {
            int year{0};
            int month{0};
            int day{0};

            bool operator==(const YMD& data) const
            {
                return year == data.year &&
                       month == data.month &&
                       day == data.day;
            }

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(year);
                serializer(month);
                serializer(day);
            }
        };

        TimeStampUTC() = default;

        explicit TimeStampUTC(const std::time_t tp);
        explicit TimeStampUTC(const YMD& ymd);
        TimeStampUTC(int year, int month, int day);
        TimeStampUTC(const YMD& ymd,
                     int hour,
                     int minutes,
                     int seconds,
                     int usec);

        TimeStampUTC& operator=(const std::time_t tp);
        bool operator==(const TimeStampUTC& data) const;

        TimeStampUTC& hour(const int h);
        TimeStampUTC& minutes(const int m);
        TimeStampUTC& seconds(const int s);
        TimeStampUTC& microseconds(const int us);

        const YMD& ymd() const { return ymd_; }
        int year()         const { return this->ymd_.year;  }
        int month()        const { return this->ymd_.month; }
        int day()          const { return this->ymd_.day;   }
        int hour()         const { return this->hour_;      }
        int minutes()      const { return this->minutes_;   }
        int seconds()      const { return this->seconds_;   }
        int microseconds() const { return this->usec_;      }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            ymd_.serializeOp(serializer);
            serializer(hour_);
            serializer(minutes_);
            serializer(seconds_);
            serializer(usec_);
        }

    private:

        YMD ymd_{};
        int hour_{0};
        int minutes_{0};
        int seconds_{0};
        int usec_{0};
    };

    TimeStampUTC operator+(const TimeStampUTC& lhs, std::chrono::duration<double> delta);
    std::time_t asTimeT(const TimeStampUTC& tp);
    std::time_t asLocalTimeT(const TimeStampUTC& tp);

} // namespace Opm

#endif // OPM_TIMESERVICE_HEADER_INCLUDED

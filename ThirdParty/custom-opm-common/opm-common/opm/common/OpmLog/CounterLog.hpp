/*
  Copyright 2014 Andreas Lauser

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
#ifndef OPM_COUNTERLOG_HPP
#define OPM_COUNTERLOG_HPP

#include <string>
#include <memory>
#include <map>

#include <opm/common/OpmLog/LogBackend.hpp>

namespace Opm {
/*!
 * \brief Provides a simple sytem for log message which are found by the
 *        Parser/Deck/EclipseState classes during processing the deck.
 */
    class CounterLog : public LogBackend
    {
    public:
        explicit CounterLog(int64_t messageMask);
        CounterLog();

        size_t numMessages(int64_t messageType) const;

        void clear();

    protected:
        void addMessageUnconditionally(int64_t messageFlag,
                                       const std::string& message) override;
    private:
        std::map<int64_t , size_t> m_count;
    };

} // namespace Opm

#endif


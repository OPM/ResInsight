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
#include <cmath>
#include <limits>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Completion.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>

namespace Opm {

    CompletionSet::CompletionSet() {}


    size_t CompletionSet::size() const {
        return m_completions.size();
    }


    CompletionConstPtr CompletionSet::get(size_t index) const {
        if (index >= m_completions.size())
            throw std::range_error("Out of bounds");

        return m_completions[index];
    }


    CompletionConstPtr CompletionSet::getFromIJK(const int i, const int j, const int k) const {
        for (size_t ic = 0; ic < size(); ++ic) {
            if (get(ic)->sameCoordinate(i, j, k)) {
                return get(ic);
            }
        }
        throw std::runtime_error(" the completion is not found! \n ");
    }


    void CompletionSet::add(CompletionConstPtr completion) {
        bool inserted = false;

        for (size_t ic = 0; ic < m_completions.size(); ic++) {
            CompletionConstPtr current = m_completions[ic];
            if (current->sameCoordinate( *completion )) {
                m_completions[ic] = completion;
                inserted = true;
            }
        }

        if (!inserted)
            m_completions.push_back( completion );
    }


    CompletionSet * CompletionSet::shallowCopy() const {
        CompletionSet * copy = new CompletionSet();
        for (size_t ic = 0; ic < m_completions.size(); ic++) {
            CompletionConstPtr completion = m_completions[ic];
            copy->m_completions.push_back( completion );
        }
        return copy;
    }


    bool CompletionSet::allCompletionsShut( ) const {
      bool allShut = true;
      for (auto ci = m_completions.begin(); ci != m_completions.end(); ++ci) {
          CompletionConstPtr completion_ptr = *ci;
          if (completion_ptr->getState() != WellCompletion::StateEnum::SHUT) {
              allShut = false;
              break;
          }
      }
      return allShut;
    }



    void CompletionSet::orderCompletions(size_t well_i, size_t well_j)
    {
        if (m_completions.empty()) {
            return;
        }

        // Find the first completion and swap it into the 0-position.
        const double surface_z = 0.0;
        size_t first_index = findClosestCompletion(well_i, well_j, surface_z, 0);
        std::swap(m_completions[first_index], m_completions[0]);

        // Repeat for remaining completions.
        //
        // Note that since findClosestCompletion() is O(n), this is an
        // O(n^2) algorithm. However, it should be acceptable since
        // the expected number of completions is fairly low (< 100).

        for (size_t pos = 1; pos < m_completions.size() - 1; ++pos) {
            CompletionConstPtr prev = m_completions[pos - 1];
            const double prevz = prev->getCenterDepth();
            size_t next_index = findClosestCompletion(prev->getI(), prev->getJ(), prevz, pos);
            std::swap(m_completions[next_index], m_completions[pos]);
        }
    }



    size_t CompletionSet::findClosestCompletion(int oi, int oj, double oz, size_t start_pos)
    {
        size_t closest = std::numeric_limits<size_t>::max();
        int min_ijdist2 = std::numeric_limits<int>::max();
        double min_zdiff = std::numeric_limits<double>::max();
        for (size_t pos = start_pos; pos < m_completions.size(); ++pos) {
            const double depth = m_completions[pos]->getCenterDepth();
            const int ci = m_completions[pos]->getI();
            const int cj = m_completions[pos]->getJ();
            // Using square of distance to avoid non-integer arithmetics.
            const int ijdist2 = (ci - oi) * (ci - oi) + (cj - oj) * (cj - oj);
            if (ijdist2 < min_ijdist2) {
                min_ijdist2 = ijdist2;
                min_zdiff = std::abs(depth - oz);
                closest = pos;
            } else if (ijdist2 == min_ijdist2) {
                const double zdiff = std::abs(depth - oz);
                if (zdiff < min_zdiff) {
                    min_zdiff = zdiff;
                    closest = pos;
                }
            }
        }
        assert(closest != std::numeric_limits<size_t>::max());
        return closest;
    }

}

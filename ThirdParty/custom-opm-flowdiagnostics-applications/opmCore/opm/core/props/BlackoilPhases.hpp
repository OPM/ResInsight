/*
  Copyright 2010, 2011, 2012 SINTEF ICT, Applied Mathematics.
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

#ifndef OPM_BLACKOILPHASES_HEADER_INCLUDED
#define OPM_BLACKOILPHASES_HEADER_INCLUDED


namespace Opm
{

    class BlackoilPhases
    {
    public:
        static const int MaxNumPhases = 3;
        // enum ComponentIndex { Water = 0, Oil = 1, Gas = 2 };
        enum PhaseIndex { Aqua = 0, Liquid = 1, Vapour = 2 };

    };

    struct PhaseUsage : public BlackoilPhases
    {
        int num_phases;
        int phase_used[MaxNumPhases];
        int phase_pos[MaxNumPhases];
    };

    /// Check or assign presence of a formed, free phase.  Limited to
    /// the 'BlackoilPhases'.
    ///
    /// Use a std::vector<PhasePresence> to represent the conditions
    /// in an entire model.
    class PhasePresence
    {
    public:
        PhasePresence()
            : present_(0)
        {}

        bool hasFreeWater() const { return present(BlackoilPhases::Aqua  ); }
        bool hasFreeOil  () const { return present(BlackoilPhases::Liquid); }
        bool hasFreeGas  () const { return present(BlackoilPhases::Vapour); }

        void setFreeWater() { insert(BlackoilPhases::Aqua  ); }
        void setFreeOil  () { insert(BlackoilPhases::Liquid); }
        void setFreeGas  () { insert(BlackoilPhases::Vapour); }

        bool operator==(const PhasePresence& other) const { return present_ == other.present_; }
        bool operator!=(const PhasePresence& other) const { return !this->operator==(other); }

    private:
        unsigned char present_;

        bool present(const BlackoilPhases::PhaseIndex i) const
        {
            return present_ & (1 << i);
        }

        void insert(const BlackoilPhases::PhaseIndex i)
        {
            present_ |= (1 << i);
        }
    };

} // namespace Opm

#endif // OPM_BLACKOILPHASES_HEADER_INCLUDED
/*
  Copyright 2015 Statoil ASA.

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

#ifndef OPM_TRESHOLD_PRESSURES_HPP
#define OPM_TRESHOLD_PRESSURES_HPP

#include <map>
#include <vector>

namespace Opm {


    class Deck;
    class FieldPropsManager;

    class ThresholdPressure {

    public:
        using ThresholdPressureTable = std::vector<std::pair<bool,double>>;
        using PressureTable = std::map<std::pair<int,int>,std::pair<bool,double>>;

        ThresholdPressure(bool restart,
                          const Deck& deck,
                          const FieldPropsManager& fp);

        ThresholdPressure()
            : m_active(false)
            , m_restart(false)
            , m_irreversible(false)
        {}

        //! \brief Returns an instance for serialization tests.
        static ThresholdPressure serializeObject();

        /*
          The hasRegionBarrier() method checks if a threshold pressure
          has been configured between the equilibration regions r1 and
          r2; i.e. if the deck contains a THPRES record with regions
          r1 and r2.
        */
        bool   hasRegionBarrier(int r1 , int r2) const;

        /*
          Checks if a threshold presssure has been configured between
          the equilibration regions r1 and r2; the function will
          return false either if no THPRES record with r1 and r2 has
          been configured - or if THPRES record with ra and r2 has
          defaulted pressure.
        */
        bool   hasThresholdPressure(int r1 , int r2) const;

        /*
          Will return the threshold pressure between equilibration
          regions r1 and r2; if the pressure has been defaulted the
          function will raise the error
          INTERNAL_ERROR_UNINITIALIZED_THPRES - check with
          hasThresholdPressure(r1,r2) first to be safe.
        */
        double getThresholdPressure(int r1 , int r2) const;
        size_t size() const;
        bool active() const;
        bool restart() const;

        bool operator==(const ThresholdPressure& data) const;
        static bool rst_cmp(const ThresholdPressure& full_arg, const ThresholdPressure& rst_arg);


        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_active);
            serializer(m_restart);
            serializer(m_irreversible);
            serializer(m_thresholdPressureTable);
            serializer(m_pressureTable);
        }

    private:
        bool m_active;
        bool m_restart;
        bool m_irreversible;
        std::pair<int,int> makeIndex(int r1 , int r2) const;
        void addPair(int r1 , int r2 , const std::pair<bool , double>& valuePair);
        void addBarrier(int r1 , int r2);
        void addBarrier(int r1 , int r2 , double p);

        std::vector<std::pair<bool,double>> m_thresholdPressureTable;
        std::map<std::pair<int,int> , std::pair<bool , double> > m_pressureTable;
    };
} //namespace Opm

#endif

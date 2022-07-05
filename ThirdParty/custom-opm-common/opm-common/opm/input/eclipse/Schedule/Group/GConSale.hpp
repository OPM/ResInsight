/*
  Copyright 2019 Equinor ASA.

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

#ifndef GCONSALE_H
#define GCONSALE_H

#include <map>
#include <string>

#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

namespace Opm {

    class SummaryState;

    class GConSale {
    public:

        enum class MaxProcedure {
            NONE, CON, CON_P, WELL, PLUG, RATE, MAXR, END
        };

        struct GCONSALEGroup {
            UDAValue sales_target;
            UDAValue max_sales_rate;
            UDAValue min_sales_rate;
            MaxProcedure max_proc;
            double udq_undefined;
            UnitSystem unit_system;

            bool operator==(const GCONSALEGroup& data) const {
                return sales_target == data.sales_target &&
                       max_sales_rate == data.max_sales_rate &&
                       min_sales_rate == data.min_sales_rate &&
                       max_proc == data.max_proc &&
                       udq_undefined == data.udq_undefined &&
                       unit_system == data.unit_system;
            }

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                sales_target.serializeOp(serializer);
                max_sales_rate.serializeOp(serializer);
                min_sales_rate.serializeOp(serializer);
                serializer(max_proc);
                serializer(udq_undefined);
                unit_system.serializeOp(serializer);
            }
        };

        struct GCONSALEGroupProp {
            double sales_target;
            double max_sales_rate;
            double min_sales_rate;
            MaxProcedure max_proc;
        };

        static GConSale serializeObject();
        
        bool has(const std::string& name) const;
        const GCONSALEGroup& get(const std::string& name) const;
        const GCONSALEGroupProp get(const std::string& name, const SummaryState& st) const;
        static MaxProcedure stringToProcedure(const std::string& procedure);
        void add(const std::string& name, const UDAValue& sales_target, const UDAValue& max_rate, const UDAValue& min_rate, const std::string& procedure, double udq_undefined_arg, const UnitSystem& unit_system);
        size_t size() const;

        bool operator==(const GConSale& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer.map(groups);
        }

    private:
        std::map<std::string, GCONSALEGroup> groups;
    };

}


#endif

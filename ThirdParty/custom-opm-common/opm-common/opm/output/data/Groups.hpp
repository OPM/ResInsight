/*
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_OUTPUT_GROUPS_HPP
#define OPM_OUTPUT_GROUPS_HPP

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <opm/output/data/GuideRateValue.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>

namespace Opm { namespace data {

    struct GroupConstraints {
        Opm::Group::ProductionCMode currentProdConstraint;
        Opm::Group::InjectionCMode  currentGasInjectionConstraint;
        Opm::Group::InjectionCMode  currentWaterInjectionConstraint;

        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const;

        template <class MessageBufferType>
        void read(MessageBufferType& buffer);

        bool operator==(const GroupConstraints& other) const
        {
            return this->currentProdConstraint == other.currentProdConstraint &&
                   this->currentGasInjectionConstraint == other.currentGasInjectionConstraint &&
                   this->currentWaterInjectionConstraint == other.currentWaterInjectionConstraint;
        }

        inline GroupConstraints& set(Opm::Group::ProductionCMode cpc,
                                     Opm::Group::InjectionCMode  cgic,
                                     Opm::Group::InjectionCMode  cwic);
    };

    struct GroupGuideRates {
        GuideRateValue production{};
        GuideRateValue injection{};

        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const
        {
            this->production.write(buffer);
            this->injection .write(buffer);
        }

        template <class MessageBufferType>
        void read(MessageBufferType& buffer)
        {
            this->production.read(buffer);
            this->injection .read(buffer);
        }

        bool operator==(const GroupGuideRates& other) const
        {
            return this->production == other.production
                && this->injection  == other.injection;
        }
    };

    struct GroupData {
        GroupConstraints currentControl;
        GroupGuideRates  guideRates{};

        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const
        {
            this->currentControl.write(buffer);
            this->guideRates    .write(buffer);
        }

        template <class MessageBufferType>
        void read(MessageBufferType& buffer)
        {
            this->currentControl.read(buffer);
            this->guideRates    .read(buffer);
        }

        bool operator==(const GroupData& other) const
        {
            return this->currentControl == other.currentControl
                && this->guideRates     == other.guideRates;
        }
    };

    class GroupValues : public std::map<std::string, GroupData>  {
    public:
        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const
        {
            unsigned int size = this->size();
            buffer.write(size);

            for (const auto& [gname, gdata] : *this) {
                buffer.write(gname);
                gdata .write(buffer);
            }
        }

        template <class MessageBufferType>
        void read(MessageBufferType& buffer)
        {
            unsigned int size;
            buffer.read(size);

            for (size_t i = 0; i < size; ++i) {
                std::string name;
                buffer.read(name);

                auto gdata = GroupData{};
                gdata.read(buffer);

                this->emplace(name, gdata);
            }
        }
    };

    /* IMPLEMENTATIONS */

    template <class MessageBufferType>
    void GroupConstraints::write(MessageBufferType& buffer) const {
        buffer.write(this->currentProdConstraint);
        buffer.write(this->currentGasInjectionConstraint);
        buffer.write(this->currentWaterInjectionConstraint);
    }

    template <class MessageBufferType>
    void GroupConstraints::read(MessageBufferType& buffer) {
        buffer.read(this->currentProdConstraint);
        buffer.read(this->currentGasInjectionConstraint);
        buffer.read(this->currentWaterInjectionConstraint);
    }

    inline GroupConstraints&
    GroupConstraints::set(Opm::Group::ProductionCMode cpc,
                          Opm::Group::InjectionCMode  cgic,
                          Opm::Group::InjectionCMode  cwic)
    {
        this->currentGasInjectionConstraint = cgic;
        this->currentWaterInjectionConstraint = cwic;
        this->currentProdConstraint = cpc;

        return *this;
    }

}} // Opm::data

#endif //OPM_OUTPUT_GROUPS_HPP

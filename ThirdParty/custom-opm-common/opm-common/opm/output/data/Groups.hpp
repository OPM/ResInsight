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

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>

namespace Opm {

    namespace data {

    struct currentGroupConstraints {
        Opm::Group::ProductionCMode currentProdConstraint;
        Opm::Group::InjectionCMode  currentGasInjectionConstraint;
        Opm::Group::InjectionCMode  currentWaterInjectionConstraint;

        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const;
        template <class MessageBufferType>
        void read(MessageBufferType& buffer);

        inline currentGroupConstraints& set(  Opm::Group::ProductionCMode cpc,
        Opm::Group::InjectionCMode  cgic,
        Opm::Group::InjectionCMode  cwic);
    };


    class Group : public std::map<std::string, Opm::data::currentGroupConstraints>  {
    public:

        template <class MessageBufferType>
        void write(MessageBufferType& buffer) const {
            unsigned int size = this->size();
            buffer.write(size);
            for (const auto& witr : *this) {
                const std::string& name = witr.first;
                buffer.write(name);
                const auto& pi_constr = witr.second;
                pi_constr.write(buffer);
            }
        }

        template <class MessageBufferType>
        void read(MessageBufferType& buffer) {
             unsigned int size;
            buffer.read(size);
            for (size_t i = 0; i < size; ++i) {
                std::string name;
                buffer.read(name);
                currentGroupConstraints cgc;
                cgc.read(buffer);
                this->emplace(name, cgc);
            }
        }
    };

    /* IMPLEMENTATIONS */

    template <class MessageBufferType>
    void currentGroupConstraints::write(MessageBufferType& buffer) const {
        buffer.write(this->currentProdConstraint);
        buffer.write(this->currentGasInjectionConstraint);
        buffer.write(this->currentWaterInjectionConstraint);
    }

    template <class MessageBufferType>
    void currentGroupConstraints::read(MessageBufferType& buffer) {
        buffer.read(this->currentProdConstraint);
        buffer.read(this->currentGasInjectionConstraint);
        buffer.read(this->currentWaterInjectionConstraint);
    }


    inline currentGroupConstraints& currentGroupConstraints::set(  Opm::Group::ProductionCMode cpc,
        Opm::Group::InjectionCMode  cgic,
        Opm::Group::InjectionCMode  cwic) {
        this->currentGasInjectionConstraint = cgic;
        this->currentWaterInjectionConstraint = cwic;
        this->currentProdConstraint = cpc;
        return *this;
    }

}} // Opm::data

#endif //OPM_OUTPUT_GROUPS_HPP

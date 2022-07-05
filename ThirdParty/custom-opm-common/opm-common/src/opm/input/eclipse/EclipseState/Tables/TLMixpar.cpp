/*
  Copyright 2020 Statoil ASA.

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


#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TLMixpar.hpp>

namespace Opm {

TLMixpar::TLMixpar(const Deck& deck) {
    using TLM = ParserKeywords::TLMIXPAR;
    if (!deck.hasKeyword<TLM>())
        return;

    const auto& keyword = deck.get<TLM>().back();
    for (const auto& record : keyword) {
        const double viscosity_parameter = record.getItem<TLM::TL_VISCOSITY_PARAMETER>().getSIDouble(0);
        double density_parameter = viscosity_parameter;
        const auto& density_item = record.getItem<TLM::TL_DENSITY_PARAMETER>();
        if (density_item.hasValue(0))
            density_parameter = density_item.getSIDouble(0);

        this->data.emplace_back(viscosity_parameter, density_parameter);
    }
}


bool TLMixpar::empty() const {
    return this->data.empty();
}

std::size_t TLMixpar::size() const {
    return this->data.size();
}

TLMixpar TLMixpar::serializeObject() {
    TLMixpar tlm;
    return tlm;
}


const TLMixRecord& TLMixpar::operator[](const std::size_t index) const {
    return this->data.at(index);
}

}

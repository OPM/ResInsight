/*
  Copyright 2015 IRIS
  
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
#include <array>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/N.hpp>


namespace Opm
{
    /// [[deprecated]]
    NNC::NNC(std::shared_ptr<const Deck> deck, EclipseGridConstPtr eclipseGrid) :
            NNC(*deck, eclipseGrid)
    {}

    NNC::NNC(const Deck& deck, EclipseGridConstPtr eclipseGrid) {
        const auto& nncs = deck.getKeywordList<ParserKeywords::NNC>();
        for (size_t idx_nnc = 0; idx_nnc<nncs.size(); ++idx_nnc) {
            const auto& nnc = *nncs[idx_nnc];
            size_t numNNC = nnc.size();
            for (size_t i = 0; i<numNNC; ++i) {
                std::array<size_t, 3> ijk1;
                ijk1[0] = static_cast<size_t>(nnc.getRecord(i).getItem(0).get< int >(0)-1);
                ijk1[1] = static_cast<size_t>(nnc.getRecord(i).getItem(1).get< int >(0)-1);
                ijk1[2] = static_cast<size_t>(nnc.getRecord(i).getItem(2).get< int >(0)-1);
                size_t global_index1 = eclipseGrid->getGlobalIndex(ijk1[0],ijk1[1],ijk1[2]);
                
                std::array<size_t, 3> ijk2;
                ijk2[0] = static_cast<size_t>(nnc.getRecord(i).getItem(3).get< int >(0)-1);
                ijk2[1] = static_cast<size_t>(nnc.getRecord(i).getItem(4).get< int >(0)-1);
                ijk2[2] = static_cast<size_t>(nnc.getRecord(i).getItem(5).get< int >(0)-1);
                size_t global_index2 = eclipseGrid->getGlobalIndex(ijk2[0],ijk2[1],ijk2[2]);
                
                const double trans = nnc.getRecord(i).getItem(6).getSIDouble(0);
                
                addNNC(global_index1,global_index2,trans);
            }
        }
    }

    // default constructor
    NNC::NNC() {
    }


    void NNC::addNNC(const size_t cell1, const size_t cell2, const double trans) {
        NNCdata nncdata;
        nncdata.cell1 = cell1;
        nncdata.cell2 = cell2;
        nncdata.trans = trans;
        m_nnc.push_back(nncdata);
    }

    size_t NNC::numNNC() const {
        return(m_nnc.size());
    }

    bool NNC::hasNNC() const {
        return m_nnc.size()>0;
    }

} // namespace Opm


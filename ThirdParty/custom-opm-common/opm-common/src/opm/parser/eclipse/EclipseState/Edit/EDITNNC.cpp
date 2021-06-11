
#include <array>
#include <sstream>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridDims.hpp>
#include <opm/parser/eclipse/EclipseState/Edit/EDITNNC.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>


namespace Opm
{

bool isNeighbor(const std::array<size_t, 3>& ijk1, const std::array<size_t, 3>& ijk2)
{
    if ( (ijk1[0] + 1) == ijk2[0] || (ijk1[0] - 1) == ijk2[0] )
    {
        return ijk1[1] == ijk2[1] && ijk1[2] == ijk2[2];
    }
    if  ( (ijk1[1] + 1) == ijk2[1] || (ijk1[1] - 1) == ijk2[1] )
    {
        return ijk1[0] == ijk2[0] && ijk1[2] == ijk2[2];
    }
    if( (ijk1[2] + 1) == ijk2[2] || (ijk1[2] - 1) == ijk2[2] )
    {
        return ijk1[1] == ijk2[1] && ijk1[1] == ijk2[1];
    }
    return false;
}

void readEditNncs(const std::vector< const DeckKeyword* >& editNncsKw, std::vector<NNCdata>& editNncs, const GridDims& gridDims)
{
    for (size_t idx_nnc = 0; idx_nnc<editNncsKw.size(); ++idx_nnc) {
        const auto& nnc = *editNncsKw[idx_nnc];
        editNncs.reserve(editNncs.size()+nnc.size());
        for (size_t i = 0; i < nnc.size(); ++i) {
            std::array<size_t, 3> ijk1;
            ijk1[0] = static_cast<size_t>(nnc.getRecord(i).getItem(0).get< int >(0)-1);
            ijk1[1] = static_cast<size_t>(nnc.getRecord(i).getItem(1).get< int >(0)-1);
            ijk1[2] = static_cast<size_t>(nnc.getRecord(i).getItem(2).get< int >(0)-1);
            size_t global_index1 = gridDims.getGlobalIndex(ijk1[0],ijk1[1],ijk1[2]);
                
            std::array<size_t, 3> ijk2;
            ijk2[0] = static_cast<size_t>(nnc.getRecord(i).getItem(3).get< int >(0)-1);
            ijk2[1] = static_cast<size_t>(nnc.getRecord(i).getItem(4).get< int >(0)-1);
            ijk2[2] = static_cast<size_t>(nnc.getRecord(i).getItem(5).get< int >(0)-1);
            size_t global_index2 = gridDims.getGlobalIndex(ijk2[0],ijk2[1],ijk2[2]);
                
            const double trans = nnc.getRecord(i).getItem(6).get<double>(0);
            using std::abs;
            if ( !isNeighbor(ijk1, ijk2) )
            {
                editNncs.emplace_back(global_index1, global_index2, trans);
            }
            else
            {
                std::ostringstream sstr;
                sstr << "Cannot edit neighboring connection from " << global_index1 <<" to "<<
                     global_index2<< " with EDITNNC";
                Opm::OpmLog::warning(sstr.str());
            }
        }
    }
}

EDITNNC::EDITNNC(const Deck& deck)
{
    GridDims gridDims(deck);
    const auto& tmpEditNncs = deck.getKeywordList<ParserKeywords::EDITNNC>();
    readEditNncs(tmpEditNncs, m_editnnc, gridDims);
    auto compare = [](const NNCdata& d1, const NNCdata& d2)
        { return d1.cell1 < d2.cell1 ||
          ( d1.cell1 == d2.cell1 && d1.cell2 < d2.cell2 );};
    std::sort(m_editnnc.begin(), m_editnnc.end(), compare);
}

EDITNNC EDITNNC::serializeObject()
{
    EDITNNC result;
    result.m_editnnc = {{1,2,1.0},{2,3,2.0}};

    return result;
}

size_t EDITNNC::size() const {
    return(m_editnnc.size());
}

bool EDITNNC::empty() const {
    return m_editnnc.empty();
}

bool EDITNNC::operator==(const EDITNNC& data) const {
    return m_editnnc == data.m_editnnc;
}

} // namespace Opm

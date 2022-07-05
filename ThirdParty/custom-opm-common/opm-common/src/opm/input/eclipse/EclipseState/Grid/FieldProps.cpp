/*
  Copyright 2019  Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/input/eclipse/EclipseState/Grid/FieldProps.hpp>

#include <functional>
#include <algorithm>
#include <unordered_map>
#include <array>
#include <vector>
#include <set>
#include <unordered_set>

#include <fmt/format.h>
#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/O.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/Tables/RtempvdTable.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/common/utility/Serializer.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/NumericalAquifer/NumericalAquifers.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>

#include "Operate.hpp"


namespace Opm {

namespace Fieldprops
{

namespace keywords {

static const std::set<std::string> oper_keywords = {"ADD", "EQUALS", "MAXVALUE", "MINVALUE", "MULTIPLY"};
static const std::set<std::string> region_oper_keywords = {"MULTIREG", "ADDREG", "EQUALREG", "OPERATER"};
static const std::set<std::string> box_keywords = {"BOX", "ENDBOX"};


std::string get_keyword_from_alias(const std::string& name) {
    if (ALIAS::aliased_keywords.count(name))
        return ALIAS::aliased_keywords.at(name);
    return name;
}


template <>
keyword_info<double> global_kw_info(const std::string& name,
                                    bool allow_unsupported) {
    if (GRID::double_keywords.count(name))
        return GRID::double_keywords.at(name);

    if (EDIT::double_keywords.count(name))
        return EDIT::double_keywords.at(name);

    if (PROPS::double_keywords.count(name))
        return PROPS::double_keywords.at(name);

    if (PROPS::satfunc.count(name))
        return keyword_info<double>{};

    if (SOLUTION::double_keywords.count(name))
        return SOLUTION::double_keywords.at(name);

    if (SCHEDULE::double_keywords.count(name))
        return SCHEDULE::double_keywords.at(name);

    if (allow_unsupported)
        return keyword_info<double>{};

    throw std::out_of_range("INFO: No such keyword: " + name);
}


template <>
keyword_info<int> global_kw_info(const std::string& name, bool) {
    if (GRID::int_keywords.count(name))
        return GRID::int_keywords.at(name);

    if (EDIT::int_keywords.count(name))
        return EDIT::int_keywords.at(name);

    if (PROPS::int_keywords.count(name))
        return PROPS::int_keywords.at(name);

    if (REGIONS::int_keywords.count(name))
        return REGIONS::int_keywords.at(name);

    if (SCHEDULE::int_keywords.count(name))
        return SCHEDULE::int_keywords.at(name);

    throw std::out_of_range("No such keyword: " + name);
}

} // end namespace keywords

} // end namespace Fieldprops


namespace {
/*
 * The EQUALREG, MULTREG, COPYREG, ... keywords are used to manipulate
 * vectors based on region values; for instance the statement
 *
 *   EQUALREG
 *      PORO  0.25  3    /   -- Region array not specified
 *      PERMX 100   3  F /
 *   /
 *
 * will set the PORO field to 0.25 for all cells in region 3 and the PERMX
 * value to 100 mD for the same cells. The fourth optional argument to the
 * EQUALREG keyword is used to indicate which REGION array should be used
 * for the selection.
 *
 * If the REGION array is not indicated (as in the PORO case) above, the
 * default region to use in the xxxREG keywords depends on the GRIDOPTS
 * keyword:
 *
 *   1. If GRIDOPTS is present, and the NRMULT item is greater than zero,
 *      the xxxREG keywords will default to use the MULTNUM region.
 *
 *   2. If the GRIDOPTS keyword is not present - or the NRMULT item equals
 *      zero, the xxxREG keywords will default to use the FLUXNUM keyword.
 *
 * This quite weird behaviour comes from reading the GRIDOPTS and MULTNUM
 * documentation, and practical experience with ECLIPSE
 * simulations. Ufortunately the documentation of the xxxREG keywords does
 * not confirm this.
 */
std::string default_region_keyword(const Deck& deck) {
    if (deck.hasKeyword("GRIDOPTS")) {
        const auto& gridOpts = deck["GRIDOPTS"].back();
        const auto& record = gridOpts.getRecord(0);
        const auto& nrmult_item = record.getItem("NRMULT");

        if (nrmult_item.get<int>(0) > 0)
            return "MULTNUM"; // GRIDOPTS and positive NRMULT
    }
    return "FLUXNUM";
}


template <typename T>
void verify_deck_data(const DeckKeyword& keyword, const std::vector<T>& deck_data, const Box& box) {
    if (box.size() != deck_data.size()) {
        const auto& location = keyword.location();
        std::string msg = "Fundamental error with keyword: " + keyword.name() +
            " at: " + location.filename + ", line: " + std::to_string(location.lineno) +
            " got " + std::to_string(deck_data.size()) + " elements - expected : " + std::to_string(box.size());
        throw std::invalid_argument(msg);
    }
}


template <typename T>
void assign_deck(const Fieldprops::keywords::keyword_info<T>& kw_info, const DeckKeyword& keyword, Fieldprops::FieldData<T>& field_data, const std::vector<T>& deck_data, const std::vector<value::status>& deck_status, const Box& box) {
    verify_deck_data(keyword, deck_data, box);
    for (const auto& cell_index : box.index_list()) {
        auto active_index = cell_index.active_index;
        auto data_index = cell_index.data_index;

        if (value::has_value(deck_status[data_index])) {
            if (deck_status[data_index] == value::status::deck_value || field_data.value_status[active_index] == value::status::uninitialized) {
                field_data.data[active_index] = deck_data[data_index];
                field_data.value_status[active_index] = deck_status[data_index];
            }
        }
    }

    if (kw_info.global) {
        auto& global_data = field_data.global_data.value();
        auto& global_status = field_data.global_value_status.value();
        const auto& index_list = box.global_index_list();

        for (const auto& cell : index_list) {
            if (deck_status[cell.data_index] == value::status::deck_value || global_status[cell.global_index] == value::status::uninitialized) {
                global_data[cell.global_index] = deck_data[cell.data_index];
                global_status[cell.global_index] = deck_status[cell.data_index];
            }
        }
    }
}


template <typename T>
void multiply_deck(const Fieldprops::keywords::keyword_info<T>& kw_info, const DeckKeyword& keyword, Fieldprops::FieldData<T>& field_data, const std::vector<T>& deck_data, const std::vector<value::status>& deck_status, const Box& box) {
    verify_deck_data(keyword, deck_data, box);
    for (const auto& cell_index : box.index_list()) {
        auto active_index = cell_index.active_index;
        auto data_index = cell_index.data_index;

        if (value::has_value(deck_status[data_index]) && value::has_value(field_data.value_status[active_index])) {
            field_data.data[active_index] *= deck_data[data_index];
            field_data.value_status[active_index] = deck_status[data_index];
        }
    }

    if (kw_info.global) {
        auto& global_data = field_data.global_data.value();
        auto& global_status = field_data.global_value_status.value();
        const auto& index_list = box.global_index_list();

        for (const auto& cell : index_list) {
            if (deck_status[cell.data_index] == value::status::deck_value || global_status[cell.global_index] == value::status::uninitialized) {
                global_data[cell.global_index] *= deck_data[cell.data_index];
                global_status[cell.global_index] = deck_status[cell.data_index];
            }
        }
    }
}


template <typename T>
void assign_scalar(std::vector<T>& data, std::vector<value::status>& value_status, T value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        data[cell_index.active_index] = value;
        value_status[cell_index.active_index] = value::status::deck_value;
    }
}

template <typename T>
void multiply_scalar(std::vector<T>& data, std::vector<value::status>& value_status, T value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(value_status[cell_index.active_index]))
            data[cell_index.active_index] *= value;
    }
}

template <typename T>
void add_scalar(std::vector<T>& data, std::vector<value::status>& value_status, T value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(value_status[cell_index.active_index]))
            data[cell_index.active_index] += value;
    }
}

template <typename T>
void min_value(std::vector<T>& data, std::vector<value::status>& value_status, T min_value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(value_status[cell_index.active_index])) {
            T value = data[cell_index.active_index];
            data[cell_index.active_index] = std::max(value, min_value);
        }
    }
}

template <typename T>
void max_value(std::vector<T>& data, std::vector<value::status>& value_status, T max_value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(value_status[cell_index.active_index])) {
            T value = data[cell_index.active_index];
            data[cell_index.active_index] = std::min(value, max_value);
        }
    }
}

std::string make_region_name(const std::string& deck_value) {
    if (deck_value == "O")
        return "OPERNUM";

    if (deck_value == "F")
        return "FLUXNUM";

    if (deck_value == "M")
        return "MULTNUM";

    throw std::invalid_argument("The input string: " + deck_value + " was invalid. Expected: O/F/M");
}

Fieldprops::ScalarOperation fromString(const std::string& keyword) {
    if (keyword == ParserKeywords::ADD::keywordName || keyword == ParserKeywords::ADDREG::keywordName)
        return Fieldprops::ScalarOperation::ADD;

    if (keyword == ParserKeywords::EQUALS::keywordName || keyword == ParserKeywords::EQUALREG::keywordName)
        return Fieldprops::ScalarOperation::EQUAL;

    if (keyword == ParserKeywords::MULTIPLY::keywordName || keyword == ParserKeywords::MULTIREG::keywordName)
        return Fieldprops::ScalarOperation::MUL;

    if (keyword == ParserKeywords::MINVALUE::keywordName)
        return Fieldprops::ScalarOperation::MIN;

    if (keyword == ParserKeywords::MAXVALUE::keywordName)
        return Fieldprops::ScalarOperation::MAX;

    throw std::invalid_argument(fmt::format("Keyword operation ({}) not recognized", keyword));
}


void handle_box_keyword(const DeckKeyword& deckKeyword,  Box& box) {
    if (deckKeyword.name() == ParserKeywords::BOX::keywordName) {
        const auto& record = deckKeyword.getRecord(0);
        box.update(record);
    } else
        box.reset();
}


std::vector<double> extract_cell_volume(const EclipseGrid& grid) {
    return grid.activeVolume();
}

std::vector<double> extract_cell_depth(const EclipseGrid& grid) {
    std::vector<double> cell_depth(grid.getNumActive());
    for (std::size_t active_index = 0; active_index < grid.getNumActive(); active_index++)
        cell_depth[active_index] = grid.getCellDepth( grid.getGlobalIndex(active_index));
    return cell_depth;
}



/*
  The rst_compare_data function compares the main std::map<std::string,
  std::vector<T>> data containers. If one of the containers contains a keyword
  *which is fully defaulted* and the other container does not contain said
  keyword - the containers are considered to be equal.
*/
template <typename T>
bool rst_compare_data(const std::unordered_map<std::string, Fieldprops::FieldData<T>>& data1,
                  const std::unordered_map<std::string, Fieldprops::FieldData<T>>& data2) {
    std::unordered_set<std::string> keys;
    for (const auto& [key, _] : data1) {
        (void)_;
        keys.insert(key);
    }

    for (const auto& [key, _] : data2) {
        (void)_;
        keys.insert(key);
    }

    for (const auto& key : keys) {
        const auto& d1 = data1.find(key);
        const auto& d2 = data2.find(key);

        if (d1 == data1.end()) {
            if (!d2->second.valid_default())
                return false;
            continue;
        }

        if (d2 == data2.end()) {
            if (!d1->second.valid_default())
                return false;
            continue;
        }

        if (!(d1->second == d2->second))
            return false;
    }

    return true;
}


}





bool FieldProps::operator==(const FieldProps& other) const {
    return this->unit_system == other.unit_system &&
           this->nx == other.nx &&
           this->ny == other.ny &&
           this->nz == other.nz &&
           this->m_phases == other.m_phases &&
           this->m_satfuncctrl == other.m_satfuncctrl &&
           this->m_actnum == other.m_actnum &&
           this->cell_volume == other.cell_volume &&
           this->cell_depth == other.cell_depth &&
           this->m_default_region == other.m_default_region &&
           this->m_rtep == other.m_rtep &&
           this->tables == other.tables &&
           this->int_data == other.int_data &&
           this->double_data == other.double_data &&
           this->multregp == other.multregp &&
           this->tran == other.tran;
}

bool FieldProps::rst_cmp(const FieldProps& full_arg, const FieldProps& rst_arg) {

    if (!rst_compare_data(full_arg.double_data, rst_arg.double_data))
        return false;

    if (!rst_compare_data(full_arg.int_data, rst_arg.int_data))
        return false;

    if (!UnitSystem::rst_cmp(full_arg.unit_system, rst_arg.unit_system))
        return false;

    return full_arg.nx == rst_arg.nx &&
        full_arg.ny == rst_arg.ny &&
        full_arg.nz == rst_arg.nz &&
        full_arg.m_phases == rst_arg.m_phases &&
        full_arg.m_satfuncctrl == rst_arg.m_satfuncctrl &&
        full_arg.m_actnum == rst_arg.m_actnum &&
        full_arg.cell_volume == rst_arg.cell_volume &&
        full_arg.cell_depth == rst_arg.cell_depth &&
        full_arg.m_default_region == rst_arg.m_default_region &&
        full_arg.m_rtep == rst_arg.m_rtep &&
        full_arg.tables == rst_arg.tables &&
        full_arg.multregp == rst_arg.multregp &&
        full_arg.tran == rst_arg.tran;
}


FieldProps::FieldProps(const Deck& deck, const Phases& phases, const EclipseGrid& grid, const TableManager& tables_arg) :
    active_size(grid.getNumActive()),
    global_size(grid.getCartesianSize()),
    unit_system(deck.getActiveUnitSystem()),
    nx(grid.getNX()),
    ny(grid.getNY()),
    nz(grid.getNZ()),
    m_phases(phases),
    m_satfuncctrl(deck),
    m_actnum(grid.getACTNUM()),
    cell_volume(extract_cell_volume(grid)),
    cell_depth(extract_cell_depth(grid)),
    m_default_region(default_region_keyword(deck)),
    grid_ptr(&grid),
    tables(tables_arg)
{
    this->tran.emplace( "TRANX", Fieldprops::TranCalculator("TRANX") );
    this->tran.emplace( "TRANY", Fieldprops::TranCalculator("TRANY") );
    this->tran.emplace( "TRANZ", Fieldprops::TranCalculator("TRANZ") );

    if (deck.hasKeyword<ParserKeywords::MULTREGP>()) {
        const DeckKeyword& multregpKeyword = deck["MULTREGP"].back();
        for (const auto& record : multregpKeyword) {
            int region_value = record.getItem("REGION").get<int>(0);
            if (region_value <= 0)
                continue;

            std::string region_name = make_region_name( record.getItem("REGION_TYPE").get<std::string>(0) );
            double multiplier = record.getItem("MULTIPLIER").get<double>(0);
            auto iter = std::find_if(this->multregp.begin(), this->multregp.end(), [region_value](const MultregpRecord& mregp) { return mregp.region_value == region_value; });
            /*
              There is some weirdness if the same region value is entered in several records,
              then only the last applies.
            */
            if (iter != this->multregp.end()) {
                iter->region_name = region_name;
                iter->multiplier = multiplier;
            } else
                this->multregp.emplace_back( region_value, multiplier, region_name );
        }
    }


    if (DeckSection::hasGRID(deck))
        this->scanGRIDSection(GRIDSection(deck));

    if (DeckSection::hasEDIT(deck))
        this->scanEDITSection(EDITSection(deck));

    if (DeckSection::hasREGIONS(deck))
        this->scanREGIONSSection(REGIONSSection(deck));

    if (DeckSection::hasPROPS(deck))
        this->scanPROPSSection(PROPSSection(deck));

    if (DeckSection::hasSOLUTION(deck))
        this->scanSOLUTIONSection(SOLUTIONSection(deck));
}


// Special constructor ONLY used to get the correct ACTNUM.
// The grid argument should have all active cells.
FieldProps::FieldProps(const Deck& deck, const EclipseGrid& grid) :
    active_size(grid.getNumActive()),
    global_size(grid.getCartesianSize()),
    unit_system(deck.getActiveUnitSystem()),
    nx(grid.getNX()),
    ny(grid.getNY()),
    nz(grid.getNZ()),
    m_phases(),
    m_satfuncctrl(deck),
    m_actnum(global_size, 1),  // NB! activates all at start!
    cell_volume(),             // NB! empty for this purpose.
    cell_depth(),              // NB! empty for this purpose.
    m_default_region(default_region_keyword(deck)),
    grid_ptr(&grid),
    tables()                   // NB! empty for this purpose.
{
    if (this->active_size != this->global_size) {
        throw std::logic_error("Programmer error: FieldProps special case processing for ACTNUM called with grid object that already had deactivated cells.");
    }
    if (DeckSection::hasGRID(deck))
        this->scanGRIDSectionOnlyACTNUM(GRIDSection(deck));
}



void FieldProps::reset_actnum(const std::vector<int>& new_actnum) {
    if (this->global_size != new_actnum.size())
        throw std::logic_error("reset_actnum() must be called with the same number of global cells");

    if (new_actnum == this->m_actnum)
        return;

    std::vector<bool> active_map(this->active_size, true);
    std::size_t active_index = 0;
    std::size_t new_active_size = 0;
    for (std::size_t g = 0; g < this->m_actnum.size(); g++) {
        if (this->m_actnum[g] != 0) {
            if (new_actnum[g] == 0)
                active_map[active_index] = false;
            else
                new_active_size += 1;

            active_index += 1;
        } else {
            if (new_actnum[g] != 0)
                throw std::logic_error("It is not possible to activate cells");
        }
    }

    for (auto& data : this->double_data)
        data.second.compress(active_map);

    for (auto& data : this->int_data)
        data.second.compress(active_map);

    Fieldprops::compress(this->cell_volume, active_map);
    Fieldprops::compress(this->cell_depth, active_map);

    this->m_actnum = std::move(new_actnum);
    this->active_size = new_active_size;
}


void FieldProps::distribute_toplayer(Fieldprops::FieldData<double>& field_data, const std::vector<double>& deck_data, const Box& box) {
    const std::size_t layer_size = this->nx * this->ny;
    Fieldprops::FieldData<double> toplayer(field_data.kw_info, layer_size, 0);
    for (const auto& cell_index : box.index_list()) {
        if (cell_index.global_index < layer_size) {
            toplayer.data[cell_index.global_index] = deck_data[cell_index.data_index];
            toplayer.value_status[cell_index.global_index] = value::status::deck_value;
        }
    }

    std::size_t active_index = 0;
    for (std::size_t k = 0; k < this->nz; k++) {
        for (std::size_t j = 0; j < this->ny; j++) {
            for (std::size_t i = 0; i < this->nx; i++) {
                std::size_t g = i + j*this->nx + k*this->nx*this->ny;
                if (this->m_actnum[g]) {
                    if (field_data.value_status[active_index] == value::status::uninitialized) {
                        std::size_t layer_index = i + j*this->nx;
                        if (toplayer.value_status[layer_index] == value::status::deck_value) {
                            field_data.data[active_index] = toplayer.data[layer_index];
                            field_data.value_status[active_index] = value::status::valid_default;
                        }
                    }
                    active_index += 1;
                }
            }
        }
    }
}


template <>
bool FieldProps::supported<double>(const std::string& keyword) {
    if (Fieldprops::keywords::GRID::double_keywords.count(keyword) != 0)
        return true;

    if (Fieldprops::keywords::EDIT::double_keywords.count(keyword) != 0)
        return true;

    if (Fieldprops::keywords::PROPS::double_keywords.count(keyword) != 0)
        return true;

    if (Fieldprops::keywords::PROPS::satfunc.count(keyword) != 0)
        return true;

    if (Fieldprops::keywords::SOLUTION::double_keywords.count(keyword) != 0)
        return true;

    return false;
}

template <>
bool FieldProps::supported<int>(const std::string& keyword) {
    if (Fieldprops::keywords::REGIONS::int_keywords.count(keyword) != 0)
        return true;

    if (Fieldprops::keywords::GRID::int_keywords.count(keyword) != 0)
        return true;

    if (Fieldprops::keywords::SCHEDULE::int_keywords.count(keyword) != 0)
        return true;

    return Fieldprops::keywords::isFipxxx(keyword);
}


template <>
Fieldprops::FieldData<double>& FieldProps::init_get(const std::string& keyword_name, const Fieldprops::keywords::keyword_info<double>& kw_info) {
    const std::string& keyword = Fieldprops::keywords::get_keyword_from_alias(keyword_name);

    auto iter = this->double_data.find(keyword);
    if (iter != this->double_data.end())
        return iter->second;

    this->double_data[keyword] = Fieldprops::FieldData<double>(kw_info, this->active_size, kw_info.global ? this->global_size : 0);

    if (keyword == ParserKeywords::PORV::keywordName)
        this->init_porv(this->double_data[keyword]);

    if (keyword == ParserKeywords::TEMPI::keywordName)
        this->init_tempi(this->double_data[keyword]);

    if (Fieldprops::keywords::PROPS::satfunc.count(keyword) == 1)
        this->init_satfunc(keyword, this->double_data[keyword]);

    return this->double_data[keyword];
}

template <>
Fieldprops::FieldData<double>& FieldProps::init_get(const std::string& keyword,
                                        bool allow_unsupported) {
    Fieldprops::keywords::keyword_info<double> kw_info = Fieldprops::keywords::global_kw_info<double>(keyword, allow_unsupported);
    return this->init_get(keyword, kw_info);
}


template <>
Fieldprops::FieldData<int>& FieldProps::init_get(const std::string& keyword, const Fieldprops::keywords::keyword_info<int>& kw_info) {
    auto iter = this->int_data.find(keyword);
    if (iter != this->int_data.end())
        return iter->second;

    this->int_data[keyword] = Fieldprops::FieldData<int>(kw_info, this->active_size, kw_info.global ? this->global_size : 0);
    return this->int_data[keyword];
}

template <>
Fieldprops::FieldData<int>& FieldProps::init_get(const std::string& keyword, bool) {
    if (Fieldprops::keywords::isFipxxx(keyword)) {
        auto kw_info = Fieldprops::keywords::keyword_info<int>{};
        kw_info.init(1);
        return this->init_get(keyword, kw_info);
    } else {
        const Fieldprops::keywords::keyword_info<int>& kw_info = Fieldprops::keywords::global_kw_info<int>(keyword);
        return this->init_get(keyword, kw_info);
    }
}


std::vector<Box::cell_index> FieldProps::region_index( const std::string& region_name, int region_value ) {
    const auto& region = this->init_get<int>(region_name);
    if (!region.valid())
        throw std::invalid_argument("Trying to work with invalid region: " + region_name);

    std::vector<Box::cell_index> index_list;
    std::size_t active_index = 0;
    const auto& region_data = region.data;
    for (std::size_t g = 0; g < this->m_actnum.size(); g++) {
        if (this->m_actnum[g] != 0) {
            if (region_data[active_index] == region_value)
                index_list.emplace_back( g, active_index, g );
            active_index += 1;
        }
    }
    return index_list;
}



std::string FieldProps::region_name(const DeckItem& region_item) {
    return region_item.defaultApplied(0) ? this->m_default_region : make_region_name(region_item.get<std::string>(0));
}

template <>
bool FieldProps::has<double>(const std::string& keyword_name) const {
    const std::string& keyword = Fieldprops::keywords::get_keyword_from_alias(keyword_name);
    return (this->double_data.count(keyword) != 0);
}

template <>
bool FieldProps::has<int>(const std::string& keyword) const {
    return (this->int_data.count(keyword) != 0);
}


/*
  The ACTNUM and PORV keywords are special cased with quite extensive
  postprocessing, and should be access through the special ::porv() and
  ::actnum() methods instead of the general ::get<T>( ) method. These two
  keywords are also hidden from the keys<T>() vectors.

  If there are TRAN? fields in the container they are transferred even if they
  are not completely defined. This is because that TRAN? fields will ultimately
  be combined with the TRAN? values calculated from the simulator, it does
  therefor not make sense to require that these fields are fully defined.
*/

template <>
std::vector<std::string> FieldProps::keys<double>() const {
    std::vector<std::string> klist;
    for (const auto& [key, field] : this->double_data) {
        if (key.rfind("TRAN", 0) == 0) {
            klist.push_back(key);
            continue;
        }

        if (field.valid() && key != "PORV")
            klist.push_back(key);
    }
    return klist;
}


template <>
std::vector<std::string> FieldProps::keys<int>() const {
    std::vector<std::string> klist;
    for (const auto& data_pair : this->int_data) {
        if (data_pair.second.valid() && data_pair.first != "ACTNUM")
            klist.push_back(data_pair.first);
    }
    return klist;
}


template <>
void FieldProps::erase<int>(const std::string& keyword) {
    this->int_data.erase(keyword);
}

template <>
void FieldProps::erase<double>(const std::string& keyword) {
    this->double_data.erase(keyword);
}

template <>
std::vector<int> FieldProps::extract<int>(const std::string& keyword) {
    auto field_iter = this->int_data.find(keyword);
    auto field = std::move(field_iter->second);
    std::vector<int> data = std::move( field.data );
    this->int_data.erase( field_iter );
    return data;
}

template <>
std::vector<double> FieldProps::extract<double>(const std::string& keyword) {
    auto field_iter = this->double_data.find(keyword);
    auto field = std::move(field_iter->second);
    std::vector<double> data = std::move( field.data );
    this->double_data.erase( field_iter );
    return data;
}





double FieldProps::getSIValue(const std::string& keyword, double raw_value) const {
    if (this->tran.count(keyword))
        return this->unit_system.to_si(UnitSystem::measure::transmissibility, raw_value);
    else {
        const auto& kw_info = Fieldprops::keywords::global_kw_info<double>(keyword);
        if (kw_info.unit) {
            const auto& dim = this->unit_system.parse( *kw_info.unit );
            return dim.convertRawToSi(raw_value);
        }
        return raw_value;
    }
}



double FieldProps::getSIValue(ScalarOperation op, const std::string& keyword, double raw_value) const {
    if (op == ScalarOperation::MUL)
        return raw_value;

    return this->getSIValue(keyword, raw_value);
}


void FieldProps::handle_int_keyword(const Fieldprops::keywords::keyword_info<int>& kw_info, const DeckKeyword& keyword, const Box& box) {
    auto& field_data = this->init_get<int>(keyword.name());
    const auto& deck_data = keyword.getIntData();
    const auto& deck_status = keyword.getValueStatus();
    assign_deck(kw_info, keyword, field_data, deck_data, deck_status, box);
}


void FieldProps::handle_double_keyword(Section section, const Fieldprops::keywords::keyword_info<double>& kw_info, const DeckKeyword& keyword, const std::string& keyword_name, const Box& box) {
    auto& field_data = this->init_get<double>(keyword_name, kw_info);
    const auto& deck_data = keyword.getSIDoubleData();
    const auto& deck_status = keyword.getValueStatus();

    if ((section == Section::EDIT || section == Section::SCHEDULE) && kw_info.multiplier)
        multiply_deck(kw_info, keyword, field_data, deck_data, deck_status, box);
    else
        assign_deck(kw_info, keyword, field_data, deck_data, deck_status, box);


    if (section == Section::GRID) {
        if (field_data.valid())
            return;

        if (kw_info.top)
            this->distribute_toplayer(field_data, deck_data, box);
    }
}

void FieldProps::handle_double_keyword(Section section, const Fieldprops::keywords::keyword_info<double>& kw_info, const DeckKeyword& keyword, const Box& box) {
    this->handle_double_keyword(section, kw_info, keyword, keyword.name(), box );
}



template <typename T>
void FieldProps::apply(Fieldprops::ScalarOperation op, std::vector<T>& data, std::vector<value::status>& value_status, T scalar_value, const std::vector<Box::cell_index>& index_list) {
    if (op == Fieldprops::ScalarOperation::EQUAL)
        assign_scalar(data, value_status, scalar_value, index_list);

    else if (op == Fieldprops::ScalarOperation::MUL)
        multiply_scalar(data, value_status, scalar_value, index_list);

    else if (op == Fieldprops::ScalarOperation::ADD)
        add_scalar(data, value_status, scalar_value, index_list);

    else if (op == Fieldprops::ScalarOperation::MIN)
        min_value(data, value_status, scalar_value, index_list);

    else if (op == Fieldprops::ScalarOperation::MAX)
        max_value(data, value_status, scalar_value, index_list);
}

double FieldProps::get_alpha(const std::string& func_name, const std::string& target_array, double raw_alpha) {
    if ( !(func_name == "ADDX" || func_name == "MAXLIM" || func_name == "MINLIM") )
        return raw_alpha;

    return this->getSIValue(target_array, raw_alpha);
}

double FieldProps::get_beta(const std::string& func_name, const std::string& target_array, double raw_beta) {
    if ( func_name != "MULTA")
        return raw_beta;

    return this->getSIValue(target_array, raw_beta);
}

template <typename T>
void FieldProps::operate(const DeckRecord& record, Fieldprops::FieldData<T>& target_data, const Fieldprops::FieldData<T>& src_data, const std::vector<Box::cell_index>& index_list) {
    const std::string& func_name = record.getItem("OPERATION").get< std::string >(0);
    const std::string& target_array = record.getItem("TARGET_ARRAY").get<std::string>(0);
    const double alpha           = this->get_alpha(func_name, target_array, record.getItem("PARAM1").get< double >(0));
    const double beta            = this->get_beta( func_name, target_array, record.getItem("PARAM2").get< double >(0));
    Operate::function func       = Operate::get( func_name, alpha, beta );
    bool check_target            = (func_name == "MULTIPLY" || func_name == "POLY");

    if (target_data.global_data)
        throw std::logic_error("The OPERATE and OPERATER keywords are not supported for keywords with global storage");

    if (this->tran.find(target_array) != this->tran.end())
        throw std::logic_error("The OPERATE keyword can not be used for manipulations of TRANX, TRANY or TRANZ");

    for (const auto& cell_index : index_list) {
        if (value::has_value(src_data.value_status[cell_index.active_index])) {
            if ((check_target == false) || (value::has_value(target_data.value_status[cell_index.active_index]))) {
                target_data.data[cell_index.active_index]         = func(target_data.data[cell_index.active_index], src_data.data[cell_index.active_index]);
                target_data.value_status[cell_index.active_index] = src_data.value_status[cell_index.active_index];
            } else
                throw std::invalid_argument("Tried to use unset property value in OPERATE/OPERATER keyword");
        } else
            throw std::invalid_argument("Tried to use unset property value in OPERATE/OPERATER keyword");
    }
}

void FieldProps::handle_region_operation(const DeckKeyword& keyword) {
    for (const auto& record : keyword) {
        const std::string& target_kw = Fieldprops::keywords::get_keyword_from_alias(record.getItem(0).get<std::string>(0));
        int region_value = record.getItem("REGION_NUMBER").get<int>(0);

        if (this->tran.find(target_kw) != this->tran.end())
            throw std::logic_error("The region operations can not be used for manipulations of TRANX, TRANY or TRANZ");

        if (FieldProps::supported<double>(target_kw)) {
            if (keyword.name() == ParserKeywords::OPERATER::keywordName) {
                // For the OPERATER keyword we fetch the region name from the deck record
                // with no extra hoops.
                std::string region_name = record.getItem("REGION_NAME").get<std::string>(0);
                const auto& index_list = this->region_index(region_name, region_value);
                const std::string& src_kw = record.getItem("ARRAY_PARAMETER").get<std::string>(0);
                const auto& src_data = this->init_get<double>(src_kw);
                auto& field_data = this->init_get<double>(target_kw);
                FieldProps::operate(record, field_data, src_data, index_list);
            } else {
                auto operation = fromString(keyword.name());
                const double scalar_value = this->getSIValue(operation, target_kw, record.getItem(1).get<double>(0));
                std::string region_name = this->region_name( record.getItem("REGION_NAME") );
                const auto& index_list = this->region_index( region_name, region_value);
                auto& field_data = this->init_get<double>(target_kw);
                /*
                  To support region operations on keywords with global storage we
                  would need to also have global storage for the xxxNUM region
                  keywords involved. To avoid a situation where a significant
                  fraction of the keywords have global storage the implementation
                  has stopped here - there are no principle problems with extending
                  the implementation to also support region operations on fields
                  with global storage.
                */
                if (field_data.global_data)
                {
                    const auto& location = keyword.location();
                    using namespace std::string_literals;
                    throw OpmInputError(fmt::format("region operation on 3D field {} with "s +
                                                    "global storage is not implemented!"s,
                                                    target_kw),
                                        location);
                }

                FieldProps::apply(fromString(keyword.name()), field_data.data, field_data.value_status, scalar_value, index_list);
            }

            continue;
        }

        if (FieldProps::supported<int>(target_kw)) {
            continue;
        }
    }
}


void FieldProps::handle_OPERATE(const DeckKeyword& keyword, Box box) {
    for (const auto& record : keyword) {
        const std::string& target_kw = Fieldprops::keywords::get_keyword_from_alias(record.getItem(0).get<std::string>(0));
        box.update(record);

        auto& field_data = this->init_get<double>(target_kw);
        const std::string& src_kw = record.getItem("ARRAY").get<std::string>(0);
        const auto& src_data = this->init_get<double>(src_kw);
        FieldProps::operate(record, field_data, src_data, box.index_list());
    }
}


void FieldProps::handle_operation(const DeckKeyword& keyword, Box box) {
    std::unordered_map<std::string, std::string> tran_fields;
    for (const auto& record : keyword) {
        const std::string& target_kw = Fieldprops::keywords::get_keyword_from_alias(record.getItem(0).get<std::string>(0));
        box.update(record);

        if (FieldProps::supported<double>(target_kw) || this->tran.count(target_kw) > 0) {
            std::string unique_name = target_kw;
            auto operation = fromString(keyword.name());
            const double scalar_value = this->getSIValue(operation, target_kw, record.getItem(1).get<double>(0));
            Fieldprops::keywords::keyword_info<double> kw_info;

            auto tran_iter = this->tran.find(target_kw);
            // Check if the target keyword is one of the TRANX, TRANY or TRANZ keywords.
            if (tran_iter != this->tran.end()) {
                auto tran_field_iter = tran_fields.find(target_kw);
                /*
                  The transmissibility calculations are applied to one "work" 3D
                  field per direction and per keyword. Here we check if we have
                  encountered this TRAN direction previously for this keyword,
                  if not we generate a new 3D field and register a new tran
                  calculator operation.
                 */
                if (tran_field_iter == tran_fields.end()) {
                    unique_name = tran_iter->second.next_name();
                    tran_fields.emplace(target_kw, unique_name);
                    tran_iter->second.add_action(operation, unique_name);
                    kw_info = tran_iter->second.make_kw_info(operation);
                } else
                    unique_name = tran_field_iter->second;

            } else
                kw_info = Fieldprops::keywords::global_kw_info<double>(target_kw);

            auto& field_data = this->init_get<double>(unique_name, kw_info);

            FieldProps::apply(operation, field_data.data, field_data.value_status, scalar_value, box.index_list());
            if (field_data.global_data)
                FieldProps::apply(operation, *field_data.global_data, *field_data.global_value_status, scalar_value, box.global_index_list());

            continue;
        }


        if (FieldProps::supported<int>(target_kw)) {
            int scalar_value = static_cast<int>(record.getItem(1).get<double>(0));
            auto& field_data = this->init_get<int>(target_kw);
            FieldProps::apply(fromString(keyword.name()), field_data.data, field_data.value_status, scalar_value, box.index_list());
            continue;
        }

        throw OpmInputError("Operation keyword " + keyword.name() + " does not support the keyword " + target_kw, keyword.location());
    }
}


void FieldProps::handle_COPY(const DeckKeyword& keyword, Box box, bool region) {
    for (const auto& record : keyword) {
        const std::string& src_kw = Fieldprops::keywords::get_keyword_from_alias(record.getItem(0).get<std::string>(0));
        const std::string& target_kw = Fieldprops::keywords::get_keyword_from_alias(record.getItem(1).get<std::string>(0));
        std::vector<Box::cell_index> index_list;

        if (region) {
            int region_value = record.getItem(2).get<int>(0);
            const auto& region_item = record.getItem(3);
            const auto& region_name = this->region_name( region_item );
            index_list = this->region_index(region_name, region_value);
        } else {
            box.update(record);
            index_list = box.index_list();
        }


        if (FieldProps::supported<double>(src_kw)) {
            const auto& src_data = this->try_get<double>(src_kw);
            src_data.verify_status();

            auto& target_data = this->init_get<double>(target_kw);
            target_data.copy(src_data.field_data(), index_list);
            continue;
        }

        if (FieldProps::supported<int>(src_kw)) {
            const auto& src_data = this->try_get<int>(src_kw);
            src_data.verify_status();

            auto& target_data = this->init_get<int>(target_kw);
            target_data.copy(src_data.field_data(), index_list);
            continue;
        }
    }
}

void FieldProps::handle_keyword(const DeckKeyword& keyword, Box& box) {
    const std::string& name = keyword.name();

    if (Fieldprops::keywords::oper_keywords.count(name) == 1)
        this->handle_operation(keyword, box);

    else if (name == ParserKeywords::OPERATE::keywordName)
        this->handle_OPERATE(keyword, box);

    else if (Fieldprops::keywords::region_oper_keywords.count(name) == 1)
        this->handle_region_operation(keyword);

    else if (Fieldprops::keywords::box_keywords.count(name) == 1)
        handle_box_keyword(keyword, box);

    else if (name == ParserKeywords::COPY::keywordName)
        handle_COPY(keyword, box, false);

    else if (name == ParserKeywords::COPYREG::keywordName)
        handle_COPY(keyword, box, true);
}

/**********************************************************************/


void FieldProps::init_tempi(Fieldprops::FieldData<double>& tempi) {
    if (this->tables.hasTables("RTEMPVD")) {
        const auto& eqlnum = this->get<int>("EQLNUM");
        const auto& rtempvd = this->tables.getRtempvdTables();
        std::vector< double > tempi_values( this->active_size, 0 );

        for (size_t active_index = 0; active_index < this->active_size; active_index++) {
            const auto& table = rtempvd.getTable<RtempvdTable>(eqlnum[active_index] - 1);
            double depth = this->cell_depth[active_index];
            tempi_values[active_index] = table.evaluate("Temperature", depth);
        }

        tempi.default_update(tempi_values);
    } else
        tempi.default_assign(this->tables.rtemp());
}

void FieldProps::init_porv(Fieldprops::FieldData<double>& porv) {
    auto& porv_data = porv.data;
    auto& porv_status = porv.value_status;

    const auto& poro = this->init_get<double>("PORO");
    const auto& poro_status = poro.value_status;
    const auto& poro_data = poro.data;

    for (std::size_t active_index = 0; active_index < this->active_size; active_index++) {
        if (value::has_value(poro_status[active_index])) {
            porv_data[active_index] = this->cell_volume[active_index] * poro_data[active_index];
            porv_status[active_index] = value::status::valid_default;
        }
    }

    if (this->has<double>("NTG")) {
        const auto& ntg = this->get<double>("NTG");
        for (std::size_t active_index = 0; active_index < this->active_size; active_index++)
            porv_data[active_index] *= ntg[active_index];
    }

    if (this->has<double>("MULTPV")) {
        const auto& multpv = this->get<double>("MULTPV");
        std::transform(porv_data.begin(), porv_data.end(), multpv.begin(), porv_data.begin(), std::multiplies<double>());
    }

    for (const auto& mregp: this->multregp) {
        const auto& index_list = this->region_index(mregp.region_name, mregp.region_value);
        for (const auto& cell_index : index_list)
            porv_data[cell_index.active_index] *= mregp.multiplier;
    }
}


/*
  This function generates a new ACTNUM vector.The ACTNUM vector which is
  returned is joined result of three different data sources:

     1. The ACTNUM of if the grid which is part of this FieldProps structure.

     2. If there have been ACTNUM operations in the DECK of the type:

        EQUALS
            ACTNUM 0 1 10 1 10 1 3 /
        /

     3. Cells with PORV == 0 will get ACTNUM = 0.

  Observe that due to steps 2 and 3 the ACTNUM vector returned from this
  function will in general differ from the internal ACTNUM used in the
  FieldProps instance.
*/
std::vector<int> FieldProps::actnum() {
    auto actnum = this->m_actnum;
    const auto& deck_actnum = this->init_get<int>("ACTNUM");

    std::vector<int> global_map(this->active_size);
    {
        std::size_t active_index = 0;
        for (std::size_t g = 0; g < this->global_size; g++) {
            if (this->m_actnum[g]) {
                global_map[active_index] = g;
                active_index++;
            }
        }
    }


    const auto& porv = this->init_get<double>("PORV");
    const auto& porv_data = porv.data;
    for (std::size_t active_index = 0; active_index < this->active_size; active_index++) {
        auto global_index = global_map[active_index];
        actnum[global_index] = deck_actnum.data[active_index];
        if (porv_data[active_index] == 0)
            actnum[global_index] = 0;
    }
    return actnum;
}


const std::vector<int>& FieldProps::actnumRaw() const {
    return m_actnum;
}


void FieldProps::scanGRIDSection(const GRIDSection& grid_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : grid_section) {
        const std::string& name = keyword.name();

        if (Fieldprops::keywords::GRID::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::GRID, Fieldprops::keywords::GRID::double_keywords.at(name), keyword, box);
            continue;
        }

        if (Fieldprops::keywords::GRID::int_keywords.count(name) == 1) {
            this->handle_int_keyword(Fieldprops::keywords::GRID::int_keywords.at(name), keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}

void FieldProps::scanGRIDSectionOnlyACTNUM(const GRIDSection& grid_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : grid_section) {
        const std::string& name = keyword.name();
        if (name == "ACTNUM") {
            this->handle_int_keyword(Fieldprops::keywords::GRID::int_keywords.at(name), keyword, box);
        } else if (name == "EQUALS" || (Fieldprops::keywords::box_keywords.count(name) == 1)) {
            this->handle_keyword(keyword, box);
        }
    }
    const auto iter = this->int_data.find("ACTNUM");
    if (iter == this->int_data.end()) {
        m_actnum.assign(this->grid_ptr->getCartesianSize(), 1);
    } else {
        m_actnum = iter->second.data;
    }
}

void FieldProps::scanEDITSection(const EDITSection& edit_section) {
    Box box(*this->grid_ptr);
    for (const auto& keyword : edit_section) {
        const std::string& name = keyword.name();

        auto tran_iter = this->tran.find(name);
        if (tran_iter!= this->tran.end()) {
            auto& tran_calc = tran_iter->second;
            auto unique_name = tran_calc.next_name();
            this->handle_double_keyword(Section::EDIT, {}, keyword, unique_name, box);
            tran_calc.add_action( Fieldprops::ScalarOperation::EQUAL, unique_name );
            continue;
        }

        if (Fieldprops::keywords::EDIT::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::EDIT, Fieldprops::keywords::EDIT::double_keywords.at(name), keyword, box);
            continue;
        }

        if (Fieldprops::keywords::EDIT::int_keywords.count(name) == 1) {
            this->handle_int_keyword(Fieldprops::keywords::GRID::int_keywords.at(name), keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}


void FieldProps::init_satfunc(const std::string& keyword, Fieldprops::FieldData<double>& satfunc) {
    if (!this->m_rtep.has_value())
        this->m_rtep = satfunc::getRawTableEndpoints(this->tables, this->m_phases,
                                                     this->m_satfuncctrl.minimumRelpermMobilityThreshold());

    const auto& endnum = this->get<int>("ENDNUM");
    const auto& satreg = (keyword[0] == 'I')
        ? this->get<int>("IMBNUM")
        : this->get<int>("SATNUM");

    satfunc.default_update(satfunc::init(keyword, this->tables, this->m_phases, this->m_rtep.value(), this->cell_depth, satreg, endnum));
}


void FieldProps::scanPROPSSection(const PROPSSection& props_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : props_section) {
        const std::string& name = keyword.name();
        if (Fieldprops::keywords::PROPS::satfunc.count(name) == 1) {
            Fieldprops::keywords::keyword_info<double> sat_info{};
            this->handle_double_keyword(Section::PROPS, sat_info, keyword, box);
            continue;
        }

        if (Fieldprops::keywords::PROPS::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::PROPS, Fieldprops::keywords::PROPS::double_keywords.at(name), keyword, box);
            continue;
        }

        if (Fieldprops::keywords::PROPS::int_keywords.count(name) == 1) {
            this->handle_int_keyword(Fieldprops::keywords::PROPS::int_keywords.at(name), keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}


void FieldProps::scanREGIONSSection(const REGIONSSection& regions_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : regions_section) {
        const std::string& name = keyword.name();
        if (Fieldprops::keywords::REGIONS::int_keywords.count(name)) {
            this->handle_int_keyword(Fieldprops::keywords::REGIONS::int_keywords.at(name), keyword, box);
            continue;
        }

        if (Fieldprops::keywords::isFipxxx(name)) {
            auto kw_info = Fieldprops::keywords::keyword_info<int>{};
            kw_info.init(1);
            this->handle_int_keyword(kw_info, keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}


void FieldProps::scanSOLUTIONSection(const SOLUTIONSection& solution_section) {
    Box box(*this->grid_ptr);
    for (const auto& keyword : solution_section) {
        const std::string& name = keyword.name();
        if (Fieldprops::keywords::SOLUTION::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::SOLUTION, Fieldprops::keywords::SOLUTION::double_keywords.at(name), keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}

void FieldProps::handle_schedule_keywords(const std::vector<DeckKeyword>& keywords) {
    Box box(*this->grid_ptr);

    // When called in the SCHEDULE section the context is that the scaling factors
    // have already been applied.
    for (const auto& [kw, _] : Fieldprops::keywords::SCHEDULE::double_keywords) {
        (void)_;
        if (this->has<double>(kw)) {
            auto& field_data = this->init_get<double>(kw);
            field_data.default_assign(1.0);
        }
    }


    for (const auto& keyword : keywords) {
        const std::string& name = keyword.name();
        if (Fieldprops::keywords::SCHEDULE::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::SCHEDULE, Fieldprops::keywords::SCHEDULE::double_keywords.at(name), keyword, box);
            continue;
        }

        if (Fieldprops::keywords::box_keywords.count(name) == 1) {
            handle_box_keyword(keyword, box);
            continue;
        }
    }
}

const std::string& FieldProps::default_region() const {
    return this->m_default_region;
}

void FieldProps::apply_tran(const std::string& keyword, std::vector<double>& data) {
    Opm::apply_tran(this->tran, this->double_data, this->active_size, keyword, data);
}


std::vector<char> FieldProps::serialize_tran() const {
    Serializer ser;
    ser.put(this->tran.size());
    for (const auto& tran_pair : this->tran) {
        const auto& calc = tran_pair.second;
        ser.put(calc.name());
        ser.put(calc.size());
        for (const auto& action : calc) {
            ser.put(static_cast<int>(action.op));
            ser.put(action.field);
        }
    }
    return std::move(ser.buffer);
}

void FieldProps::deserialize_tran(const std::vector<char>& buffer) {
    Opm::deserialize_tran(this->tran, buffer);
}

bool FieldProps::tran_active(const std::string& keyword) const {
    auto calculator = this->tran.find(keyword);
    return calculator != this->tran.end() && calculator->second.size() > 0;
}

void FieldProps::apply_numerical_aquifers(const NumericalAquifers& numerical_aquifers) {
    auto& porv_data = this->init_get<double>("PORV").data;
    auto& poro_data = this->init_get<double>("PORO").data;
    auto& satnum_data = this->init_get<int>("SATNUM").data;
    auto& pvtnum_data = this->init_get<int>("PVTNUM").data;

    auto& permx_data = this->init_get<double>("PERMX").data;
    auto& permy_data = this->init_get<double>("PERMY").data;
    auto& permz_data = this->init_get<double>("PERMZ").data;

    const auto& aqu_cell_props = numerical_aquifers.aquiferCellProps();
    for (const auto& [global_index, cellprop] : aqu_cell_props) {
        const size_t active_index = this->grid_ptr->activeIndex(global_index);
        this->cell_volume[active_index] = cellprop.volume;
        this->cell_depth[active_index] = cellprop.depth;

        porv_data[active_index] = cellprop.pore_volume;
        poro_data[active_index] = cellprop.porosity;
        satnum_data[active_index] = cellprop.satnum;
        pvtnum_data[active_index] = cellprop.pvtnum;

        // isolate the numerical aquifer cells by setting permeability to be zero
        permx_data[active_index] = 0.;
        permy_data[active_index] = 0.;
        permz_data[active_index] = 0.;
    }
}


template std::vector<bool> FieldProps::defaulted<int>(const std::string& keyword);
template std::vector<bool> FieldProps::defaulted<double>(const std::string& keyword);
}

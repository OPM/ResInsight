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
#include <functional>
#include <algorithm>

#include <opm/parser/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/B.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/O.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RtempvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>

#include "FieldProps.hpp"
#include "Operate.hpp"


namespace Opm {

namespace {

namespace keywords {


/*
  If a keyword is not mentioned here the getSIValue() function will silently
  assume the keyword is dimensionless.
*/
static const std::map<std::string, std::string> unit_string = {{"PERMX", "Permeability"},
                                                               {"PERMY", "Permeability"},
                                                               {"PERMZ", "Permeability"},
                                                               {"PORV",  "ReservoirVolume"},
                                                               {"SPOLY", "Density"},
                                                               {"TRANX", "Transmissibility"},
                                                               {"TRANY", "Transmissibility"},
                                                               {"TRANZ", "Transmissibility"},
                                                               {"NTG", "1"},
                                                               {"RS", "GasDissolutionFactor"},
                                                               {"RV", "OilDissolutionFactor"},
                                                               {"TEMPI", "Temperature"},
                                                               {"THCROCK", "Energy/AnsoluteTemperature*Length*Time"},
                                                               {"THCOIL", "Energy/AnsoluteTemperature*Length*Time"},
                                                               {"THCGAS", "Energy/AnsoluteTemperature*Length*Time"},
                                                               {"THCWATER", "Energy/AnsoluteTemperature*Length*Time"}};

static const std::set<std::string> multiplier_keywords = {"MULTX", "MULTX-", "MULTY-", "MULTY", "MULTZ", "MULTZ-"};

static const std::set<std::string> oper_keywords = {"ADD", "EQUALS", "MAXVALUE", "MINVALUE", "MULTIPLY", "OPERATE"};
static const std::set<std::string> region_oper_keywords = {"ADDREG", "EQUALREG", "OPERATER"};
static const std::set<std::string> box_keywords = {"BOX", "ENDBOX"};
static const std::map<std::string, double> double_scalar_init = {{"NTG", 1},
                                                                 {"TRANX", 1},    // The default scalar init for TRAN is a hack to support
                                                                 {"TRANY", 1},    // TRAN modification in the deck. downstream implementation
                                                                 {"TRANZ", 1},    // in ecltransmissibility.hh - quite broken.
                                                                 {"MULTPV", 1},
                                                                 {"MULTX", 1},
                                                                 {"MULTX-", 1},
                                                                 {"MULTY", 1},
                                                                 {"MULTY-", 1},
                                                                 {"MULTZ", 1},
                                                                 {"MULTZ-", 1}};

static const std::map<std::string, int> int_scalar_init = {{"SATNUM", 1},
                                                           {"ENDNUM", 1},
                                                           {"EQLNUM", 1},
                                                           {"IMBNUM", 1},
                                                           {"ISOLNUM",1},
                                                           {"FIPNUM", 1},   // All FIPxxx keywords should (probably) be added with init==1
                                                           {"EQLNUM", 1},
                                                           {"PVTNUM", 1},
                                                           {"ACTNUM", 1}};


/*
bool isFipxxx< int >(const std::string& keyword) {
    // FIPxxxx can be any keyword, e.g. FIPREG or FIPXYZ that has the pattern "FIP.+"
    // However, it can not be FIPOWG as that is an actual keyword.
    if (keyword.size() < 4 || keyword == "FIPOWG") {
        return false;
    }
    return keyword[0] == 'F' && keyword[1] == 'I' && keyword[2] == 'P';
}
*/

namespace GRID {
static const std::set<std::string> double_keywords = {"MULTPV", "NTG", "PORO", "PERMX", "PERMY", "PERMZ", "THCONR", "MULTX", "MULTX-", "MULTY-", "MULTY", "MULTZ", "MULTZ-",
                                                      "THCONSF", "THCROCK", "THCOIL", "THCGAS", "THCWATER"};    // The THxxxx keywords are related to thermal properties - they are all E300 keywords.
static const std::set<std::string> int_keywords    = {"ACTNUM", "FLUXNUM", "ISOLNUM", "MULTNUM", "OPERNUM", "ROCKNUM"};
static const std::set<std::string> top_keywords    = {"PORO", "PERMX", "PERMY", "PERMZ"};
}

namespace EDIT {
static const std::set<std::string> double_keywords = {"MULTPV", "PORV","MULTX", "MULTX-", "MULTY-", "MULTY", "MULTZ", "MULTZ-", "TRANX", "TRANY", "TRANZ"};
static const std::set<std::string> int_keywords = {};
}

namespace PROPS {
static const std::set<std::string> double_keywords = {"SWATINIT"};
static const std::set<std::string> int_keywords = {};

#define dirfunc(base) base, base "X", base "X-", base "Y", base "Y-", base "Z", base "Z-"

static const std::set<std::string> satfunc = {"SWLPC", "ISWLPC", "SGLPC", "ISGLPC",
                                              dirfunc("SGL"),
                                              dirfunc("ISGL"),
                                              dirfunc("SGU"),
                                              dirfunc("ISGU"),
                                              dirfunc("SWL"),
                                              dirfunc("ISWL"),
                                              dirfunc("SWU"),
                                              dirfunc("ISWU"),
                                              dirfunc("SGCR"),
                                              dirfunc("ISGCR"),
                                              dirfunc("SOWCR"),
                                              dirfunc("ISOWCR"),
                                              dirfunc("SOGCR"),
                                              dirfunc("ISOGCR"),
                                              dirfunc("SWCR"),
                                              dirfunc("ISWCR"),
                                              dirfunc("PCW"),
                                              dirfunc("IPCW"),
                                              dirfunc("PCG"),
                                              dirfunc("IPCG"),
                                              dirfunc("KRW"),
                                              dirfunc("IKRW"),
                                              dirfunc("KRWR"),
                                              dirfunc("IKRWR"),
                                              dirfunc("KRO"),
                                              dirfunc("IKRO"),
                                              dirfunc("KRORW"),
                                              dirfunc("IKRORW"),
                                              dirfunc("KRORG"),
                                              dirfunc("IKRORG"),
                                              dirfunc("KRG"),
                                              dirfunc("IKRG"),
                                              dirfunc("KRGR"),
                                              dirfunc("IKRGR")};

static const std::map<std::string,std::string> sogcr_shift = {{"SOGCR",    "SWL"},
                                                              {"SOGCRX",   "SWLX"},
                                                              {"SOGCRX-",  "SWLX-"},
                                                              {"SOGCRY",   "SWLY"},
                                                              {"SOGCRY-",  "SWLY-"},
                                                              {"SOGCRZ",   "SWLZ"},
                                                              {"SOGCRZ-",  "SWLZ-"},
                                                              {"ISOGCR",   "ISWL"},
                                                              {"ISOGCRX",  "ISWLX"},
                                                              {"ISOGCRX-", "ISWLX-"},
                                                              {"ISOGCRY",  "ISWLY"},
                                                              {"ISOGCRY-", "ISWLY-"},
                                                              {"ISOGCRZ",  "ISWLZ"},
                                                              {"ISOGCRZ-", "ISWLZ-"}};

}

namespace REGIONS {
static const std::set<std::string> int_keywords = {"ENDNUM", "EQLNUM", "FIPNUM", "IMBNUM", "MISCNUM", "OPERNUM", "PVTNUM", "SATNUM", "LWSLTNUM", "ROCKNUM"};
}

namespace SOLUTION {
static const std::set<std::string> double_keywords = {"PRESSURE", "SPOLY", "SPOLYMW", "SSOL", "SWAT", "SGAS", "TEMPI", "RS", "RV"};
static const std::set<std::string> int_keywords = {};
}

namespace SCHEDULE {
static const std::set<std::string> int_keywords = {"ROCKNUM"};
static const std::set<std::string> double_keywords = {};
}
}

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
        const auto& gridOpts = deck.getKeyword("GRIDOPTS");
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
void assign_deck(const DeckKeyword& keyword, FieldProps::FieldData<T>& field_data, const std::vector<T>& deck_data, const std::vector<value::status>& deck_status, const Box& box) {
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
}


template <typename T>
void multiply_deck(const DeckKeyword& keyword, FieldProps::FieldData<T>& field_data, const std::vector<T>& deck_data, const std::vector<value::status>& deck_status, const Box& box) {
    verify_deck_data(keyword, deck_data, box);
    for (const auto& cell_index : box.index_list()) {
        auto active_index = cell_index.active_index;
        auto data_index = cell_index.data_index;

        if (value::has_value(deck_status[data_index]) && value::has_value(field_data.value_status[active_index])) {
            field_data.data[active_index] *= deck_data[data_index];
            field_data.value_status[active_index] = deck_status[data_index];
        }
    }
}


template <typename T>
void distribute_toplayer(const EclipseGrid& grid, FieldProps::FieldData<T>& field_data, const std::vector<T>& deck_data, const Box& box) {
    const std::size_t layer_size = grid.getNX() * grid.getNY();
    FieldProps::FieldData<double> toplayer(grid.getNX() * grid.getNY());
    for (const auto& cell_index : box.index_list()) {
        if (cell_index.global_index < layer_size) {
            toplayer.data[cell_index.global_index] = deck_data[cell_index.data_index];
            toplayer.value_status[cell_index.global_index] = value::status::deck_value;
        }
    }

    for (std::size_t active_index = 0; active_index < field_data.size(); active_index++) {
        if (field_data.value_status[active_index] == value::status::uninitialized) {
            std::size_t global_index = grid.getGlobalIndex(active_index);
            const auto ijk = grid.getIJK(global_index);
            std::size_t layer_index = ijk[0] + ijk[1] * grid.getNX();
            if (toplayer.value_status[layer_index] == value::status::deck_value) {
                field_data.data[active_index] = toplayer.data[layer_index];
                field_data.value_status[active_index] = value::status::valid_default;
            }
        }
    }
}


template <typename T>
void assign_scalar(FieldProps::FieldData<T>& field_data, T value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        field_data.data[cell_index.active_index] = value;
        field_data.value_status[cell_index.active_index] = value::status::deck_value;
    }
}

template <typename T>
void multiply_scalar(FieldProps::FieldData<T>& field_data, T value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(field_data.value_status[cell_index.active_index]))
            field_data.data[cell_index.active_index] *= value;
    }
}

template <typename T>
void add_scalar(FieldProps::FieldData<T>& field_data, T value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(field_data.value_status[cell_index.active_index]))
            field_data.data[cell_index.active_index] += value;
    }
}

template <typename T>
void min_value(FieldProps::FieldData<T>& field_data, T min_value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(field_data.value_status[cell_index.active_index])) {
            T value = field_data.data[cell_index.active_index];
            field_data.data[cell_index.active_index] = std::max(value, min_value);
        }
    }
}

template <typename T>
void max_value(FieldProps::FieldData<T>& field_data, T max_value, const std::vector<Box::cell_index>& index_list) {
    for (const auto& cell_index : index_list) {
        if (value::has_value(field_data.value_status[cell_index.active_index])) {
            T value = field_data.data[cell_index.active_index];
            field_data.data[cell_index.active_index] = std::min(value, max_value);
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

FieldProps::ScalarOperation fromString(const std::string& keyword) {
    if (keyword == ParserKeywords::ADD::keywordName || keyword == ParserKeywords::ADDREG::keywordName)
        return FieldProps::ScalarOperation::ADD;

    if (keyword == ParserKeywords::EQUALS::keywordName || keyword == ParserKeywords::EQUALREG::keywordName)
        return FieldProps::ScalarOperation::EQUAL;

    if (keyword == ParserKeywords::MULTIPLY::keywordName || keyword == ParserKeywords::MULTIREG::keywordName)
        return FieldProps::ScalarOperation::MUL;

    if (keyword == ParserKeywords::MINVALUE::keywordName)
        return FieldProps::ScalarOperation::MIN;

    if (keyword == ParserKeywords::MAXVALUE::keywordName)
        return FieldProps::ScalarOperation::MAX;

    throw std::invalid_argument("Keyword operation not recognized");
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

}



FieldProps::FieldProps(const Deck& deck, const Phases& phases, const EclipseGrid& grid, const TableManager& tables_arg) :
    active_size(grid.getNumActive()),
    global_size(grid.getCartesianSize()),
    unit_system(deck.getActiveUnitSystem()),
    nx(grid.getNX()),
    ny(grid.getNY()),
    nz(grid.getNZ()),
    m_phases(phases),
    m_actnum(grid.getACTNUM()),
    cell_volume(extract_cell_volume(grid)),
    cell_depth(extract_cell_depth(grid)),
    m_default_region(default_region_keyword(deck)),
    grid_ptr(&grid),
    tables(tables_arg)
{
    if (deck.hasKeyword<ParserKeywords::MULTREGP>()) {
        const DeckKeyword& multregpKeyword = deck.getKeyword("MULTREGP");
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

    FieldProps::compress(this->cell_volume, active_map);
    FieldProps::compress(this->cell_depth, active_map);

    this->m_actnum = std::move(new_actnum);
    this->active_size = new_active_size;
}


void FieldProps::distribute_toplayer(FieldProps::FieldData<double>& field_data, const std::vector<double>& deck_data, const Box& box) {
    const std::size_t layer_size = this->nx * this->ny;
    FieldProps::FieldData<double> toplayer(layer_size);
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
    if (keywords::GRID::double_keywords.count(keyword) != 0)
        return true;

    if (keywords::EDIT::double_keywords.count(keyword) != 0)
        return true;

    if (keywords::PROPS::double_keywords.count(keyword) != 0)
        return true;

    if (keywords::PROPS::satfunc.count(keyword) != 0)
        return true;

    if (keywords::SOLUTION::double_keywords.count(keyword) != 0)
        return true;

    return false;
}

template <>
bool FieldProps::supported<int>(const std::string& keyword) {
    if (keywords::REGIONS::int_keywords.count(keyword) != 0)
        return true;

    if (keywords::GRID::int_keywords.count(keyword) != 0)
        return true;

    if (keywords::SCHEDULE::int_keywords.count(keyword) != 0)
        return true;

    return false;
}

template <>
FieldProps::FieldData<double>& FieldProps::init_get(const std::string& keyword) {
    auto iter = this->double_data.find(keyword);
    if (iter != this->double_data.end())
        return iter->second;

    this->double_data[keyword] = FieldData<double>(this->active_size);
    auto init_iter = keywords::double_scalar_init.find(keyword);
    if (init_iter != keywords::double_scalar_init.end())
        this->double_data[keyword].default_assign(init_iter->second);

    if (keyword == ParserKeywords::PORV::keywordName)
        this->init_porv(this->double_data[keyword]);

    if (keyword == ParserKeywords::TEMPI::keywordName)
        this->init_tempi(this->double_data[keyword]);

    if (keywords::PROPS::satfunc.count(keyword) == 1) {
        this->init_satfunc(keyword, this->double_data[keyword]);

        if (this->tables.hasTables("SGOF")) {
            const auto shift_iter = keywords::PROPS::sogcr_shift.find(keyword);
            if (shift_iter != keywords::PROPS::sogcr_shift.end())
                this->subtract_swl(this->double_data[keyword], shift_iter->second);
        }
    }

    return this->double_data[keyword];
}



template <>
FieldProps::FieldData<int>& FieldProps::init_get(const std::string& keyword) {
    auto iter = this->int_data.find(keyword);
    if (iter != this->int_data.end())
        return iter->second;

    this->int_data[keyword] = FieldData<int>(this->active_size);
    auto init_iter = keywords::int_scalar_init.find(keyword);
    if (init_iter != keywords::int_scalar_init.end())
        this->int_data[keyword].default_assign(init_iter->second);

    return this->int_data[keyword];
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

std::vector<Box::cell_index> FieldProps::region_index( const DeckItem& region_item, int region_value ) {
    std::string region_name = region_item.defaultApplied(0) ? this->m_default_region : make_region_name(region_item.get<std::string>(0));
    return this->region_index(region_name, region_value);
}


template <>
bool FieldProps::has<double>(const std::string& keyword) const {
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
*/

template <>
std::vector<std::string> FieldProps::keys<double>() const {
    std::vector<std::string> klist;
    for (const auto& data_pair : this->double_data) {
        if (data_pair.second.valid() && data_pair.first != "PORV")
            klist.push_back(data_pair.first);
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
    const auto& iter = keywords::unit_string.find(keyword);
    std::string dim_string = "1";
    if (iter != keywords::unit_string.end())
        dim_string = iter->second;

    const auto& dim = this->unit_system.parse( dim_string );
    return dim.convertRawToSi(raw_value);
}





void FieldProps::handle_int_keyword(const DeckKeyword& keyword, const Box& box) {
    auto& field_data = this->init_get<int>(keyword.name());
    const auto& deck_data = keyword.getIntData();
    const auto& deck_status = keyword.getValueStatus();
    assign_deck(keyword, field_data, deck_data, deck_status, box);
}


void FieldProps::handle_double_keyword(Section section, const DeckKeyword& keyword, const Box& box) {
    auto& field_data = this->init_get<double>(keyword.name());
    const auto& deck_data = keyword.getSIDoubleData();
    const auto& deck_status = keyword.getValueStatus();

    if (section == Section::EDIT && keywords::multiplier_keywords.count(keyword.name()) == 1)
        multiply_deck(keyword, field_data, deck_data, deck_status, box);
    else
        assign_deck(keyword, field_data, deck_data, deck_status, box);


    if (section == Section::GRID) {
        if (field_data.valid())
            return;

        if (keywords::GRID::top_keywords.count(keyword.name()) == 1)
            this->distribute_toplayer(field_data, deck_data, box);
    }
}



template <typename T>
void FieldProps::apply(ScalarOperation op, FieldData<T>& data, T scalar_value, const std::vector<Box::cell_index>& index_list) {
    if (op == ScalarOperation::EQUAL)
        assign_scalar(data, scalar_value, index_list);

    else if (op == ScalarOperation::MUL)
        multiply_scalar(data, scalar_value, index_list);

    else if (op == ScalarOperation::ADD)
        add_scalar(data, scalar_value, index_list);

    else if (op == ScalarOperation::MIN)
        min_value(data, scalar_value, index_list);

    else if (op == ScalarOperation::MAX)
        max_value(data, scalar_value, index_list);
}

template <typename T>
void FieldProps::apply(const DeckRecord& record, FieldData<T>& target_data, const FieldData<T>& src_data, const std::vector<Box::cell_index>& index_list) {
    const std::string& func_name = record.getItem("OPERATION").get< std::string >(0);
    const double alpha           = record.getItem("PARAM1").get< double >(0);
    const double beta            = record.getItem("PARAM2").get< double >(0);
    Operate::function func       = Operate::get( func_name, alpha, beta );
    bool check_target            = (func_name == "MULTIPLY" || func_name == "POLY");

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
        const std::string& target_kw = record.getItem(0).get<std::string>(0);
        int region_value = record.getItem("REGION_NUMBER").get<int>(0);

        if (FieldProps::supported<double>(target_kw)) {
            auto& field_data = this->init_get<double>(target_kw);

            if (keyword.name() == ParserKeywords::OPERATER::keywordName) {
                // For the OPERATER keyword we fetch the region name from the deck record
                // with no extra hoops.
                const auto& index_list = this->region_index(record.getItem("REGION_NAME").get<std::string>(0), region_value);
                const std::string& src_kw = record.getItem("ARRAY_PARAMETER").get<std::string>(0);
                const auto& src_data = this->init_get<double>(src_kw);
                FieldProps::apply(record, field_data, src_data, index_list);
            } else {
                double value = record.getItem(1).get<double>(0);
                const auto& index_list = this->region_index(record.getItem("REGION_NAME"), region_value);
                if (keyword.name() != ParserKeywords::MULTIPLY::keywordName)
                    value = this->getSIValue(target_kw, value);
                FieldProps::apply(fromString(keyword.name()), field_data, value, index_list);
            }

            continue;
        }

        if (FieldProps::supported<int>(target_kw)) {
            continue;
        }

        //throw std::out_of_range("The keyword: " + keyword + " is not supported");
    }
}

/* This can just use a local box - no need for the manager */
void FieldProps::handle_operation(const DeckKeyword& keyword, Box box) {
    for (const auto& record : keyword) {
        const std::string& target_kw = record.getItem(0).get<std::string>(0);
        box.update(record);

        if (FieldProps::supported<double>(target_kw)) {
            auto& field_data = this->init_get<double>(target_kw);

            if (keyword.name() == ParserKeywords::OPERATE::keywordName) {
                const std::string& src_kw = record.getItem("ARRAY").get<std::string>(0);
                const auto& src_data = this->init_get<double>(src_kw);
                FieldProps::apply(record, field_data, src_data, box.index_list());
            } else {
                double scalar_value = record.getItem(1).get<double>(0);
                if (keyword.name() != ParserKeywords::MULTIPLY::keywordName)
                    scalar_value = this->getSIValue(target_kw, scalar_value);
                FieldProps::apply(fromString(keyword.name()), field_data, scalar_value, box.index_list());
            }

            continue;
        }


        if (FieldProps::supported<int>(target_kw)) {
            int scalar_value = static_cast<int>(record.getItem(1).get<double>(0));
            auto& field_data = this->init_get<int>(target_kw);
            FieldProps::apply(fromString(keyword.name()), field_data, scalar_value, box.index_list());
            continue;
        }

        //throw std::out_of_range("The keyword: " + target_kw + " is not supported");
    }
}


void FieldProps::handle_COPY(const DeckKeyword& keyword, Box box, bool region) {
    for (const auto& record : keyword) {
        const std::string& src_kw = record.getItem(0).get<std::string>(0);
        const std::string& target_kw = record.getItem(1).get<std::string>(0);
        std::vector<Box::cell_index> index_list;

        if (region) {
            int region_value = record.getItem(2).get<int>(0);
            const auto& region_item = record.getItem(4);
            index_list = this->region_index(region_item, region_value);
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

    if (keywords::oper_keywords.count(name) == 1)
        this->handle_operation(keyword, box);

    else if (keywords::region_oper_keywords.count(name) == 1)
        this->handle_region_operation(keyword);

    else if (keywords::box_keywords.count(name) == 1)
        handle_box_keyword(keyword, box);

    else if (name == ParserKeywords::COPY::keywordName)
        handle_COPY(keyword, box, false);

    else if (name == ParserKeywords::COPYREG::keywordName)
        handle_COPY(keyword, box, true);
}

/**********************************************************************/


void FieldProps::init_tempi(FieldData<double>& tempi) {
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

void FieldProps::init_porv(FieldData<double>& porv) {
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


void FieldProps::scanGRIDSection(const GRIDSection& grid_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : grid_section) {
        const std::string& name = keyword.name();

        if (keywords::GRID::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::GRID, keyword, box);
            continue;
        }

        if (keywords::GRID::int_keywords.count(name) == 1) {
            this->handle_int_keyword(keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}

void FieldProps::scanEDITSection(const EDITSection& edit_section) {
    Box box(*this->grid_ptr);
    for (const auto& keyword : edit_section) {
        const std::string& name = keyword.name();
        if (keywords::EDIT::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::EDIT, keyword, box);
            continue;
        }

        if (keywords::EDIT::int_keywords.count(name) == 1) {
            this->handle_int_keyword(keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}


void FieldProps::init_satfunc(const std::string& keyword, FieldData<double>& satfunc) {
    const auto& endnum = this->get<int>("ENDNUM");
    if (keyword[0] == 'I') {
        const auto& imbnum = this->get<int>("IMBNUM");
        satfunc.default_update(satfunc::init(keyword, this->tables, this->m_phases, this->cell_depth, imbnum, endnum));
    } else {
        const auto& satnum = this->get<int>("SATNUM");
        satfunc.default_update(satfunc::init(keyword, this->tables, this->m_phases, this->cell_depth, satnum, endnum));
    }
}

/**
 * Special purpose operation to make various *SOGCR* data elements
 * account for (scaled) connate water saturation.
 *
 * Must only be called if run uses SGOF, because that table is implicitly
 * defined in terms of connate water saturation.  Subtracts SWL only
 * if the data item was defaulted (i.e., extracted from unscaled table).
 */
void FieldProps::subtract_swl(FieldProps::FieldData<double>& sogcr, const std::string& swl_kw)
{
    const auto& swl = this->init_get<double>(swl_kw);
    for (std::size_t i = 0; i < sogcr.size(); i++) {
        if (value::defaulted(sogcr.value_status[i]))
            sogcr.data[i] -= swl.data[i];
    }
}



void FieldProps::scanPROPSSection(const PROPSSection& props_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : props_section) {
        const std::string& name = keyword.name();
        if (keywords::PROPS::satfunc.count(name) == 1) {
            this->handle_double_keyword(Section::PROPS, keyword, box);
            continue;
        }

        if (keywords::PROPS::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::PROPS, keyword, box);
            continue;
        }

        if (keywords::PROPS::int_keywords.count(name) == 1) {
            this->handle_int_keyword(keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}


void FieldProps::scanREGIONSSection(const REGIONSSection& regions_section) {
    Box box(*this->grid_ptr);

    for (const auto& keyword : regions_section) {
        const std::string& name = keyword.name();
        if (keywords::REGIONS::int_keywords.count(name) == 1) {
            this->handle_int_keyword(keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}


void FieldProps::scanSOLUTIONSection(const SOLUTIONSection& solution_section) {
    Box box(*this->grid_ptr);
    for (const auto& keyword : solution_section) {
        const std::string& name = keyword.name();
        if (keywords::SOLUTION::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::SOLUTION, keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}

void FieldProps::scanSCHEDULESection(const SCHEDULESection& schedule_section) {
    Box box(*this->grid_ptr);
    for (const auto& keyword : schedule_section) {
        const std::string& name = keyword.name();
        if (keywords::SCHEDULE::double_keywords.count(name) == 1) {
            this->handle_double_keyword(Section::SCHEDULE, keyword, box);
            continue;
        }

        if (keywords::SCHEDULE::int_keywords.count(name) == 1) {
            this->handle_int_keyword(keyword, box);
            continue;
        }

        this->handle_keyword(keyword, box);
    }
}

const std::string& FieldProps::default_region() const {
    return this->m_default_region;
}


template std::vector<bool> FieldProps::defaulted<int>(const std::string& keyword);
template std::vector<bool> FieldProps::defaulted<double>(const std::string& keyword);
}

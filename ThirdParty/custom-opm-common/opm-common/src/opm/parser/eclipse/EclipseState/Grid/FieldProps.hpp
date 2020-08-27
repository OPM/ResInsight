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

#ifndef FIELDPROPS_HPP
#define FIELDPROPS_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <opm/parser/eclipse/Deck/value_status.hpp>
#include <opm/parser/eclipse/Deck/DeckSection.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>

namespace Opm {

class Deck;
class EclipseGrid;
class TableManager;

class FieldProps {
public:

    struct MultregpRecord {
        int region_value;
        double multiplier;
        std::string region_name;


        MultregpRecord(int rv, double m, const std::string& rn) :
            region_value(rv),
            multiplier(m),
            region_name(rn)
        {}

    };

    enum class ScalarOperation {
         ADD = 1,
         EQUAL = 2,
         MUL = 3,
         MIN = 4,
         MAX = 5
    };

    template<typename T>
    static void compress(std::vector<T>& data, const std::vector<bool>& active_map) {
        std::size_t shift = 0;
        for (std::size_t g = 0; g < active_map.size(); g++) {
            if (active_map[g] && shift > 0) {
                data[g - shift] = data[g];
                continue;
            }

            if (!active_map[g])
                shift += 1;
        }

        data.resize(data.size() - shift);
    }

    enum class GetStatus {
         OK = 1,
         INVALID_DATA = 2,               // std::runtime_error
         MISSING_KEYWORD = 3,            // std::out_of_range
         NOT_SUPPPORTED_KEYWORD = 4      // std::logic_error
    };




    template<typename T>
    struct FieldData {
        std::vector<T> data;
        std::vector<value::status> value_status;
        mutable bool all_set;

        FieldData() = default;

        FieldData(std::size_t active_size) :
            data(std::vector<T>(active_size)),
            value_status(active_size, value::status::uninitialized),
            all_set(false)
        {
        }


        std::size_t size() const {
            return this->data.size();
        }

        bool valid() const {
            if (this->all_set)
                return true;

            static const std::array<value::status,2> invalid_value = {value::status::uninitialized, value::status::empty_default};
            const auto& it = std::find_first_of(this->value_status.begin(), this->value_status.end(), invalid_value.begin(), invalid_value.end());
            this->all_set = (it == this->value_status.end());

            return this->all_set;
        }

        void compress(const std::vector<bool>& active_map) {
            FieldProps::compress(this->data, active_map);
            FieldProps::compress(this->value_status, active_map);
        }

        void copy(const FieldData<T>& src, const std::vector<Box::cell_index>& index_list) {
            for (const auto& ci : index_list) {
                this->data[ci.active_index] = src.data[ci.active_index];
                this->value_status[ci.active_index] = src.value_status[ci.active_index];
            }
        }

        void default_assign(T value) {
            std::fill(this->data.begin(), this->data.end(), value);
            std::fill(this->value_status.begin(), this->value_status.end(), value::status::valid_default);
        }

        void default_assign(const std::vector<T>& src) {
            if (src.size() != this->size())
                throw std::invalid_argument("Size mismatch got: " + std::to_string(src.size()) + " expected: " + std::to_string(this->size()));

            std::copy(src.begin(), src.end(), this->data.begin());
            std::fill(this->value_status.begin(), this->value_status.end(), value::status::valid_default);
        }

        void default_update(const std::vector<T>& src) {
            if (src.size() != this->size())
                throw std::invalid_argument("Size mismatch got: " + std::to_string(src.size()) + " expected: " + std::to_string(this->size()));

            for (std::size_t i = 0; i < src.size(); i++) {
                if (!value::has_value(this->value_status[i])) {
                    this->value_status[i] = value::status::valid_default;
                    this->data[i] = src[i];
                }
            }
        }

        void update(std::size_t index, T value, value::status status) {
            this->data[index] = value;
            this->value_status[index] = status;
        }

    };


    template<typename T>
    struct FieldDataManager {
        const std::string& keyword;
        GetStatus status;
        const FieldData<T> * data_ptr;

        FieldDataManager(const std::string& k, GetStatus s, const FieldData<T> * d) :
            keyword(k),
            status(s),
            data_ptr(d)
        { }


        void verify_status() const {
            switch (status) {
            case FieldProps::GetStatus::OK:
                return;
            case FieldProps::GetStatus::INVALID_DATA:
                throw std::runtime_error("The keyword: " + keyword + " has not been fully initialized");
            case FieldProps::GetStatus::MISSING_KEYWORD:
                throw std::out_of_range("No such keyword in deck: " + keyword);
            case FieldProps::GetStatus::NOT_SUPPPORTED_KEYWORD:
                throw std::logic_error("The kewyord  " + keyword + " is not supported");
            }
        }

        const std::vector<T>* ptr() const {
            if (this->data_ptr)
                return std::addressof(this->data_ptr->data);
            else
                return nullptr;
        }

        const std::vector<T>& data() const {
            this->verify_status();
            return this->data_ptr->data;
        }

        const FieldData<T>& field_data() const {
            this->verify_status();
            return *this->data_ptr;
        }

        bool valid() const {
            return (this->status == GetStatus::OK);
        }

    };




    FieldProps(const Deck& deck, const Phases& phases, const EclipseGrid& grid, const TableManager& table_arg);
    void reset_actnum(const std::vector<int>& actnum);

    const std::string& default_region() const;

    std::vector<int> actnum();

    template <typename T>
    static bool supported(const std::string& keyword);

    template <typename T>
    bool has(const std::string& keyword) const;

    template <typename T>
    std::vector<std::string> keys() const;


    template <typename T>
    FieldDataManager<T> try_get(const std::string& keyword) {
        if (!FieldProps::supported<T>(keyword))
            return FieldDataManager<T>(keyword, GetStatus::NOT_SUPPPORTED_KEYWORD, nullptr);

        const FieldData<T> * field_data;
        bool has0 = this->has<T>(keyword);

        field_data = std::addressof(this->init_get<T>(keyword));
        if (field_data->valid())
            return FieldDataManager<T>(keyword, GetStatus::OK, field_data);

        if (!has0) {
            this->erase<T>(keyword);
            return FieldDataManager<T>(keyword, GetStatus::MISSING_KEYWORD, nullptr);
        }

        return FieldDataManager<T>(keyword, GetStatus::INVALID_DATA, nullptr);
    }


    template <typename T>
    const std::vector<T>& get(const std::string& keyword) {
        const auto& data = this->try_get<T>(keyword);
        return data.data();
    }



    template <typename T>
    std::vector<T> get_copy(const std::string& keyword, bool global) {
        bool has0 = this->has<T>(keyword);
        const auto& data = this->get<T>(keyword);

        if (has0) {
            if (global)
                return this->global_copy(data);
            else
                return data;
        } else {
            if (global)
                return this->global_copy(this->extract<T>(keyword));
            else
                return this->extract<T>(keyword);
        }
    }


    template <typename T>
    std::vector<bool> defaulted(const std::string& keyword) {
        const auto& field = this->init_get<T>(keyword);
        std::vector<bool> def(field.size());

        for (std::size_t i=0; i < def.size(); i++)
            def[i] = value::defaulted( field.value_status[i]);

        return def;
    }


    template <typename T>
    std::vector<T> global_copy(const std::vector<T>& data) const {
        std::vector<T> global_data(this->global_size);
        std::size_t i = 0;
        for (std::size_t g = 0; g < this->global_size; g++) {
            if (this->m_actnum[g]) {
                global_data[g] = data[i];
                i++;
            }
        }
        return global_data;
    }

    std::size_t active_size;
    std::size_t global_size;

    std::size_t num_int() const {
        return this->int_data.size();
    }

    std::size_t num_double() const {
        return this->double_data.size();
    }

private:
    void scanGRIDSection(const GRIDSection& grid_section);
    void scanEDITSection(const EDITSection& edit_section);
    void scanPROPSSection(const PROPSSection& props_section);
    void scanREGIONSSection(const REGIONSSection& regions_section);
    void scanSOLUTIONSection(const SOLUTIONSection& solution_section);
    void scanSCHEDULESection(const SCHEDULESection& schedule_section);
    double getSIValue(const std::string& keyword, double raw_value) const;
    template <typename T>
    void erase(const std::string& keyword);

    template <typename T>
    std::vector<T> extract(const std::string& keyword);

    template <typename T>
    void apply(const DeckRecord& record, FieldData<T>& target_data, const FieldData<T>& src_data, const std::vector<Box::cell_index>& index_list);

    template <typename T>
    static void apply(ScalarOperation op, FieldData<T>& data, T scalar_value, const std::vector<Box::cell_index>& index_list);

    template <typename T>
    FieldData<T>& init_get(const std::string& keyword);

    std::vector<Box::cell_index> region_index( const DeckItem& regionItem, int region_value );
    std::vector<Box::cell_index> region_index( const std::string& region_name, int region_value );
    void handle_operation(const DeckKeyword& keyword, Box box);
    void handle_region_operation(const DeckKeyword& keyword);
    void handle_COPY(const DeckKeyword& keyword, Box box, bool region);
    void distribute_toplayer(FieldProps::FieldData<double>& field_data, const std::vector<double>& deck_data, const Box& box);
    double get_beta(const std::string& func_name, const std::string& target_array, double raw_beta);
    double get_alpha(const std::string& func_name, const std::string& target_array, double raw_alpha);

    void handle_keyword(const DeckKeyword& keyword, Box& box);
    void handle_double_keyword(Section section, const DeckKeyword& keyword, const Box& box);
    void handle_int_keyword(const DeckKeyword& keyword, const Box& box);
    void init_satfunc(const std::string& keyword, FieldData<double>& satfunc);
    void init_porv(FieldData<double>& porv);
    void init_tempi(FieldData<double>& tempi);

    const UnitSystem unit_system;
    std::size_t nx,ny,nz;
    Phases m_phases;
    SatFuncControls m_satfuncctrl;
    std::vector<int> m_actnum;
    std::vector<double> cell_volume;
    std::vector<double> cell_depth;
    const std::string m_default_region;
    const EclipseGrid * grid_ptr;      // A bit undecided whether to properly use the grid or not ...
    const TableManager& tables;
    std::shared_ptr<satfunc::RawTableEndPoints> m_rtep;
    std::vector<MultregpRecord> multregp;
    std::unordered_map<std::string, FieldData<int>> int_data;
    std::unordered_map<std::string, FieldData<double>> double_data;
};

}
#endif

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

#ifndef FIELDPROPS_MANAGER_HPP
#define FIELDPROPS_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <opm/input/eclipse/EclipseState/Grid/TranCalculator.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldData.hpp>

namespace Opm {

class EclipseGrid;
class Deck;
class DeckKeyword;
class FieldProps;
class Phases;
class TableManager;
class NumericalAquifers;

class FieldPropsManager {


public:
    // The default constructor should be removed when the FieldPropsManager is mandatory
    // The default constructed fieldProps object is **NOT** usable
    FieldPropsManager() = default;
    FieldPropsManager(const Deck& deck, const Phases& ph, const EclipseGrid& grid, const TableManager& tables);
    virtual void reset_actnum(const std::vector<int>& actnum);
    const std::string& default_region() const;
    virtual std::vector<int> actnum() const;
    virtual std::vector<double> porv(bool global = false) const;


    void apply_schedule_keywords(const std::vector<DeckKeyword>& keywords);

    /// \brief Whether we can call methods on the manager
    bool is_usable() const;

    /*
     The number of cells in the fields managed by this FieldPropsManager.
     Initially this will correspond to the number of active cells in the grid
     used when constructing the FieldPropsManager, but using the reset_actnum()
     method it is possible to deactivate additional cells.
    */
    std::size_t active_size() const;

    bool operator==(const FieldPropsManager& other) const;
    static bool rst_cmp(const FieldPropsManager& full_arg, const FieldPropsManager& rst_arg);
    /*
      Because the FieldProps class can autocreate properties the semantics of
      get() and has() is slightly non intuitve:

      - The has<T>("KW") method will check if the current FieldProps container
        has an installed "KW" keyword; if the container has the keyword in
        question it will check if all elements have been assigned a value - only
        in that case will it return true. The has<T>("KW") method will *not* try
        to create a new keyword.

      - The has<T>("KW") method will *not* consult the supported<T>("KW")
        method; i.e. if you ask has<T>("UNKNOWN_KEYWORD") you will just get a
        false.

      - The get<T>("KW") method will try to create a new keyword if it does not
        already have the keyword you are asking for. This implies that you can
        get the following non intuitive sequence of events:

            FieldPropsManager fpm(deck, grid);

            fpm.has<int>("SATNUM");                => false
            auto satnum = fpm.get<int>("SATNUM");  => SATNUM is autocreated
            fpm.has<int>("SATNUM");                => true

      - When checking whether the container has the keyword you should rephrase
        the question slightly:

        * Does the container have the keyword *right now* => has<T>("KW")
        * Can the container provide the keyword => ptr = try_get<T>("KW")

      - It is quite simple to create a deck where the keywords are only partly
        initialized, all the methods in the FieldPropsManager only consider
        fully initialized keywords.
     */


    /*
      The get_copy() has exactly the same behaviour as get(), but the important
      difference is that said keyword is not already in the container it is not
      installed in the container; if we look at SATNUM which is a keywor which
      can be automatically instantiated we have the following behavior:

      get():
          fp.has<int>("SATNUM") -> false
          const std::vector<int>& satnum = fp.get<int>("SATNUM")
          fp.has<int>("SATNUM") -> true;


      get_copy():
          fp.has<int>("SATNUM") -> false
          const std::vector<int>& satnum = fp.get_copy<int>("SATNUM")
          fp.has<int>("SATNUM") -> false
    */


    template <typename T>
    std::vector<T> get_copy(const std::string& keyword, bool global=false) const;

    /*
      Will return a pointer to the keyword data, or nullptr if the container
      does not have suce a keyword. Observe that container will hold on to an
      manage the underlying keyword data.

      The try_get function will return a nullptr if the container does not
      contain said keyword, or if the keyword has not been fully initialized. If
      you ask for a totally unknown keyword the method will return nullptr.
    */
    template <typename T> const std::vector<T>* try_get(const
    std::string& keyword) const;

    /*
      You can ask whether the elements in the keyword have a default value -
      which typically is calculated in some way, or if it has been explicitly
      assigned to in the deck.
    */
    template <typename T>
    std::vector<bool> defaulted(const std::string& keyword) const;


    /*
      Check whether the container supports/recognizes a keyword at all:

        supported<double>("PORO")            => true
        supported<double>("NO_SUCH_KEYWORD") => false

      The method does not at all consult the content of the container - it is a
      static method.
    */
    template <typename T>
    static bool supported(const std::string& keyword);

    /*
      The keys() function will return a list of keys corresponding to the fully
      initialized keywords in the container. Observe that the implementation
      special cases the PORV and ACTNUM keywords, since these are present with
      special functions porv(bool) and actnum() the "PORV" and "ACTNUM" string
      literals are excluded from the keys() list.
    */
    template <typename T>
    std::vector<std::string> keys() const;

    const Fieldprops::FieldData<int>&
    get_int_field_data(const std::string& keyword) const;

    /// \brief Get double field data associated with a keyword
    /// \param allow_unsupported If true we deactivate some checks on the
    ///        keyword and thus allow getting FieldData used by the TranCalculator.
    const Fieldprops::FieldData<double>&
    get_double_field_data(const std::string& keyword, bool allow_unsupported=false) const;
    virtual const std::vector<int>& get_int(const std::string& keyword) const { return this->get<int>(keyword); }
    virtual std::vector<int> get_global_int(const std::string& keyword) const { return this->get_global<int>(keyword); }

    virtual const std::vector<double>& get_double(const std::string& keyword) const { return this->get<double>(keyword); }
    virtual std::vector<double> get_global_double(const std::string& keyword) const { return this->get_global<double>(keyword); }

    virtual bool has_int(const std::string& keyword) const { return this->has<int>(keyword); }
    virtual bool has_double(const std::string& keyword) const { return this->has<double>(keyword); }

    /*
      The transmissibility keywords TRANX, TRANY and TRANZ do not really fit
      well in the FieldProps system. The opm codebase is based on a full
      internalization in the parse phase, and then passing fully assembled
      objects to the simulator. When it comes to the transmissibilities this
      model breaks down because the input code in opm-common is not capable of
      calculating the transmissibility, that is performed in the simulator.

      The EDIT section can have modifiers on TRAN, these must be applied *after*
      the initial transmissibilities are calculated. To support this all the
      modifiers to the TRAN{XYZ} fields are assembled in "transmissibility
      calculators", and then these modifiers can be applied to a TRAN vector
      after it has been calculated in the simulator. Usage from the simulator
      could look like:


          const auto& fp = eclState.fieldProps();

          // Calculate transmissibilities using grid and permeability
          std::vector<double> tranx = ....

          // Check if there are any active TRANX modifiers and apply them
          if (fp.tran_active("TRANX"))
               fp.apply_tran("TRANX", tranx);


    */

    /*
      Will check if there are any TRAN{XYZ} modifiers active in the deck.
    */
    virtual bool tran_active(const std::string& keyword) const;


    /*
      Will apply all the TRAN modifiers which are present in the deck on the
      already initialized vector tran_data. The vector tran_data should be
      organised as the data vectors in the fieldpropsmanager - i.e. one element
      for each active cell - in lexicographical order. The operations which are
      supported by the transmissibility calculator are those given by the enum
      ScalarOperation in FieldProps.hpp.
    */
    virtual void apply_tran(const std::string& keyword, std::vector<double>& tran_data) const;

    void apply_numerical_aquifers(const NumericalAquifers& aquifers);


    /*
      When using MPI the FieldPropsManager is typically only assembled on the
      root node and then distributed to the other nodes afterwards. These
      methods are support methods for that, the real data used by the
      transmissibility calculators is in the form of custom 3D fields, they are
      distributed the same way the rest of the 3D fields are distributed.
    */
    virtual std::vector<char> serialize_tran() const;
    virtual void deserialize_tran(const std::vector<char>& buffer);
private:
    /*
      Return the keyword values as a std::vector<>. All elements in the return
      value are guaranteed to be assigned a valid value. If the keyword is not
      in the container, or not all elements have a valid value - an exception
      will be raised:

       - keyword which is not supported at all -> std::logic_error
       - keyword which is not in the deck at all -> std::out_of_range
       - keyword which has not been fully initialized -> std::runtime_error

      Many of the keywords in the container can be automatically created, in
      that case the get() method will silently create a new keyword and default
      initialize if it is not already in the container. The different exceptions
      raised for the different error conditions are the same for get(),
      get_copy() and get_global().
    */
    template <typename T>
    const std::vector<T>& get(const std::string& keyword) const;

    /*
      Will check if the container has the keyword loaded; in a fully initialized
      state. If you ask for a keyword which is not supported at all you will
      just get false back.
    */
    template <typename T>
    bool has(const std::string& keyword) const;

    /*
      This is exactly like the get() method, but the returned vector will have
      global cartesian size. If the field has a default value that value will be
      used for filling in in the inactive cells, otherwise zero is used.
    */
    template <typename T>
    std::vector<T> get_global(const std::string& keyword) const;


    std::shared_ptr<FieldProps> fp;
};


void deserialize_tran(std::unordered_map<std::string, Fieldprops::TranCalculator>& tran,
                      const std::vector<char>& buffer);

template<class MapType>
void apply_tran(const std::unordered_map<std::string, Fieldprops::TranCalculator>& tran,
                const MapType& double_data,
                std::size_t active_size,
                const std::string& keyword, std::vector<double>& data);

}

#endif

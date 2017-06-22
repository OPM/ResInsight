/*
  Copyright 2016, 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <opm/utility/ECLResultData.hpp>

#include <cassert>
#include <ctime>
#include <exception>
#include <initializer_list>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_file_kw.h>
#include <ert/ecl/ecl_file_view.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_nnc_export.h>
#include <ert/ecl/ecl_util.h>
#include <ert/util/ert_unique_ptr.hpp>

#if defined(HAVE_ERT_ECL_TYPE_H) && HAVE_ERT_ECL_TYPE_H
#include <ert/ecl/ecl_type.h>
#endif // defined(HAVE_ERT_ECL_TYPE_H) && HAVE_ERT_ECL_TYPE_H

/// \file
///
/// Implementation of ECL Result-Set Interface.

namespace {
    inline ecl_type_enum
    getKeywordElementType(const ecl_kw_type* kw)
    {
#if defined(HAVE_ERT_ECL_TYPE_H) && HAVE_ERT_ECL_TYPE_H
        return ecl_type_get_type(ecl_kw_get_data_type(kw));

#else // ! (defined(HAVE_ERT_ECL_TYPE_H) && HAVE_ERT_ECL_TYPE_H)

        return ecl_kw_get_type(kw);
#endif // defined(HAVE_ERT_ECL_TYPE_H) && HAVE_ERT_ECL_TYPE_H
    }

    namespace ECLImpl {
        using FilePtr = std::shared_ptr<ecl_file_type>;

        namespace Details {

            inline std::string
            firstBlockKeyword(const ecl_file_view_type* block)
            {
                return ecl_kw_get_header(ecl_file_view_iget_kw(block, 0));
            }

            /// Convert vector of elements from one element type to another.
            ///
            /// \tparam Input Element type of input collection.
            template <typename Input>
            struct Convert
            {
                /// Convert from one element type to another.
                ///
                /// Implements case of *different* element types.
                ///
                /// \tparam Output Element type of output collection.  Must
                ///    be different from \c Input.
                ///
                /// \param[in] x Input vector.
                ///
                /// \return Result vector (elements of \p x converted to
                ///    type \p Output).
                template <typename Output>
                static std::vector<Output>
                to(const std::vector<Input>& x, std::false_type)
                {
                    static_assert(! std::is_same<Output, Input>::value,
                                  "Logic Error: Convert<Input>::to<Output>"
                                  " for Output!=Input");

                    auto result = std::vector<Output>{};
                    result.reserve(x.size());

                    for (const auto& xi : x) {
                        // push_back(T(xi)) because vector<bool> does not
                        // support emplace_back until C++14.
                        result.push_back(Output(xi));
                    }

                    return result;
                }

                /// Convert from one element type to another.
                ///
                /// Implements special case of same element types.  This is
                /// the identity operator.
                ///
                /// \tparam Output Element type of output collection.  Must
                ///    be the same as \c Input.
                ///
                /// \param[in] x Input vector.
                ///
                /// \return Result vector (copy of \p x).
                ///
                template <typename Output>
                static std::vector<Output>
                to(const std::vector<Input>& x, std::true_type)
                {
                    static_assert(std::is_same<Output, Input>::value,
                                  "Logic Error: Convert<Input>::to<Output>"
                                  " for Output==Input");

                    return x;
                }
            };

            /// Retrieve keyword data elements, possibly converted to
            /// different destination data type.
            ///
            /// \tparam Output Destination data type.  Type of result
            ///    vector.  Assumed to be arithmetic.
            ///
            /// \tparam Input Source data type.  Type of keyword data
            ///    elements.  Assumed to be arithmetic.
            ///
            /// \tparam GetData Data element accessor.  Callable.
            ///
            /// \param[in] kw Particular ECLIPSE result-set data vector.
            ///
            /// \param[in] extractElements Accessor for keyword data
            ///    elements.  Must support the syntax
            ///
            ///    \code
            ///       extractElements(const ecl_kw_type* kw, Input* x)
            ///    \endcode
            ///
            /// \return Data elements of keyword as a \code
            ///    std::vector<Output> \endcode.
            template <typename Output, typename Input, class GetData>
            std::vector<Output>
            getData(const ecl_kw_type* kw, GetData&& extractElements)
            {
                auto x = std::vector<Input>(ecl_kw_get_size(kw));

                extractElements(kw, x.data());

                return Convert<Input>::template to<Output>
                    (x, typename std::is_same<Output, Input>::type());
            }

            /// Translate ERT type class to keyword element type.
            ///
            /// Primary template.
            ///
            /// \tparam Input Class of ERT keyword data.
            template <ecl_type_enum Input>
            struct ElementType
            {
                /// Undefined element type.
                using type = void;
            };

            /// Translate ERT type class to keyword element type.
            ///
            /// Actual element type of \code ECL_INT_TYPE \endcode.
            template <>
            struct ElementType<ECL_BOOL_TYPE>
            {
                /// Element type of ERT Boolean (LOGICAL) data.  Stored
                /// internally as 'int'.
                using type = int;
            };

            /// Translate ERT type class to keyword element type.
            ///
            /// Actual element type of \code ECL_INT_TYPE \endcode.
            template <>
            struct ElementType<ECL_INT_TYPE>
            {
                /// Element type of ERT integer data.
                using type = int;
            };

            /// Translate ERT type class to keyword element type.
            ///
            /// Actual element type of \code ECL_FLOAT_TYPE \endcode.
            template <>
            struct ElementType<ECL_FLOAT_TYPE>
            {
                /// Element type of ERT floating-point (float) data.
                using type = float;
            };

            /// Translate ERT type class to keyword element type.
            ///
            /// Actual element type of \code ECL_DOUBLE_TYPE \endcode.
            template <>
            struct ElementType<ECL_DOUBLE_TYPE>
            {
                /// Element type of ERT floating-point (double) data.
                using type = double;
            };

            /// Extract ERT keyword data for various element types.
            ///
            /// Primary template.
            ///
            /// \tparam Input Class of ERT keyword data.
            template <ecl_type_enum Input>
            struct ExtractKeywordElements;

            /// Extract ERT keyword Boolean (LOGICAL) data.
            template <>
            struct ExtractKeywordElements<ECL_BOOL_TYPE>
            {
                using EType = ElementType<ECL_BOOL_TYPE>::type;

                /// Function call operator.
                ///
                /// Retrieve actual data elements from ERT keyword of integer
                /// (specifically, \c int) type.
                ///
                /// \param[in] kw ERT keyword instance.
                ///
                /// \param[in,out] x Linearised keyword data elements.  On
                ///    input points to memory block of size \code
                ///    ecl_kw_get_size(kw) * sizeof *x \endcode bytes.  On
                ///    output, those bytes are filled with the actual data
                ///    values of \p kw.
                void operator()(const ecl_kw_type* kw, EType* x) const
                {
                    // 1) Extract raw 'int' values.
                    ecl_kw_get_memcpy_int_data(kw, x);

                    // 2) Convert to 'bool'-like values by comparing to
                    //    magic constant ECL_BOOL_TRUE_INT (ecl_util.h).
                    for (auto n = ecl_kw_get_size(kw), i = 0*n; i < n; ++i) {
                        x[i] = static_cast<EType>(x[i] == ECL_BOOL_TRUE_INT);
                    }
                }
            };

            /// Extract ERT keyword integer data.
            template <>
            struct ExtractKeywordElements<ECL_INT_TYPE>
            {
                using EType = ElementType<ECL_INT_TYPE>::type;

                /// Function call operator.
                ///
                /// Retrieve actual data elements from ERT keyword of integer
                /// (specifically, \c int) type.
                ///
                /// \param[in] kw ERT keyword instance.
                ///
                /// \param[in,out] x Linearised keyword data elements.  On
                ///    input points to memory block of size \code
                ///    ecl_kw_get_size(kw) * sizeof *x \endcode bytes.  On
                ///    output, those bytes are filled with the actual data
                ///    values of \p kw.
                void operator()(const ecl_kw_type* kw, EType* x) const
                {
                    ecl_kw_get_memcpy_int_data(kw, x);
                }
            };

            /// Extract ERT keyword \c float data.
            template <>
            struct ExtractKeywordElements<ECL_FLOAT_TYPE>
            {
                using EType = ElementType<ECL_FLOAT_TYPE>::type;

                /// Function call operator.
                ///
                /// Retrieve actual data elements from ERT keyword of
                /// floating-point (specifically, \c float) type.
                ///
                /// \param[in] kw ERT keyword instance.
                ///
                /// \param[in,out] x Linearised keyword data elements.  On
                ///    input points to memory block of size \code
                ///    ecl_kw_get_size(kw) * sizeof *x \endcode bytes.  On
                ///    output, those bytes are filled with the actual data
                ///    values of \p kw.
                void operator()(const ecl_kw_type* kw, EType* x) const
                {
                    ecl_kw_get_memcpy_float_data(kw, x);
                }
            };

            /// Extract ERT keyword \c double data.
            template <>
            struct ExtractKeywordElements<ECL_DOUBLE_TYPE>
            {
                using EType = ElementType<ECL_DOUBLE_TYPE>::type;

                /// Function call operator.
                ///
                /// Retrieve actual data elements from ERT keyword of
                /// floating-point (specifically, \c double) type.
                ///
                /// \param[in] kw ERT keyword instance.
                ///
                /// \param[in,out] x Linearised keyword data elements.  On
                ///    input points to memory block of size \code
                ///    ecl_kw_get_size(kw) * sizeof *x \endcode bytes.  On
                ///    output, those bytes are filled with the actual data
                ///    values of \p kw.
                void operator()(const ecl_kw_type* kw, EType* x) const
                {
                    ecl_kw_get_memcpy_double_data(kw, x);
                }
            };
        } // namespace Details

        /// Extract data elements from ECL keyword.
        ///
        /// Primary template--handles non-string (non-char) types.
        ///
        /// \tparam Input Type of keyword data.
        template <ecl_type_enum Input>
        struct GetKeywordData
        {
            /// Retrieve arithmetic (non-string) ECL keyword data as a \code
            /// std::vector<T> \endcode for arithmetic types \c T.
            ///
            /// \tparam T Element type of result vector.  Must not be a
            ///    string type.
            ///
            /// \param[in] kw ECL keyword instance.  Its element type must
            ///    be commensurate with \p Input.
            ///
            /// \return Keyword data elements as a \code std::vector<T>
            ///    \endcode.
            template <typename T>
            static std::vector<T>
            as(const ecl_kw_type* kw, std::false_type)
            {
                assert (getKeywordElementType(kw) == Input);

                return Details::getData<
                        T, typename Details::ElementType<Input>::type
                    >(kw, Details::ExtractKeywordElements<Input>{});
            }

            /// Retrieve vector of string data.
            ///
            /// Not supported for non-char keyword element types.
            ///
            /// \tparam T Element type of result vector.  Assumed to be
            ///    \code std::string \endcode.
            ///
            /// \return Empty string vector.  String data is unsupported for
            ///    non-char input types.
            template <typename T>
            static std::vector<std::string>
            as(const ecl_kw_type* /* kw */, std::true_type)
            {
                return {};
            }
        };

        /// Extract data elements from ECL character-type keyword.
        ///
        /// Only supports accessing elements as a vector of \code
        /// std::string \endcode.
        template <>
        struct GetKeywordData<ECL_CHAR_TYPE>
        {
            /// Retrieve arithmetic (non-string) ECL keyword data as a \code
            /// std::vector<T> \endcode for arithmetic types \c T.
            ///
            /// Not supported for character input types.
            ///
            /// \tparam T Element type of result vector.  Must not be a
            ///    string type.
            ///
            /// \return Empty data vector.
            template <typename T>
            static std::vector<T>
            as(const ecl_kw_type* /* kw */, std::false_type)
            {
                return {};
            }

            /// Retrieve vector of string data.
            ///
            /// \tparam T Element type of result vector.  Assumed to be
            ///    \code std::string \endcode.
            ///
            /// \return Keyword data elements as a \code
            ///    std::vector<std::string> \endcode.
            template <typename T>
            static std::vector<std::string>
            as(const ecl_kw_type* kw, std::true_type)
            {
                assert (getKeywordElementType(kw) == ECL_CHAR_TYPE);

                auto result = std::vector<std::string>{};
                result.reserve(ecl_kw_get_size(kw));

                for (decltype(ecl_kw_get_size(kw))
                         i = 0, nkw = ecl_kw_get_size(kw);
                     i < nkw; ++i)
                {
                    result.emplace_back(ecl_kw_iget_char_ptr(kw, i));

                    // Trim trailing white-space.
                    auto& s = result.back();

                    const auto e = s.find_last_not_of(" \t");
                    if (e != std::string::npos) {
                        s = s.substr(0, e + 1);
                    }
                }

                return result;
            }
        };

        /// Translate grid names to (local) numeric IDs within a
        /// section/view of an ECL result set.
        ///
        /// Provides a caching mechanism to accelerate repeated lookup.
        class GridIDCache
        {
        public:
            /// Constructor
            ///
            /// \param[in] block View relative to which to interpret grid
            ///    names.
            explicit GridIDCache(const ecl_file_view_type* block);

            /// Get integral grid ID of particular grid relative to cache's
            /// view.
            ///
            /// \param[in] gridName Name of particular grid.  Empty for the
            ///    main grid (grid ID 0).
            ///
            /// \return Numeric grid ID of \p gridName.  Negative if \p
            ///    gridName does not correspond to a grdi in the cache's
            ///    view.
            int getGridID(const std::string& gridName) const;

        private:
            /// View into ECL result set.
            const ecl_file_view_type* block_;

            /// Cache of name->ID map.
            mutable std::unordered_map<std::string, int> cache_;
        };

        /// Partition INIT file into sections
        class InitFileSections
        {
        public:
            explicit InitFileSections(const ecl_file_type* init);

            struct Section {
                Section(const ecl_file_view_type* blk)
                    : block   (blk)
                    , first_kw(Details::firstBlockKeyword(block))
                {}

                const ecl_file_view_type* block;
                std::string               first_kw;
            };

            const std::vector<Section>& sections() const
            {
                return this->sect_;
            }

            using SectionID = std::vector<Section>::size_type;

            SectionID numSections() const
            {
                return this->sections().size();
            }

            const Section& operator[](const SectionID i) const
            {
                assert ((i < this->numSections()) && "Internal Error");

                return this->sect_[i];
            }

        private:
            const ecl_file_view_type* init_;

            std::vector<Section> sect_;
        };

        template <typename T>
        std::vector<T> getKeywordData(const ecl_kw_type* kw)
        {
            // Whether or not caller requests a vector<string>.
            const auto makeStringVector =
                typename std::is_same<T, std::string>::type{};

            switch (getKeywordElementType(kw)) {
            case ECL_CHAR_TYPE:
                return GetKeywordData<ECL_CHAR_TYPE>::
                    as<T>(kw, makeStringVector);

            case ECL_BOOL_TYPE:
                return GetKeywordData<ECL_BOOL_TYPE>::
                    as<T>(kw, makeStringVector);

            case ECL_INT_TYPE:
                return GetKeywordData<ECL_INT_TYPE>::
                    as<T>(kw, makeStringVector);

            case ECL_FLOAT_TYPE:
                return GetKeywordData<ECL_FLOAT_TYPE>::
                    as<T>(kw, makeStringVector);

            case ECL_DOUBLE_TYPE:
                return GetKeywordData<ECL_DOUBLE_TYPE>::
                    as<T>(kw, makeStringVector);

            default:
                // No operator exists for this type.  Return empty.
                return {};
            }
        }
    } // namespace ECLImpl

    /// Predicate for whether or not a particular path represents a regular
    /// file.
    ///
    /// \param[in] p Filesystem element.
    ///
    /// \return Whether or not the element represented by \p p exists and is
    ///   (or points to, in the case of a symbolic link) a regular file.
    bool isFile(const boost::filesystem::path& p)
    {
        namespace fs = boost::filesystem;

        auto is_regular_file = [](const fs::path& pth)
        {
            return fs::exists(pth) && fs::is_regular_file(pth);
        };

        return is_regular_file(p)
            || (fs::is_symlink(p) &&
                is_regular_file(fs::read_symlink(p)));
    }

    /// Derive filesystem element from prefix or filename.
    ///
    /// Pass-through if the input already is a regular file in order to
    /// support accessing result-sets other than restart files (e.g., the
    /// INIT vectors).
    ///
    /// Fails (throws an exception of type \code std::invalid_argument
    /// \endcode) if no valid filesystem element can be derived from the
    /// input argument
    ///
    /// \param[in] file Filename or casename prefix.
    ///
    /// \param[in] ext Set of possible filename extensions.
    ///
    /// \return Filesystem element corresponding to result-set.  Either the
    ///    input \p file itself or, in the case of a filename prefix, the
    ///    first possible match amongst the set generated by the prefix and
    ///    the input extensions.
    boost::filesystem::path
    deriveResultPath(boost::filesystem::path         file,
                     const std::vector<std::string>& ext)
    {
        if (isFile(file)) {
            return file;
        }

        for (const auto& e : ext) {
            file.replace_extension(e);

            if (isFile(file)) {
                return file;
            }
        }

        const auto prefix = file.parent_path() / file.stem();

        std::ostringstream os;

        os << "Unable to derive valid filename from model prefix "
           << prefix.generic_string();

        throw std::invalid_argument(os.str());
    }

    /// Derive filesystem element from prefix or filename.
    ///
    /// Pass-through if the input already is a regular file in order to
    /// support accessing result-sets other than restart files (e.g., the
    /// INIT vectors).
    ///
    /// Fails (throws an exception of type \code std::invalid_argument
    /// \endcode) if no valid filesystem element can be derived from the
    /// input argument
    ///
    /// \param[in] file Filename or casename prefix.
    ///
    /// \return Filesystem element corresponding to result-set.  Either the
    ///    input \p file itself or, in the case of a casename prefix, the
    ///    path to a restart file (unified format only).
    boost::filesystem::path
    deriveRestartPath(boost::filesystem::path file)
    {
        return deriveResultPath(std::move(file), { ".UNRST", ".FUNRST" });
    }

    /// Derive filesystem element from prefix or filename.
    ///
    /// Pass-through if the input already is a regular file in order to
    /// support accessing result-sets other than restart files (e.g., the
    /// INIT vectors).
    ///
    /// Fails (throws an exception of type \code std::invalid_argument
    /// \endcode) if no valid filesystem element can be derived from the
    /// input argument
    ///
    /// \param[in] file Filename or casename prefix.
    ///
    /// \return Filesystem element corresponding to result-set.  Either the
    ///    input \p file itself or, in the case of a casename prefix, the
    ///    path to an INIT file (unformatted or formatted).
    boost::filesystem::path
    deriveInitPath(boost::filesystem::path file)
    {
        return deriveResultPath(std::move(file), { ".INIT", ".FINIT" });
    }

    /// Open ECL result set from pathname.
    ///
    /// Fails (throws an exception of type \code std::invalid_argument
    /// \endcode) if the input argument does not refer to a valid filesystem
    /// element.
    ///
    /// \param[in] file Filename.
    ///
    /// \return Open stream corresponding to result-set.
    ECLImpl::FilePtr openResultSet(const boost::filesystem::path& fname)
    {
        // Read-only, keep open between requests
        const auto open_flags = 0;

        auto F = ECLImpl::FilePtr {
            ecl_file_open(fname.generic_string().c_str(), open_flags),
            ecl_file_close
        };

        if (! F) {
            std::ostringstream os;

            os << "Failed to load ECL Result object from "
               << fname.generic_string();

            throw std::invalid_argument(os.str());
        }

        return F;
    }

    /// Retrieve first keyword from file
    ///
    /// Useful in order to identify a result-set as either a unified restart
    /// file or some other result-set type.
    ///
    /// \param[in] file Raw result-set.
    ///
    /// \return First keyword in result-set represented by \p file.
    std::string firstFileKeyword(const ecl_file_type* file)
    {
        // Note: ecl_file_get_global_view() does not modify its input.
        auto* globView =
            ecl_file_get_global_view(const_cast<ecl_file_type*>(file));

        return ECLImpl::Details::firstBlockKeyword(globView);
    }

    std::string paddedGridName(const std::string& gridName)
    {
        if (gridName.empty()) {
            return gridName;
        }

        std::ostringstream os;

        os << std::setw(8) << std::left << gridName;

        return os.str();
    }
} // namespace Anonymous

// ======================================================================
// Class (Anonymous)::ECLImpl::GridIDCache
// ======================================================================

ECLImpl::GridIDCache::GridIDCache(const ecl_file_view_type* block)
    : block_(block)
{}

int ECLImpl::GridIDCache::getGridID(const std::string& gridName) const
{
    if (gridName.empty()) {
        return ECL_GRID_MAINGRID_LGR_NR;
    }

    {
        auto i = this->cache_.find(gridName);
        if (i != std::end(this->cache_)) {
            return i->second;
        }
    }

    const auto nLGR = ecl_file_view_get_num_named_kw
        (this->block_, LGR_KW);

    auto lgrID = 0 * nLGR;
    for (; lgrID < nLGR; ++lgrID)
    {
        const auto* kw = ecl_file_view_iget_named_kw
            (this->block_, LGR_KW, lgrID);

        if ((getKeywordElementType(kw) != ECL_CHAR_TYPE) ||
            (ecl_kw_get_size(kw) != 1))
        {
            // Huh !?!
            continue;
        }

        const auto kwname = GetKeywordData<ECL_CHAR_TYPE>
            ::as<std::string>(kw, std::true_type());

        if (kwname[0] == gridName) {
            break;
        }
    }

    if (lgrID == nLGR) {
        // No such LGR in block.  Somewhat surprising.
        return -1;
    }

    return this->cache_[gridName] =
        ECL_GRID_MAINGRID_LGR_NR + 1 + lgrID;
}

// ======================================================================
// Class (Anonymous)::ECLImpl::InitFileSections
// ======================================================================

ECLImpl::InitFileSections::InitFileSections(const ecl_file_type* init)
    // Note: ecl_file_get_global_view() does not modify input arg
    : init_(ecl_file_get_global_view(const_cast<ecl_file_type*>(init)))
{
    const auto* endLGR_kw = "LGRSGONE";
    const auto nEndLGR =
        ecl_file_view_get_num_named_kw(this->init_, endLGR_kw);

    if (nEndLGR == 0) {
        // No LGRs in model.  INIT file consists of global section only,
        // meaning that the only available section is equal to the global
        // view (i.e., this->init_).
        this->sect_.push_back(Section{ this->init_ });
    }
    else {
        const auto* start_kw = INTEHEAD_KW;
        const auto* end_kw   = endLGR_kw;

        this->sect_.reserve(2 * nEndLGR);

        for (auto sectID = 0*nEndLGR; sectID < nEndLGR; ++sectID) {
            // Note: Start keyword occurrence lags one behind section ID
            // when creating sections *between* LGRSGONE keywords.
            const auto start_kw_occurrence = (sectID > 0)
                ? sectID - 1 : 0*sectID;

            // Main section 'sectID': [ start_kw, LGRSGONE ]
            const auto* sect =
                ecl_file_view_add_blockview2(this->init_, start_kw,
                                             end_kw, start_kw_occurrence);

            if (sect == nullptr) {
                continue;
            }

            const auto firstkw = Details::firstBlockKeyword(sect);
            const auto occurrence = 0;

            // Main grid sub-section of 'sectID': [ start_kw, LGR ]
            const auto* main_grid_sect =
                ecl_file_view_add_blockview2(sect, firstkw.c_str(),
                                             LGR_KW, occurrence);

            // LGR sub-section of 'sectID': [ LGR, LGRSGONE ]
            const auto* lgr_sect =
                ecl_file_view_add_blockview2(sect, LGR_KW,
                                             end_kw, occurrence);

            // Note: main_grid_sect or lgr_sect *may* (in rare cases) be
            // nullptr, but we'll deal with that in the calling context.
            this->sect_.push_back(Section { main_grid_sect });
            this->sect_.push_back(Section { lgr_sect });

            // start_kw == end_kw for all but first section.
            start_kw = end_kw;
        }
    }
}

// ======================================================================
// Class Opm::ECLRestartData::Impl
// ======================================================================

/// Engine powering implementation of \c ECLRestartData interface
class Opm::ECLRestartData::Impl
{
public:
    using Path = boost::filesystem::path;

    /// Constructor
    ///
    /// \param[in] rstrt Filesystem element or casename prefix representing
    ///    an ECL result-set.
    Impl(Path rstrt);

    /// Constructor
    ///
    /// \param[in] rstrt ECL restart result set
    Impl(std::shared_ptr<ecl_file_type> rstrt);

    /// Copy constructor.
    ///
    /// \param[in] rhs Object from which to construct new \c Impl instance.
    Impl(const Impl& rhs);

    /// Move constructor.
    ///
    /// \param[in,out] rhs Object from which to constructo new \c Impl
    ///    instance.  Underlying result-set accessor is null upon return.
    Impl(Impl&& rhs);

    /// Select a result-set view that corresponds to a single report step.
    ///
    /// This is needed when working with dynamic restart data.
    ///
    /// \param[in] step Report step number.
    ///
    /// \return Whether or not selecting the report step succeeded.  The
    ///    typical failure case is the report step not being available
    ///    in the result-set.
    bool selectReportStep(const int step);

    /// Query current result-set view for availability of particular named
    /// result vector in particular enumerated grid.
    ///
    /// \param[in] vector Named result vector for which to query data
    ///    availability.
    ///
    /// \param[in] gridID Identity of specific grid for which to query data
    ///    availability.
    ///
    /// \return Whether or not keyword data for the named result vector is
    ///    available in the specific grid.
    bool haveKeywordData(const std::string& vector,
                         const std::string& gridName) const;

    /// Retrieve current result-set view's data values for particular named
    /// result vector in particular enumerated grid.
    ///
    /// Will fail (throw an exception of type std::invalid_argument) unless
    /// the requested keyword data is available in the specific grid in the
    /// current active view.
    ///
    /// \tparam T Element type of return value.  The underlying keyword data
    ///    will be converted to this type if needed and possible.  Note that
    ///    some type combinations do not make sense.  It is, for instance,
    ///    not possible to retrieve keyword values of an underlying
    ///    arithmetic type in the form of a \code std::vector<std::string>
    ///    \endcode.  Similarly, we cannot access underlying character data
    ///    through elements of an arithmetic type (e.g., \code
    ///    std::vector<double> \endcode.)
    ///
    /// \param[in] vector Named result vector for which to retrieve
    ///    keyword data.
    ///
    /// \param[in] gridID Identity of specific grid for which to
    ///    retrieve keyword data.
    ///
    /// \return Keyword data values.  Empty if type conversion fails.
    template <typename T>
    std::vector<T>
    keywordData(const std::string& vector,
                const std::string& gridName) const;

private:
    /// RAII class to select a sub-block pertaining to a particular grid
    /// within the current active result-set view.
    ///
    /// An object of this type restricts the view to a particular grid in
    /// the constructor and restores the original active view in the
    /// destructor.
    class Restrict
    {
    public:
        /// Constructor.
        ///
        /// Restricts active result-set view to particular grid.
        ///
        /// \param[in] host Result-set host object that maintains the
        ///    current active view.
        ///
        /// \param[in] gridID Identity of specific grid to which to restrict
        ///    the current active result-set view.
        Restrict(const Impl& host, const int gridID)
            : host_(host)
            , save_(host_.activeBlock_)
        {
            if (gridID == ECL_GRID_MAINGRID_LGR_NR) {
                const auto& start = this->host_.mainGridStart();

                this->host_.activeBlock_ =
                    ecl_file_view_add_blockview2(this->host_.activeBlock_,
                                                 start.c_str(), LGR_KW, 0);
            }
            else if (gridID > ECL_GRID_MAINGRID_LGR_NR) {
                this->host_.activeBlock_ =
                    ecl_file_view_add_blockview2(this->host_.activeBlock_,
                                                 LGR_KW, LGR_KW, gridID - 1);
            }
        }

        /// Destructor.
        ///
        /// Restores original active result-set view (i.e., widens view to
        /// encompass all grids).
        ~Restrict()
        {
            this->host_.activeBlock_ = this->save_;
        }

    private:
        /// Host object that maintains the active result-set view.
        const Impl& host_;

        /// Saved original active view from host (prior to restricting view
        /// to single grid ID).
        const ecl_file_view_type* save_;
    };

    /// Casename prefix.  Mostly to implement copy ctor.
    const Path prefix_;

    /// Active result-set.
    ECLImpl::FilePtr result_;

    /// First keyword in result-set (\c result_).  Needed to identify start
    /// of main grid's section within a view.
    std::string firstKeyword_;

    /// True if path (or pointer) to unified restart file was passed
    /// as constructor argument.
    bool isUnified_;

    /// Map LGR names to integral grid IDs.
    std::unique_ptr<ECLImpl::GridIDCache> gridIDCache_;

    /// Current active result-set view.
    mutable const ecl_file_view_type* activeBlock_{ nullptr };

    /// Support for passing \code *this \endcode to ERT functions that
    /// require an \c ecl_file_type, particularly the function that selects
    /// the file's global view--ecl_file_get_global_view().
    ///
    /// Note mutable return type.  This is okay because the relevant
    /// functions don't modify their inputs.
    operator ecl_file_type*() const;

    /// Support for passing \code *this \endcode to ERT functions that
    /// require an \c ecl_file_view_type.
    operator const ecl_file_view_type*() const;

    /// Retrieve result-set keyword that identifies beginning of main grid's
    /// result vectors.
    const std::string& mainGridStart() const;

    int gridID(const std::string& gridName) const;
};

Opm::ECLRestartData::Impl::Impl(Path prefix)
    : prefix_      (std::move(prefix))
    , result_      (openResultSet(deriveRestartPath(prefix_)))
    , firstKeyword_(firstFileKeyword(result_.get()))
    , isUnified_   (firstKeyword_ == "SEQNUM")
{}

Opm::ECLRestartData::Impl::Impl(std::shared_ptr<ecl_file_type> rstrt)
    : prefix_      (ecl_file_get_src_file(rstrt.get()))
    , result_      (std::move(rstrt))
    , firstKeyword_(firstFileKeyword(result_.get()))
    , isUnified_   (firstKeyword_ == "SEQNUM")
{}

Opm::ECLRestartData::Impl::Impl(const Impl& rhs)
    : prefix_      (rhs.prefix_)
    , result_      (openResultSet(deriveRestartPath(prefix_)))
    , firstKeyword_(firstFileKeyword(result_.get()))
    , isUnified_   (rhs.isUnified_)
{}

Opm::ECLRestartData::Impl::Impl(Impl&& rhs)
    : prefix_      (std::move(rhs.prefix_))
    , result_      (std::move(rhs.result_))
    , firstKeyword_(std::move(rhs.firstKeyword_))
    , isUnified_   (rhs.isUnified_)
{}

bool Opm::ECLRestartData::Impl::selectReportStep(const int step)
{
    if (isUnified_ && ! ecl_file_has_report_step(*this, step)) {
        return false;
    }

    this->gridIDCache_.reset();

    if (auto* globView = ecl_file_get_global_view(*this)) {
        if (isUnified_) {
            // Set active block view to particular report step.
            // Ignore sequence numbers, dates, and simulation time.
            const auto seqnum  = -1;
            const auto dates   = static_cast<std::size_t>(-1);
            const auto simdays = -1.0;

            this->activeBlock_ =
                ecl_file_view_add_restart_view(globView, seqnum,
                                               step, dates, simdays);
        } else {
            // Set active block view to global.
            this->activeBlock_ = globView;
        }

        // Update grid id cache from active view.
        if (this->activeBlock_ != nullptr) {
            this->gridIDCache_
                .reset(new ECLImpl::GridIDCache(this->activeBlock_));

            return true;
        }
    }

    return false;
}

bool Opm::ECLRestartData::Impl::
haveKeywordData(const std::string& vector,
                const std::string& gridName) const
{
    const auto gridID = this->gridIDCache_->getGridID(gridName);

    if (gridID < 0) {
        return false;
    }

    // Note: Non-trivial dtor.  Compiler can't ignore object.
    const auto block = Restrict{ *this, gridID };

    const auto count =
        ecl_file_view_get_num_named_kw(*this, vector.c_str());

    return count > 0;
}

namespace Opm {

    template <typename T>
    std::vector<T>
    ECLRestartData::Impl::keywordData(const std::string& vector,
                                      const std::string& gridName) const
    {
        if (! this->haveKeywordData(vector, gridName)) {
            std::ostringstream os;

            os << "RESTART: Cannot Access Non-Existent Keyword Data Pair ("
               << vector << ", "
               << (gridName.empty() ? "Main Grid" : gridName)
               << ')';

            throw std::invalid_argument(os.str());
        }

        const auto gridID = this->gridIDCache_->getGridID(gridName);

        // Note: Non-trivial dtor.  Compiler can't ignore object.
        const auto block = Restrict{ *this, gridID };

        const auto occurrence = 0;

        const auto* kw =
            ecl_file_view_iget_named_kw(*this, vector.c_str(),
                                        occurrence);

        assert ((kw != nullptr) &&
                "Logic Error In Data Availability Check");

        return ECLImpl::getKeywordData<T>(kw);
    }

} // namespace Opm

Opm::ECLRestartData::Impl::operator ecl_file_type*() const
{
    return this->result_.get();
}

Opm::ECLRestartData::Impl::operator const ecl_file_view_type*() const
{
    return this->activeBlock_;
}

const std::string&
Opm::ECLRestartData::Impl::mainGridStart() const
{
    return this->firstKeyword_;
}

int
Opm::ECLRestartData::Impl::gridID(const std::string& gridName) const
{
    return this->gridIDCache_->getGridID(gridName);
}

// ======================================================================
// Class Opm::ECLInitFileData::Impl
// ======================================================================

class Opm::ECLInitFileData::Impl
{
public:
    using Path = boost::filesystem::path;

    /// Constructor.
    ///
    /// Construct from filename.  Owning semantics.
    ///
    /// \param[in] casePrefix Name or prefix of ECL result data.
    Impl(Path initFile);

    /// Constructor.
    ///
    /// Construct from dataset already input through other means.
    ///
    /// Non-owning semantics.
    Impl(std::shared_ptr<ecl_file_type> initFile);

    /// Copy constructor.
    ///
    /// \param[in] rhs Object from which to construct new \c Impl instance.
    Impl(const Impl& rhs);

    /// Move constructor.
    ///
    /// \param[in,out] rhs Object from which to constructo new \c Impl
    ///    instance.  Underlying result-set accessor is null upon return.
    Impl(Impl&& rhs);

    const ecl_file_type* getRawFilePtr() const;

    /// Query current result-set view for availability of particular named
    /// result vector in particular enumerated grid.
    ///
    /// \param[in] vector Named result vector for which to query data
    ///    availability.
    ///
    /// \param[in] gridID Identity of specific grid for which to query data
    ///    availability.
    ///
    /// \return Whether or not keyword data for the named result vector is
    ///    available in the specific grid.
    bool haveKeywordData(const std::string& vector,
                         const std::string& gridName) const;

    /// Retrieve current result-set view's data values for particular named
    /// result vector in particular enumerated grid.
    ///
    /// Will fail (throw an exception of type std::invalid_argument) unless
    /// the requested keyword data is available in the specific grid in the
    /// current active view.
    ///
    /// \tparam T Element type of return value.  The underlying keyword data
    ///    will be converted to this type if needed and possible.  Note that
    ///    some type combinations do not make sense.  It is, for instance,
    ///    not possible to retrieve keyword values of an underlying
    ///    arithmetic type in the form of a \code std::vector<std::string>
    ///    \endcode.  Similarly, we cannot access underlying character data
    ///    through elements of an arithmetic type (e.g., \code
    ///    std::vector<double> \endcode.)
    ///
    /// \param[in] vector Named result vector for which to retrieve
    ///    keyword data.
    ///
    /// \param[in] gridID Identity of specific grid for which to
    ///    retrieve keyword data.
    ///
    /// \return Keyword data values.  Empty if type conversion fails.
    template <typename T>
    std::vector<T>
    keywordData(const std::string& vector,
                const std::string& gridName) const;

private:
    using SectionID =
        ECLImpl::InitFileSections::SectionID;

    /// Result of searching for a particular pairing of (vector,gridName) in
    /// INIT.
    struct LookupResult
    {
        /// In what INIT file section the KW was located (-1 if not found).
        SectionID sectID;

        /// Local ID, within sectID, of grid section data section that hosts
        /// the 'vector' (-1 if not found).
        int gridSectID;
    };

    /// Pairing of kw vector and grid name.
    struct KWKey {
        /// Keyword/result set vector
        std::string vector;

        /// ID of grid for which to look up 'vector'.
        std::string gridName;
    };

    /// Comparator (std::set<> and std::map<>) for KWKeys.
    struct CompareKWKey {
        bool operator()(const KWKey& k1, const KWKey& k2) const
        {
            return std::tie(k1.vector, k1.gridName)
                <  std::tie(k2.vector, k2.gridName);
        }
    };

    /// Negative look-up cache.
    using MissingKW = std::set<KWKey, CompareKWKey>;

    /// Keyword-to-section cache to accelerate repeated look-up.
    using KWSection = std::map<KWKey, LookupResult, CompareKWKey>;

    /// Casename prefix.  Mostly to implement copy ctor.
    const Path prefix_;

    /// Raw result set.
    ECLImpl::FilePtr initFile_;

    /// Sections of the INIT result set.
    ECLImpl::InitFileSections sections_;

    mutable const ecl_file_view_type* activeBlock_{ nullptr };

    /// Negative look-up cache for haveKeywordData() queries.
    mutable MissingKW missing_kw_;

    /// Keyword-to-section map for successful haveKeywordData() and
    /// keywordData() queries.  Accelerates repeated look-up queries.
    mutable KWSection kw_section_;

    operator const ecl_file_view_type*() const;

    LookupResult
    lookup(const std::string& vector,
           const std::string& gridName) const;

    LookupResult lookupMainGrid(const KWKey& key) const;

    LookupResult lookupLGR(const KWKey& key) const;

    void setActiveBlock(const SectionID sect) const;

    const ECLImpl::InitFileSections::Section&
    getSection(const SectionID sect) const;
};

Opm::ECLInitFileData::Impl::Impl(Path initFile)
    : prefix_  (initFile.stem())
    , initFile_(openResultSet(deriveInitPath(std::move(initFile))))
    , sections_(initFile_.get())
{}

Opm::ECLInitFileData::Impl::Impl(std::shared_ptr<ecl_file_type> initFile)
    : prefix_  (Path(ecl_file_get_src_file(initFile.get())).stem())
    , initFile_(std::move(initFile))
    , sections_(initFile_.get())
{}

Opm::ECLInitFileData::Impl::Impl(const Impl& rhs)
    : prefix_  (rhs.prefix_)
    , initFile_(rhs.initFile_)
    , sections_(initFile_.get())
{}

Opm::ECLInitFileData::Impl::Impl(Impl&& rhs)
    : prefix_  (std::move(rhs.prefix_))
    , initFile_(std::move(rhs.initFile_))
    , sections_(std::move(rhs.sections_))
{}

const ecl_file_type*
Opm::ECLInitFileData::Impl::getRawFilePtr() const
{
    return this->initFile_.get();
}

bool
Opm::ECLInitFileData::Impl::
haveKeywordData(const std::string& vector,
                const std::string& gridName) const
{
    const auto kwloc = this->lookup(vector, gridName);

    return (kwloc.sectID < this->sections_.numSections())
        && (kwloc.gridSectID >= 0);
}

namespace Opm {

    template <typename T>
    std::vector<T>
    ECLInitFileData::Impl::
    keywordData(const std::string& vector,
                const std::string& gridName) const
    {
        if (! this->haveKeywordData(vector, gridName)) {
            std::ostringstream os;

            os << "INIT: Cannot Access Non-Existent Keyword Data Pair ("
               << vector << ", "
               << (gridName.empty() ? "Main Grid" : gridName)
               << ')';

            throw std::invalid_argument(os.str());
        };

        const auto kwloc = this->lookup(vector, gridName);

        this->setActiveBlock(kwloc.sectID);

        if (! gridName.empty()) {
            // We're cons
            this->activeBlock_ =
                ecl_file_view_add_blockview(this->activeBlock_, LGR_KW,
                                            kwloc.gridSectID);
        }

        const auto occurrence = 0;

        const auto* kw =
            ecl_file_view_iget_named_kw(*this, vector.c_str(),
                                        occurrence);

        assert ((kw != nullptr) &&
                "Logic Error In Data Availability Check");

        return ECLImpl::getKeywordData<T>(kw);
    }

}

Opm::ECLInitFileData::Impl::operator const ecl_file_view_type*() const
{
    return this->activeBlock_;
}

Opm::ECLInitFileData::Impl::LookupResult
Opm::ECLInitFileData::Impl::
lookup(const std::string& vector, const std::string& gridName) const
{
    const auto key = KWKey{ vector, gridName };

    {
        auto m = this->missing_kw_.find(key);

        if (m != std::end(this->missing_kw_)) {
            // 'vector' known to be mssing for 'gridName'.  Report as such.
            return { SectionID(-1), -1 };
        }
    }

    {
        auto i = this->kw_section_.find(key);

        if (i != std::end(this->kw_section_)) {
            // 'vector' previously located for 'gridName'.  Return it.
            return i->second;
        }
    }

    if (gridName.empty()) {
        return this->lookupMainGrid(key);
    }

    return this->lookupLGR(key);
}

Opm::ECLInitFileData::Impl::LookupResult
Opm::ECLInitFileData::Impl::lookupMainGrid(const KWKey& key) const
{
    assert (key.gridName.empty() && "Logic Error");

    const auto* kwheader = key.vector.c_str();

    // Main grid sections are even numbered.
    for (auto nSect = this->sections_.numSections(), sectID = 0*nSect;
         sectID < nSect; sectID += 2)
    {
        const auto& s = this->getSection(sectID);

        // Skip empty sections
        if (s.block == nullptr) { continue; }

        const auto count =
            ecl_file_view_get_num_named_kw(s.block, kwheader);

        if (count > 0) {
            // Result-set 'vector' present in main grid.  Record and return.

            // Assume that keyword does not occur multiple times in main
            // grid section.
            const auto gridSectID = 0;

            return this->kw_section_[key] =
                LookupResult { sectID, gridSectID };
        }
    }

    // No result-set 'vector' in main grid.  Record as missing and return
    // "not found".
    this->missing_kw_.insert(key);

    return { SectionID(-1), -1 };
}

Opm::ECLInitFileData::Impl::LookupResult
Opm::ECLInitFileData::Impl::lookupLGR(const KWKey& key) const
{
    assert ((! key.gridName.empty()) && "Logic Error");

    const auto  gridID   = paddedGridName(key.gridName);
    const auto* kwheader = key.vector.c_str();
    const auto* gridName = gridID.c_str();

    // LGR sections are odd numbered.
    for (auto nSect = this->sections_.numSections(), sectID = 0*nSect + 1;
         sectID < nSect; sectID += 2)
    {
        const auto& s = this->getSection(sectID);

        // Skip empty sections.
        if (s.block == nullptr) { continue; }

        const auto nLGR =
            ecl_file_view_get_num_named_kw(s.block, LGR_KW);

        assert ((nLGR > 0) && "Logic Error");

        for (auto lgrID = 0*nLGR; lgrID < nLGR; ++lgrID) {
            const auto* kw =
                ecl_file_view_iget_named_kw(s.block, LGR_KW, lgrID);

            if (! ecl_kw_data_equal(kw, gridName)) {
                // This is not the grid you're looking for.
                continue;
            }

            // This LGR section pertains to key.gridName.  Look for
            // key.vector between this occurrence of LGR_KW and the next.
            // Continue searching if we don't find that vector however,
            // because there may be multiple relevant LGR instances for
            // key.gridName.

            const auto* lgrSect =
                ecl_file_view_add_blockview(s.block, LGR_KW, lgrID);

            const auto kwCount =
                ecl_file_view_get_num_named_kw(lgrSect, kwheader);

            if (kwCount > 0) {
                // We found the key.vector in this LGR data section.  Record
                // position and return it to caller.

                return this->kw_section_[key] =
                    LookupResult { SectionID(sectID), lgrID };
            }

            // Did not find 'key.vector' in this LGR data section.  Continue
            // searching nonetheless because there may be more LGR sections
            // that pertain to 'key.gridName'.
        }
    }

    // key.vector not found for key.gridName in any of the LGR sections.
    // Record as missing and return "not found".

    this->missing_kw_.insert(key);

    return { SectionID(-1), -1 };
}

void
Opm::ECLInitFileData::Impl::setActiveBlock(const SectionID sect) const
{
    this->activeBlock_ = this->getSection(sect).block;
}

const ECLImpl::InitFileSections::Section&
Opm::ECLInitFileData::Impl::getSection(const SectionID sect) const
{
    assert (sect < this->sections_.numSections());

    return this->sections_[sect];
}

// ######################################################################
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// Public Interfaces Follow
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ######################################################################

// ======================================================================
// Implementation of class Opm::ECLRestartData Below Separator
// ======================================================================

Opm::ECLRestartData::ECLRestartData(boost::filesystem::path rstrt)
    : pImpl_(new Impl(std::move(rstrt)))
{}

Opm::ECLRestartData::ECLRestartData(std::shared_ptr<ecl_file_type> rstrt)
    : pImpl_(new Impl(std::move(rstrt)))
{}

Opm::ECLRestartData::ECLRestartData(const ECLRestartData& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLRestartData::ECLRestartData(ECLRestartData&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLRestartData&
Opm::ECLRestartData::operator=(const ECLRestartData& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::ECLRestartData&
Opm::ECLRestartData::operator=(ECLRestartData&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

Opm::ECLRestartData::~ECLRestartData()
{}

bool Opm::ECLRestartData::selectReportStep(const int step) const
{
    // selectReportStep() const is a bit of a lie.  pImpl_ is a const
    // pointer to modifiable Impl.

    return this->pImpl_->selectReportStep(step);
}

bool
Opm::ECLRestartData::
haveKeywordData(const std::string& vector,
                const std::string& gridID) const
{
    return this->pImpl_->haveKeywordData(vector, gridID);
}

namespace Opm {

    template <typename T>
    std::vector<T>
    ECLRestartData::keywordData(const std::string& vector,
                                const std::string& gridID) const
    {
        return this->pImpl_->template keywordData<T>(vector, gridID);
    }

    // Explicit instantiations for those types we care about.
    template std::vector<std::string>
    ECLRestartData::keywordData<std::string>(const std::string& vector,
                                             const std::string& gridID) const;

    template std::vector<bool>
    ECLRestartData::keywordData<bool>(const std::string& vector,
                                      const std::string& gridID) const;

    template std::vector<int>
    ECLRestartData::keywordData<int>(const std::string& vector,
                                     const std::string& gridID) const;

    template std::vector<double>
    ECLRestartData::keywordData<double>(const std::string& vector,
                                        const std::string& gridID) const;

} // namespace Opm::ECL

// ======================================================================
// Implementation of class Opm::ECLInitFileData Below Separator
// ======================================================================

Opm::ECLInitFileData::ECLInitFileData(boost::filesystem::path init)
    : pImpl_(new Impl(std::move(init)))
{}

Opm::ECLInitFileData::ECLInitFileData(std::shared_ptr<ecl_file_type> init)
    : pImpl_(new Impl(std::move(init)))
{}

Opm::ECLInitFileData::ECLInitFileData(const ECLInitFileData& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLInitFileData::ECLInitFileData(ECLInitFileData&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLInitFileData&
Opm::ECLInitFileData::operator=(const ECLInitFileData& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::ECLInitFileData&
Opm::ECLInitFileData::operator=(ECLInitFileData&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

Opm::ECLInitFileData::~ECLInitFileData()
{}

bool
Opm::ECLInitFileData::
haveKeywordData(const std::string& vector,
                const std::string& gridID) const
{
    return this->pImpl_->haveKeywordData(vector, gridID);
}

const ecl_file_type*
Opm::ECLInitFileData::getRawFilePtr() const
{
    return this->pImpl_->getRawFilePtr();
}

namespace Opm {

    template <typename T>
    std::vector<T>
    ECLInitFileData::keywordData(const std::string& vector,
                                 const std::string& gridID) const
    {
        return this->pImpl_->template keywordData<T>(vector, gridID);
    }

    // Explicit instantiations for those types we care about.
    template std::vector<std::string>
    ECLInitFileData::keywordData<std::string>(const std::string& vector,
                                              const std::string& gridID) const;

    template std::vector<bool>
    ECLInitFileData::keywordData<bool>(const std::string& vector,
                                       const std::string& gridID) const;

    template std::vector<int>
    ECLInitFileData::keywordData<int>(const std::string& vector,
                                      const std::string& gridID) const;

    template std::vector<double>
    ECLInitFileData::keywordData<double>(const std::string& vector,
                                         const std::string& gridID) const;

} // namespace Opm::ECL

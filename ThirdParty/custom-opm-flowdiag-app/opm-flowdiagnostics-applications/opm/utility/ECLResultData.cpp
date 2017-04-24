/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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
#include <sstream>
#include <stdexcept>
#include <type_traits>
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
        using FilePtr = ::ERT::ert_unique_ptr<ecl_file_type, ecl_file_close>;

        namespace Details {
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
                        result.emplace_back(xi);
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
                }

                return result;
            }
        };
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
    /// \return Filesystem element corresponding to result-set.  Either the
    ///    input \p file itself or, in the case of a casename prefix, the
    ///    path to a restart file (unified format only).
    boost::filesystem::path
    deriveResultPath(boost::filesystem::path file)
    {
        if (isFile(file)) {
            return file;
        }

        for (const auto* ext : { ".UNRST", ".FUNRST" }) {
            file.replace_extension(ext);

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

    /// Load result-set represented by filesystem element.
    ///
    /// Fails (throws an exception of type \code std::invalid_argument
    /// \endcode) if the input filesystem element does not represent a valid
    /// result-set resource or if the resource could not be opened (e.g.,
    /// due to lack of permission).
    ///
    /// \param[in] rset Fileystem element representing a result-set.
    ///
    /// \return Accessor handle of result-set.
    ECLImpl::FilePtr openResultSet(const boost::filesystem::path& rset)
    {
        // Read-only, keep open between requests
        const auto open_flags = 0;

        auto F = ECLImpl::FilePtr{
            ecl_file_open(rset.generic_string().c_str(), open_flags)
        };

        if (! F) {
            std::ostringstream os;

            os << "Failed to load ECL Result object from '"
               << rset.generic_string() << '\'';

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

        return ecl_kw_get_header(ecl_file_view_iget_kw(globView, 0));
    }
} // namespace Anonymous

/// Engine powering implementation of \c ECLResultData interface
class Opm::ECLResultData::Impl
{
public:
    /// Constructor
    ///
    /// \param[in] rset Filesystem element or casename prefix representing
    ///    an ECL result-set.
    Impl(Path rset);

    /// Copy constructor.
    ///
    /// \param[in] rhs Object from which to construct new \c Impl instance.
    Impl(const Impl& rhs);

    /// Move constructor.
    ///
    /// \param[in,out] rhs Object from which to constructo new \c Impl
    ///    instance.  Underlying result-set accessor is null upon return.
    Impl(Impl&& rhs);

    /// Access the underlying ERT representation of the result-set.
    ///
    /// This is a hole in the interface that exists to be able to access
    /// ERT's internal data structures for non-neighbouring connections
    /// (i.e., faults and/or connections between main grid and LGRs).  See
    /// function ecl_nnc_export() in the ERT.
    ///
    /// Handle with care.
    ///
    /// \return Handle to underlying ERT representation of result-set.
    const ecl_file_type* getRawFilePtr() const;

    /// Reset the object's internal view of the result-set to encompass the
    /// entirety of the underlying file's result vectors.
    ///
    /// This is mostly useful in the case of accessing static result-sets
    /// such as INIT files.
    ///
    /// \return Whether or not generating the global view succeeded.
    bool selectGlobalView();

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
                         const int          gridID) const;

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
                const int          gridID) const;

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
        ecl_file_view_type* save_;
    };

    /// Casename prefix.  Mostly to implement copy ctor.
    const Path prefix_;

    /// Active result-set.
    ECLImpl::FilePtr result_;

    /// First keyword in result-set (\c result_).  Needed to identify start
    /// of main grid's section within a view.
    std::string firstKeyword_;

    /// Current active result-set view.
    mutable ecl_file_view_type* activeBlock_{ nullptr };

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
};

Opm::ECLResultData::Impl::Impl(Path prefix)
    : prefix_      (std::move(prefix))
    , result_      (openResultSet(deriveResultPath(prefix_)))
    , firstKeyword_(firstFileKeyword(result_.get()))
{}

Opm::ECLResultData::Impl::Impl(const Impl& rhs)
    : prefix_      (rhs.prefix_)
    , result_      (openResultSet(deriveResultPath(prefix_)))
    , firstKeyword_(firstFileKeyword(result_.get()))
{}

Opm::ECLResultData::Impl::Impl(Impl&& rhs)
    : prefix_      (std::move(rhs.prefix_))
    , result_      (std::move(rhs.result_))
    , firstKeyword_(std::move(rhs.firstKeyword_))
{}

const ecl_file_type*
Opm::ECLResultData::Impl::getRawFilePtr() const
{
    return this->result_.get();
}

bool Opm::ECLResultData::Impl::selectGlobalView()
{
    this->activeBlock_ = ecl_file_get_global_view(*this);

    return this->activeBlock_ != nullptr;
}

bool Opm::ECLResultData::Impl::selectReportStep(const int step)
{
    if (! ecl_file_has_report_step(*this, step)) {
        return false;
    }

    if (auto* globView = ecl_file_get_global_view(*this)) {
        // Ignore sequence numbers, dates, and simulation time.
        const auto seqnum  = -1;
        const auto dates   = static_cast<std::size_t>(-1);
        const auto simdays = -1.0;

        this->activeBlock_ =
            ecl_file_view_add_restart_view(globView, seqnum,
                                           step, dates, simdays);

        return this->activeBlock_ != nullptr;
    }

    return false;
}

bool Opm::ECLResultData::Impl::
haveKeywordData(const std::string& vector, const int gridID) const
{
    assert ((gridID >= 0) && "Grid IDs must be non-negative");

    // Note: Non-trivial dtor.  Compiler can't ignore object.
    const auto block = Restrict{ *this, gridID };

    const auto count =
        ecl_file_view_get_num_named_kw(*this, vector.c_str());

    return count > 0;
}

namespace Opm {

    template <typename T>
    std::vector<T>
    ECLResultData::Impl::keywordData(const std::string& vector,
                                     const int          gridID) const
    {
        if (! this->haveKeywordData(vector, gridID)) {
            std::ostringstream os;

            os << "Cannot Access Non-Existent Keyword Data Pair ("
               << vector << ", " << gridID << ')';

            throw std::invalid_argument(os.str());
        }

        // Note: Non-trivial dtor.  Compiler can't ignore object.
        const auto block = Restrict{ *this, gridID };

        const auto occurrence = 0;

        const auto* kw =
            ecl_file_view_iget_named_kw(*this, vector.c_str(),
                                        occurrence);

        assert ((kw != nullptr) &&
                "Logic Error In Data Availability Check");

        // Whether or not caller requests a vector<string>.
        const auto makeStringVector =
            typename std::is_same<T, std::string>::type{};

        switch (getKeywordElementType(kw)) {
        case ECL_CHAR_TYPE:
            return ECLImpl::GetKeywordData<ECL_CHAR_TYPE>::
                as<T>(kw, makeStringVector);

        case ECL_INT_TYPE:
            return ECLImpl::GetKeywordData<ECL_INT_TYPE>::
                as<T>(kw, makeStringVector);

        case ECL_FLOAT_TYPE:
            return ECLImpl::GetKeywordData<ECL_FLOAT_TYPE>::
                as<T>(kw, makeStringVector);

        case ECL_DOUBLE_TYPE:
            return ECLImpl::GetKeywordData<ECL_DOUBLE_TYPE>::
                as<T>(kw, makeStringVector);

        default:
            // No operator exists for this type.  Return empty.
            return {};
        }
    }

} // namespace Opm

Opm::ECLResultData::Impl::operator ecl_file_type*() const
{
    return this->result_.get();
}

Opm::ECLResultData::Impl::operator const ecl_file_view_type*() const
{
    return this->activeBlock_;
}

const std::string&
Opm::ECLResultData::Impl::mainGridStart() const
{
    return this->firstKeyword_;
}

// ======================================================================
// Implementation of class Opm::ECLResultData Below Separator
// ======================================================================

Opm::ECLResultData::ECLResultData(const Path& prefix)
    : pImpl_(new Impl(prefix))
{}

Opm::ECLResultData::ECLResultData(const ECLResultData& rhs)
    : pImpl_(new Impl(*rhs.pImpl_))
{}

Opm::ECLResultData::ECLResultData(ECLResultData&& rhs)
    : pImpl_(std::move(rhs.pImpl_))
{}

Opm::ECLResultData&
Opm::ECLResultData::operator=(const ECLResultData& rhs)
{
    this->pImpl_.reset(new Impl(*rhs.pImpl_));

    return *this;
}

Opm::ECLResultData&
Opm::ECLResultData::operator=(ECLResultData&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);

    return *this;
}

Opm::ECLResultData::~ECLResultData()
{}

const ecl_file_type*
Opm::ECLResultData::getRawFilePtr() const
{
    return this->pImpl_->getRawFilePtr();
}

bool Opm::ECLResultData::selectGlobalView()
{
    return this->pImpl_->selectGlobalView();
}

bool Opm::ECLResultData::selectReportStep(const int step)
{
    return this->pImpl_->selectReportStep(step);
}

bool
Opm::ECLResultData::
haveKeywordData(const std::string& vector, const int gridID) const
{
    return this->pImpl_->haveKeywordData(vector, gridID);
}

namespace Opm {

    template <typename T>
    std::vector<T>
    ECLResultData::keywordData(const std::string& vector,
                               const int          gridID) const
    {
        return this->pImpl_->template keywordData<T>(vector, gridID);
    }

    // Explicit instantiations for those types we care about.
    template std::vector<std::string>
    ECLResultData::keywordData<std::string>(const std::string& vector,
                                            const int          gridID) const;

    template std::vector<int>
    ECLResultData::keywordData<int>(const std::string& vector,
                                    const int          gridID) const;

    template std::vector<double>
    ECLResultData::keywordData<double>(const std::string& vector,
                                       const int          gridID) const;

} // namespace Opm::ECL

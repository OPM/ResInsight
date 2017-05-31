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

#ifndef OPM_ECLRESULTDATA_HEADER_INCLUDED
#define OPM_ECLRESULTDATA_HEADER_INCLUDED

#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

/// \file
///
/// Interface to direct result-set data vector operations.

/// Forward-declaration of ERT's representation of an ECLIPSE result file.
///
/// This is a hole in the insulation between the interface and the
/// underlying implementation of class ECLInitFileData and furthermore
/// enables constructing wrapper objects from separately parsed result sets.
extern "C" {
    typedef struct ecl_file_struct ecl_file_type;
} // extern "C"

namespace Opm {

    class ECLGraph;

    /// Representation of an ECLIPSE Restart result-set.
    ///
    /// This class is aware of the internal structure of ECLIPSE restart
    /// files and may restrict its operation to a single report step.  The
    /// class furthermore knows about sub-blocks corresponding to main or
    /// local grids within a report step and queries only those objects that
    /// pertain to a single grid at a time.
    ///
    /// Note: The client must select a view of the result-set before
    /// accessing any vectors within the set.
    class ECLRestartData
    {
    public:
        /// Default constructor disabled.
        ECLRestartData() = delete;

        /// Constructor.
        ///
        /// Owning semantics.
        ///
        /// \param[in] rstrt Name or prefix of ECL result data.
        explicit ECLRestartData(boost::filesystem::path rstrt);

        /// Constructor
        ///
        /// Shared ownership of result set.
        ///
        /// \param[in] rstrt ECL restart result set.
        explicit ECLRestartData(std::shared_ptr<ecl_file_type> rstrt);

        /// Copy constructor.
        ///
        /// \param[in] rhs Object from which to construct new instance.
        ECLRestartData(const ECLRestartData& rhs);

        /// Move constructor.
        ///
        /// \param[in,out] rhs Object from which to construct new instance.
        ///    Its internal implementation will be subsumed into the new
        ///    object.
        ECLRestartData(ECLRestartData&& rhs);

        /// Assignment operator.
        ///
        /// \param[in] rhs Object from which to assign new values to current
        ///    instance.
        ///
        /// \return \code *this \endcode.
        ECLRestartData& operator=(const ECLRestartData& rhs);

        /// Move assignment operator.
        ///
        /// \param[in,out] Object from which to assign new instance values.
        ///    Its internal implementation will be subsumed into this
        ///    instance.
        ///
        /// \return \code *this \endcode.
        ECLRestartData& operator=(ECLRestartData&& rhs);

        /// Destructor.
        ~ECLRestartData();

        /// Select a result-set view that corresponds to a single report
        /// step.
        ///
        /// This is needed when working with dynamic restart data.
        ///
        /// \param[in] step Report step number.
        ///
        /// \return Whether or not selecting the report step succeeded.  The
        ///    typical failure case is the report step not being available
        ///    in the result-set.
        bool selectReportStep(const int step) const;

        /// Query current result-set view for availability of particular
        /// named result vector in particular enumerated grid.
        ///
        /// \param[in] vector Named result vector for which to query data
        ///    availability.
        ///
        /// \param[in] gridID Identity of specific grid for which to query
        ///    data availability.  Empty for the main grid.
        ///
        /// \return Whether or not keyword data for the named result vector
        ///    is available in the specific grid.
        bool haveKeywordData(const std::string& vector,
                             const std::string& gridID = "") const;

        /// Retrieve current result-set view's data values for particular
        /// named result vector in particular enumerated grid.
        ///
        /// Will fail (throw an exception of type std::invalid_argument)
        /// unless the requested keyword data is available in the specific
        /// grid in the current active view.
        ///
        /// \tparam T Element type of return value.  The underlying keyword
        ///    data will be converted to this type if needed and possible.
        ///    Note that some type combinations do not make sense.  It is,
        ///    for instance, not possible to retrieve keyword values of an
        ///    underlying arithmetic type in the form of a \code
        ///    std::vector<std::string> \endcode.  Similarly, we cannot
        ///    access underlying character data through elements of an
        ///    arithmetic type (e.g., \code std::vector<double> \endcode.)
        ///
        /// \param[in] vector Named result vector for which to retrieve
        ///    keyword data.
        ///
        /// \param[in] gridID Identity of specific grid for which to
        ///    retrieve keyword data.  Empty for the main grid.
        ///
        /// \return Keyword data values.  Empty if type conversion fails.
        template <typename T>
        std::vector<T>
        keywordData(const std::string& vector,
                    const std::string& gridID = "") const;

    private:
        class Impl;

        std::unique_ptr<Impl> pImpl_;
    };

    /// Representation of an ECLIPSE Initialization result-set.
    ///
    /// This class is aware of the internal structure of ECLIPSE INIT files
    /// and queries only those objects that pertain to a single grid.
    class ECLInitFileData
    {
    public:
        ECLInitFileData() = delete;

        /// Constructor.
        ///
        /// Construct from filename.  Owning semantics.
        ///
        /// \param[in] casePrefix Name or prefix of ECL result data.
        explicit ECLInitFileData(boost::filesystem::path initFile);

        /// Constructor.
        ///
        /// Construct from dataset already input through other means.
        ///
        /// Non-owning/shared ownership semantics.
        explicit ECLInitFileData(std::shared_ptr<ecl_file_type> initFile);

        /// Copy constructor.
        ///
        /// \param[in] rhs Object from which to construct new instance.
        ECLInitFileData(const ECLInitFileData& rhs);

        /// Move constructor.
        ///
        /// \param[in,out] rhs Object from which to construct new instance.
        ///    Its internal implementation will be subsumed into the new
        ///    object.
        ECLInitFileData(ECLInitFileData&& rhs);

        /// Assignment operator.
        ///
        /// \param[in] rhs Object from which to assign new values to current
        ///    instance.
        ///
        /// \return \code *this \endcode.
        ECLInitFileData& operator=(const ECLInitFileData& rhs);

        /// Move assignment operator.
        ///
        /// \param[in,out] Object from which to assign new instance values.
        ///    Its internal implementation will be subsumed into this
        ///    instance.
        ///
        /// \return \code *this \endcode.
        ECLInitFileData& operator=(ECLInitFileData&& rhs);

        /// Destructor.
        ~ECLInitFileData();

        /// Query current result-set view for availability of particular
        /// named result vector in particular enumerated grid.
        ///
        /// \param[in] vector Named result vector for which to query data
        ///    availability.
        ///
        /// \param[in] gridID Identity of specific grid for which to query
        ///    data availability.  Empty for the main grid.
        ///
        /// \return Whether or not keyword data for the named result vector
        ///    is available in the specific grid.
        bool haveKeywordData(const std::string& vector,
                             const std::string& gridID = "") const;

        /// Retrieve current result-set view's data values for particular
        /// named result vector in particular enumerated grid.
        ///
        /// Will fail (throw an exception of type std::invalid_argument)
        /// unless the requested keyword data is available in the specific
        /// grid in the current active view.
        ///
        /// \tparam T Element type of return value.  The underlying keyword
        ///    data will be converted to this type if needed and possible.
        ///    Note that some type combinations do not make sense.  It is,
        ///    for instance, not possible to retrieve keyword values of an
        ///    underlying arithmetic type in the form of a \code
        ///    std::vector<std::string> \endcode.  Similarly, we cannot
        ///    access underlying character data through elements of an
        ///    arithmetic type (e.g., \code std::vector<double> \endcode.)
        ///
        /// \param[in] vector Named result vector for which to retrieve
        ///    keyword data.
        ///
        /// \param[in] gridID Identity of specific grid for which to
        ///    retrieve keyword data.  Empty for the main grid.
        ///
        /// \return Keyword data values.  Empty if type conversion fails.
        template <typename T>
        std::vector<T>
        keywordData(const std::string& vector,
                    const std::string& gridID = "") const;

        // Grant class ECLGraph privileged access to getRawFilePtr().
        friend class ECLGraph;

    private:
        class Impl;

        std::unique_ptr<Impl> pImpl_;

        /// Access the underlying ERT representation of the result-set.
        ///
        /// This is essentially a hole in the interface that is intended to
        /// support a few very specialised uses.  Handle with care.
        ///
        /// \return Handle to underlying ERT representation of result-set.
        const ecl_file_type* getRawFilePtr() const;
    };
} // namespace Opm

#endif  // OPM_ECLRESULTDATA_HEADER_INCLUDED

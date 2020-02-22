/*
  Copyright 2017 Statoil ASA.

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

#include <opm/utility/ECLCaseUtilities.hpp>

#include <exception>
#include <initializer_list>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_file_kw.h>
#include <ert/ecl/ecl_file_view.h>
#include <ert/util/ert_unique_ptr.hpp>

/// Function object that matches strings with a common prefix (i.e., a file
/// name base) followed by .F or .X and then exactly four digits and nothing
/// further.
///
/// In other words, if prefix is "NORNE_ATW2013", then the function will
/// return true for strings of the form
///
///   -* NORNE_ATW2013.F0000
///   -* NORNE_ATW2013.X1234
///
/// but it will return false for anything else like strings of the form
///
///   -* NORNE_ATW2013.S0123
///   -* NORNE_ATW2013.FUNRST
///   -* NORNE_ATW2013.X12345
///   -* NORNE_ATW2013.F012
///   -* NORNE_ATW2013.X012Y
///   -* NORNE_ATW2013.X0123.4
///   -* NORNE_ATW2014.F0001
///
class IsSeparateRestart
{
public:
    /// Constructor.
    ///
    /// \param[in] prefix Common string prefix (file name base).
    IsSeparateRestart(const std::string& prefix)
        : restart_(prefix + R"~~(\.(F|X)\d{4}$)~~")
    {
        // Common prefix + '.' + (F or X) + four digits + "end of string".
    }

    /// Match string against stored regular expression engine.
    ///
    /// \param[in] e Name, possibly full path, of file system element.
    ///    Typically names a regular file.
    ///
    /// \return True if the element matches the entire stored regular
    ///    expression (\code regex_match() \endcode rather than \code
    ///    regex_search() \endcode), false otherwise.
    bool operator()(const boost::filesystem::path& e) const
    {
        return std::regex_match(e.filename().generic_string(),
                                this->restart_);
    }

private:
    /// Regular expression against which to match filesystem elements.
    std::regex restart_;
};

namespace {
    template <class String>
    boost::filesystem::path
    operator+(boost::filesystem::path p, const String& e)
    {
        return p += e;
    }

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

    boost::filesystem::path
    deriveFileName(boost::filesystem::path            file,
                   std::initializer_list<const char*> ext)
    {
        for (const auto& e : ext) {
            file.replace_extension(e);

            if (isFile(file)) {
                return file;
            }
        }

        return "";
    }

    // Handle the possibility that a case prefix ends in a substring that
    // looks like a file extension (e.g., "path/to/CASE.01").  This
    // substring would be stripped off during deriveFileName() processing.
    boost::filesystem::path
    safeCasePrefix(const boost::filesystem::path& casename)
    {
        auto casefile = boost::filesystem::path{};

        for (const auto* ignore : { "", ".@@@-HACK-@@@" }) {
            casefile = deriveFileName(casename + ignore,
                                      { ".DATA" ,
                                        ".EGRID", ".FEGRID" ,
                                        ".GRID" , ".FGRID"  });

            if (! casefile.empty()) {
                // Valid case file found.
                break;
            }
        }

        if (casefile.empty()) {
            return casefile;
        }

        return casefile.parent_path() / (casefile.stem() + ".Imp-Detail-Hack");
    }

    // Look for separate restart files (.X000n or .F000n) that match the
    // case-name prefix in the directory implied by prefix.  Pick the most
    // recent one (as defined by the element's write/modification time).
    //
    // Return empty if there are no separate restart files (that match the
    // prefix).
    boost::filesystem::path
    mostRecentSeparateRestart(const boost::filesystem::path& prefix)
    {
        namespace fs = boost::filesystem;

        const auto isSepRstrt = IsSeparateRestart {
            prefix.stem().generic_string()
        };

        auto most_recent = fs::path{};
        const fs::path parent_path = prefix.parent_path() == "" ? "." : prefix.parent_path();
        auto max_mtime   = 0 * fs::last_write_time(parent_path);

        for (auto i = fs::directory_iterator(parent_path),
                  e = fs::directory_iterator();
             i != e; ++i)
        {
            if (! fs::is_regular_file(i->status())) {
                // Not a file.  Ignore.
                continue;
            }

            const auto& elem = i->path();

            if (! isSepRstrt(elem)) {
                // Not a (separate) restart file.  Ignore.
                continue;
            }

            const auto mtime = fs::last_write_time(elem);

            if (mtime > max_mtime) {
                max_mtime   = mtime;
                most_recent = elem;
            }
        }

        return most_recent;
    }

    // Determine whether or not result set uses a unified restart file.
    //
    // Steps:
    //   1) Return false if no unified restart file matches prefix.
    //
    //   2) Otherwise, return true if no *separate* restart file matches
    //      prefix.
    //
    //   3) Otherwise, return true if unified restart file is more recent
    //      (in terms of modification/write time) than most recent separate
    //      restart file that matches prefix.
    bool restartIsUnified(const boost::filesystem::path& prefix)
    {
        const auto unif = deriveFileName(prefix, { ".UNRST", ".FUNRST" });

        if (unif.empty()) {
            // No unified restart file matches the 'prefix'.  Definitely not
            // a unified restart case.
            return false;
        }

        // There *is* a unified restart file, but there might be separate
        // restart files too--e.g., if the .DATA file was modified between
        // runs.  Look for those, and pick the one with the most recent
        // write time (i.e., stat::m_time on POSIX).
        const auto separate = mostRecentSeparateRestart(prefix);

        if (separate.empty()) {
            // There are no separate restart files that match the 'prefix'.
            // This is definitely a unified restart case.
            return true;
        }

        // There are *both* unified and separte candidate restart files.
        // Choose the unified file if more recent than most recent separate
        // file.
        using boost::filesystem::last_write_time;

        return ! (last_write_time(unif) < last_write_time(separate));
    }

    boost::filesystem::path
    separateRestartFile(const boost::filesystem::path& prefix,
                        const int                      reportStepID)
    {
        auto makeExt = [reportStepID](const std::string& cat) -> std::string
        {
            std::ostringstream os;

            os << cat << std::setw(4) << std::setfill('0') << reportStepID;

            return os.str();
        };

        // Formatted
        const auto F = makeExt("F");

        // Unformatted
        const auto X = makeExt("X");

        // Note: .c_str() is a bit of a hack--needed to match
        // initializer_list<const char*>.
        return deriveFileName(prefix, { X.c_str(), F.c_str() });
    }
}

Opm::ECLCaseUtilities::ResultSet::ResultSet(const Path& casename)
    : prefix_(safeCasePrefix(casename))
{
    if (this->prefix_.empty()) {
        throw std::invalid_argument {
            casename.generic_string() +
            " Is Not a Valid ECL Result Set Name"
        };
    }

    // Don't check for unified/separate until we've verified that this is
    // even a valid result set.
    this->isUnified_ = restartIsUnified(this->prefix_);
}

Opm::ECLCaseUtilities::ResultSet::Path
Opm::ECLCaseUtilities::ResultSet::gridFile() const
{
    return deriveFileName(this->prefix_,
                          { ".EGRID", ".FEGRID",
                            ".GRID" , ".FGRID" });
}

Opm::ECLCaseUtilities::ResultSet::Path
Opm::ECLCaseUtilities::ResultSet::initFile() const
{
    return deriveFileName(this->prefix_,
                          { ".INIT", ".FINIT" });
}

Opm::ECLCaseUtilities::ResultSet::Path
Opm::ECLCaseUtilities::ResultSet::restartFile(const int reportStepID) const
{
    if (this->isUnifiedRestart()) {
        return deriveFileName(this->prefix_, { ".UNRST", ".FUNRST" });
    }

    return separateRestartFile(this->prefix_, reportStepID);
}

bool
Opm::ECLCaseUtilities::ResultSet::isUnifiedRestart() const
{
    return this->isUnified_;
}

std::vector<int>
Opm::ECLCaseUtilities::ResultSet::reportStepIDs() const
{
    using FilePtr = ::ERT::
        ert_unique_ptr<ecl_file_type, ecl_file_close>;

    const auto rsspec_fn =
        deriveFileName(this->prefix_, { ".RSSPEC", ".FRSSPEC" });

    // Read-only, keep open between requests
    const auto open_flags = 0;

    auto rsspec = FilePtr{
        ecl_file_open(rsspec_fn.generic_string().c_str(), open_flags)
    };

    // If spec file was not found, return empty vector.
    if (!rsspec) {
        return {};
    }

    auto* globView = ecl_file_get_global_view(rsspec.get());

    const auto* ITIME_kw = "ITIME";
    const auto n = ecl_file_view_get_num_named_kw(globView, ITIME_kw);

    auto steps = std::vector<int>(n);

    for (auto i = 0*n; i < n; ++i) {
        const auto* itime =
            ecl_file_view_iget_named_kw(globView, ITIME_kw, i);

        const auto* itime_data =
            static_cast<const int*>(ecl_kw_iget_ptr(itime, 0));

        steps[i] = itime_data[0];
    }

    return steps;
}

/*
  Copyright (c) 2019 Equinor ASA

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

#include <opm/io/eclipse/OutputStream.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/ERst.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
    namespace FileExtension
    {
        std::string separate(const int   rptStep,
                             const bool  formatted,
                             const char* fmt_prefix,
                             const char* unfmt_prefix)
        {
            std::ostringstream ext;

            const int cycle = 10 * 1000;
            const int p_ix  = rptStep / cycle;
            const int n     = rptStep % cycle;

            ext << (formatted ? fmt_prefix[p_ix] : unfmt_prefix[p_ix])
                << std::setw(4) << std::setfill('0') << n;

            return ext.str();
        }

        std::string init(const bool formatted)
        {
            return formatted ? "FINIT" : "INIT";
        }

        std::string restart(const int  rptStep,
                            const bool formatted,
                            const bool unified)
        {
            if (unified) {
                return formatted ? "FUNRST" : "UNRST";
            }

            return separate(rptStep, formatted, "FGH", "XYZ");
        }

        std::string rft(const bool formatted)
        {
            return formatted ? "FRFT" : "RFT";
        }

        std::string smspec(const bool formatted)
        {
            return formatted ? "FSMSPEC" : "SMSPEC";
        }

        std::string summary(const int  rptStep,
                            const bool formatted,
                            const bool unified)
        {
            if (unified) {
                return formatted ? "FUNSMRY" : "UNSMRY";
            }

            return separate(rptStep, formatted, "ABC", "STU");
        }
    } // namespace FileExtension

    namespace Open
    {
        namespace Init
        {
            std::unique_ptr<Opm::EclIO::EclOutput>
            write(const std::string& filename,
                  const bool         isFmt)
            {
                return std::unique_ptr<Opm::EclIO::EclOutput> {
                    new Opm::EclIO::EclOutput {
                        filename, isFmt, std::ios_base::out
                    }
                };
            }
        }

        namespace Restart
        {
            std::unique_ptr<Opm::EclIO::ERst>
            read(const std::string& filename)
            {
                // Bypass some of the internal logic of the ERst constructor.
                //
                // Specifically, the base class EclFile's constructor throws
                // and outputs a highly visible diagnostic message if it is
                // unable to open the file.  The diagnostic message is very
                // confusing to users if they are running a simulation case
                // for the first time and will likely provoke a reaction along
                // the lines of "well of course the restart file doesn't exist".
                {
                    std::ifstream is(filename);

                    if (! is) {
                        // Unable to open (does not exist?).  Return null.
                        return {};
                    }
                }

                // File exists and can (could?) be opened.  Attempt to form
                // an ERst object on top of it.
                return std::unique_ptr<Opm::EclIO::ERst> {
                    new Opm::EclIO::ERst{filename}
                };
            }

            std::unique_ptr<Opm::EclIO::EclOutput>
            writeNew(const std::string& filename,
                     const bool         isFmt)
            {
                return std::unique_ptr<Opm::EclIO::EclOutput> {
                    new Opm::EclIO::EclOutput {
                        filename, isFmt, std::ios_base::out
                    }
                };
            }

            std::unique_ptr<Opm::EclIO::EclOutput>
            writeExisting(const std::string& filename,
                          const bool         isFmt)
            {
                return std::unique_ptr<Opm::EclIO::EclOutput> {
                    new Opm::EclIO::EclOutput {
                        filename, isFmt, std::ios_base::app
                    }
                };
            }
        } // namespace Restart

        namespace Rft
        {
            std::unique_ptr<Opm::EclIO::EclOutput>
            writeNew(const std::string& filename,
                     const bool         isFmt)
            {
                return std::unique_ptr<Opm::EclIO::EclOutput> {
                    new Opm::EclIO::EclOutput {
                        filename, isFmt, std::ios_base::out
                    }
                };
            }

            std::unique_ptr<Opm::EclIO::EclOutput>
            writeExisting(const std::string& filename,
                          const bool         isFmt)
            {
                return std::unique_ptr<Opm::EclIO::EclOutput> {
                    new Opm::EclIO::EclOutput {
                        filename, isFmt, std::ios_base::app
                    }
                };
            }
        } // namespace Rft

        namespace Smspec
        {
            std::unique_ptr<Opm::EclIO::EclOutput>
            write(const std::string& filename,
                  const bool         isFmt)
            {
                return std::unique_ptr<Opm::EclIO::EclOutput> {
                    new Opm::EclIO::EclOutput {
                        filename, isFmt, std::ios_base::out
                    }
                };
            }
        } // namespace Smspec
    } // namespace Open
} // Anonymous namespace

// =====================================================================

Opm::EclIO::OutputStream::Init::
Init(const ResultSet& rset,
     const Formatted& fmt)
{
    const auto fname = outputFileName(rset, FileExtension::init(fmt.set));

    this->open(fname, fmt.set);
}

Opm::EclIO::OutputStream::Init::~Init()
{}

Opm::EclIO::OutputStream::Init::Init(Init&& rhs)
    : stream_{ std::move(rhs.stream_) }
{}

Opm::EclIO::OutputStream::Init&
Opm::EclIO::OutputStream::Init::operator=(Init&& rhs)
{
    this->stream_ = std::move(rhs.stream_);

    return *this;
}

void
Opm::EclIO::OutputStream::Init::
write(const std::string& kw, const std::vector<int>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Init::
write(const std::string& kw, const std::vector<bool>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Init::
write(const std::string& kw, const std::vector<float>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Init::
write(const std::string& kw, const std::vector<double>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Init::
open(const std::string& fname,
     const bool         formatted)
{
    this->stream_ = Open::Init::write(fname, formatted);
}

Opm::EclIO::EclOutput&
Opm::EclIO::OutputStream::Init::stream()
{
    return *this->stream_;
}

namespace Opm { namespace EclIO { namespace OutputStream {

    template <typename T>
    void Init::writeImpl(const std::string&    kw,
                         const std::vector<T>& data)
    {
        this->stream().write(kw, data);
    }

}}}

// =====================================================================

Opm::EclIO::OutputStream::Restart::
Restart(const ResultSet& rset,
        const int        seqnum,
        const Formatted& fmt,
        const Unified&   unif)
{
    const auto ext = FileExtension::
        restart(seqnum, fmt.set, unif.set);

    const auto fname = outputFileName(rset, ext);

    if (unif.set) {
        // Run uses unified restart files.
        this->openUnified(fname, fmt.set, seqnum);

        // Write SEQNUM value to stream to start new output sequence.
        this->stream_->write("SEQNUM", std::vector<int>{ seqnum });
    }
    else {
        // Run uses separate, not unified, restart files.  Create a
        // new output file and open an output stream on it.
        this->openNew(fname, fmt.set);
    }
}

Opm::EclIO::OutputStream::Restart::~Restart()
{}

Opm::EclIO::OutputStream::Restart::Restart(Restart&& rhs)
    : stream_{ std::move(rhs.stream_) }
{}

Opm::EclIO::OutputStream::Restart&
Opm::EclIO::OutputStream::Restart::operator=(Restart&& rhs)
{
    this->stream_ = std::move(rhs.stream_);

    return *this;
}

void Opm::EclIO::OutputStream::Restart::message(const std::string& msg)
{
    this->stream().message(msg);
}

void
Opm::EclIO::OutputStream::Restart::
write(const std::string& kw, const std::vector<int>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Restart::
write(const std::string& kw, const std::vector<bool>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Restart::
write(const std::string& kw, const std::vector<float>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Restart::
write(const std::string& kw, const std::vector<double>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Restart::
write(const std::string& kw, const std::vector<std::string>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Restart::
write(const std::string&                        kw,
      const std::vector<PaddedOutputString<8>>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::Restart::
openUnified(const std::string& fname,
            const bool         formatted,
            const int          seqnum)
{
    // Determine if we're creating a new output/restart file or
    // if we're opening an existing one, possibly at a specific
    // write position.
    auto rst = Open::Restart::read(fname);

    if (rst == nullptr) {
        // No such unified restart file exists.  Create new file.
        this->openNew(fname, formatted);
    }
    else if (! rst->hasKey("SEQNUM")) {
        // File with correct filename exists but does not appear
        // to be an actual unified restart file.
        throw std::invalid_argument {
            "Purported existing unified restart file '"
            + std::filesystem::path{fname}.filename().string()
            + "' does not appear to be a unified restart file"
        };
    }
    else {
        // Restart file exists and appears to be a unified restart
        // resource.  Open writable restart stream backed by the
        // specific file.
        this->openExisting(fname, formatted,
                           rst->restartStepWritePosition(seqnum));
    }
}

void
Opm::EclIO::OutputStream::Restart::
openNew(const std::string& fname,
        const bool         formatted)
{
    this->stream_ = Open::Restart::writeNew(fname, formatted);
}

void
Opm::EclIO::OutputStream::Restart::
openExisting(const std::string&   fname,
             const bool           formatted,
             const std::streampos writePos)
{
    this->stream_ = Open::Restart::writeExisting(fname, formatted);

    if (writePos == std::streampos(-1)) {
        // No specified initial write position.  Typically the case if
        // requested SEQNUM value exceeds all existing SEQNUM values in
        // 'fname'.  This is effectively a simple append operation so
        // no further actions required.
        return;
    }

    // The user specified an initial write position.  Resize existing
    // file (as if by POSIX function ::truncate()) to requested size,
    // and place output position at that position (i.e., new EOF).  This
    // case typically corresponds to reopening a unified restart file at
    // the start of a particular SEQNUM keyword.
    //
    // Note that this intentionally operates on the file/path backing the
    // already opened 'stream_'.  In other words, 'open' followed by
    // resize_file() followed by seekp() is the intended and expected
    // order of operations.

    std::filesystem::resize_file(fname, writePos);

    if (! this->stream_->ofileH.seekp(0, std::ios_base::end)) {
        throw std::invalid_argument {
            "Unable to Seek to Write Position " +
            std::to_string(writePos) + " of File '"
            + fname + "'"
        };
    }
}

Opm::EclIO::EclOutput&
Opm::EclIO::OutputStream::Restart::stream()
{
    return *this->stream_;
}

namespace Opm { namespace EclIO { namespace OutputStream {

    template <typename T>
    void Restart::writeImpl(const std::string&    kw,
                            const std::vector<T>& data)
    {
        this->stream().write(kw, data);
    }

}}}

// =====================================================================

Opm::EclIO::OutputStream::RFT::
RFT(const ResultSet&    rset,
    const Formatted&    fmt,
    const OpenExisting& existing)
{
    const auto fname = outputFileName(rset, FileExtension::rft(fmt.set));

    this->open(fname, fmt.set, existing.set);
}

Opm::EclIO::OutputStream::RFT::~RFT()
{}

Opm::EclIO::OutputStream::RFT::RFT(RFT&& rhs)
    : stream_{ std::move(rhs.stream_) }
{}

Opm::EclIO::OutputStream::RFT&
Opm::EclIO::OutputStream::RFT::operator=(RFT&& rhs)
{
    this->stream_ = std::move(rhs.stream_);

    return *this;
}

void
Opm::EclIO::OutputStream::RFT::
write(const std::string& kw, const std::vector<int>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::RFT::
write(const std::string& kw, const std::vector<float>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::RFT::
write(const std::string&                        kw,
      const std::vector<PaddedOutputString<8>>& data)
{
    this->writeImpl(kw, data);
}

void
Opm::EclIO::OutputStream::RFT::
open(const std::string& fname,
     const bool         formatted,
     const bool         existing)
{
    this->stream_ = existing
        ? Open::Rft::writeExisting(fname, formatted)
        : Open::Rft::writeNew     (fname, formatted);
}

Opm::EclIO::EclOutput&
Opm::EclIO::OutputStream::RFT::stream()
{
    return *this->stream_;
}

namespace Opm { namespace EclIO { namespace OutputStream {

    template <typename T>
    void RFT::writeImpl(const std::string&    kw,
                        const std::vector<T>& data)
    {
        this->stream().write(kw, data);
    }

}}} // namespace Opm::EclIO::OutputStream

// =====================================================================

namespace Opm { namespace EclIO { namespace OutputStream {

namespace {
    bool validUnitConvention(const SummarySpecification::UnitConvention uconv)
    {
        using UConv = SummarySpecification::UnitConvention;

        return (uconv == UConv::Metric)
            || (uconv == UConv::Field)
            || (uconv == UConv::Lab)
            || (uconv == UConv::Pvt_M);
    }

    int unitConvention(const SummarySpecification::UnitConvention uconv)
    {
        const auto unit = static_cast<int>(uconv);

        if (! validUnitConvention(uconv)) {
            throw std::invalid_argument {
                "Invalid Unit Convention: " +
                std::to_string(unit)
            };
        }

        return unit;
    }

    int makeRestartStep(const SummarySpecification::RestartSpecification& restart)
    {
        return (restart.step >= 0) ? restart.step : -1;
    }

    std::vector<PaddedOutputString<8>>
    restartRoot(const SummarySpecification::RestartSpecification& restart)
    {
        const auto substrLength    = std::size_t{8};
        const auto maxSubstrings   = std::size_t{9};
        const auto maxStringLength = maxSubstrings * substrLength;

        auto ret = std::vector<PaddedOutputString<substrLength>>{};

        if (restart.root.empty()) {
            ret.resize(maxSubstrings);
            return ret;
        }

        if (restart.root.size() > maxStringLength) {
            const auto msg = "Restart root name of size "
                + std::to_string(restart.root.size())
                + " exceeds "
                + std::to_string(maxStringLength)
                + " character limit (Ignored)";

            Opm::OpmLog::warning(msg);

            ret.resize(maxSubstrings);
            return ret;
        }

        ret.resize(maxSubstrings);

        auto remain = restart.root.size();

        auto i = decltype(ret.size()){0};
        auto p = decltype(remain){0};
        while (remain > decltype(remain){0}) {
            const auto nchar = std::min(remain, substrLength);
            ret[i++] = restart.root.substr(p, nchar);

            remain -= nchar;
            p      += nchar;
        }

        return ret;
    }

    int microSeconds(const int sec)
    {
        using std::chrono::microseconds;
        using std::chrono::seconds;

        const auto us = microseconds(seconds(sec));

        return static_cast<int>(us.count());
    }

    std::vector<int>
    makeStartDate(const SummarySpecification::StartTime start)
    {
        const auto timepoint = std::chrono::system_clock::to_time_t(start);
        const auto tm = *std::gmtime(&timepoint);

        // { Day, Month, Year, Hour, Minute, Seconds }

        return {
            // 1..31    1..12
            tm.tm_mday, tm.tm_mon + 1,

            tm.tm_year + 1900,

            // 0..23    0..59
            tm.tm_hour, tm.tm_min,

            // 0..59,999,999
            microSeconds(std::min(tm.tm_sec, 59))
        };
    }

    std::vector<int>
    makeDimens(const int                 nparam,
               const std::array<int, 3>& cartDims,
               const int                 istart)
    {
        return { nparam, cartDims[0], cartDims[1], cartDims[2], 0, istart };
    }
} // Anonymous

}}} // namespace Opm::EclIO::OutputStream

void
Opm::EclIO::OutputStream::SummarySpecification::
Parameters::add(const std::string& keyword,
                const std::string& wgname,
                const int          num,
                const std::string& unit)
{
    this->keywords.emplace_back(keyword);
    this->wgnames .emplace_back(wgname);
    this->nums    .push_back   (num);
    this->units   .emplace_back(unit);
}

Opm::EclIO::OutputStream::SummarySpecification::
SummarySpecification(const ResultSet&            rset,
                     const Formatted&            fmt,
                     const UnitConvention        uconv,
                     const std::array<int,3>&    cartDims,
                     const RestartSpecification& restart,
                     const StartTime             start)
    : unit_       (unitConvention(uconv))
    , restartStep_(makeRestartStep(restart))
    , cartDims_   (cartDims)
    , startDate_  (start)
    , restart_    (restartRoot(restart))
{
    const auto fname = outputFileName(rset, FileExtension::smspec(fmt.set));

    this->stream_ = Open::Smspec::write(fname, fmt.set);
}

Opm::EclIO::OutputStream::SummarySpecification::~SummarySpecification()
{}

Opm::EclIO::OutputStream::SummarySpecification::
SummarySpecification(SummarySpecification && rhs)
    : unit_       (rhs.unit_)
    , restartStep_(rhs.restartStep_)
    , cartDims_   (std::move(rhs.cartDims_))
    , startDate_  (std::move(rhs.startDate_))
    , restart_    (rhs.restart_)
    , stream_     (std::move(rhs.stream_))
{}

Opm::EclIO::OutputStream::SummarySpecification&
Opm::EclIO::OutputStream::SummarySpecification::
operator=(SummarySpecification&& rhs)
{
    this->unit_        = rhs.unit_;
    this->restartStep_ = rhs.restartStep_;

    this->cartDims_  = std::move(rhs.cartDims_);
    this->startDate_ = std::move(rhs.startDate_);
    this->restart_   = rhs.restart_;
    this->stream_    = std::move(rhs.stream_);

    return *this;
}

void
Opm::EclIO::OutputStream::
SummarySpecification::write(const Parameters& params)
{
    this->rewindStream();

    auto& smspec = this->stream();

    // Pretend to be ECLIPSE 100
    smspec.write("INTEHEAD", std::vector<int>{ this->unit_, 100 });

    // if (! this->restart_.empty())
        smspec.write("RESTART", this->restart_);

    smspec.write("DIMENS",
                 makeDimens(static_cast<int>(params.keywords.size()),
                            this->cartDims_, this->restartStep_));

    smspec.write("KEYWORDS", params.keywords);
    smspec.write("WGNAMES",  params.wgnames);
    smspec.write("NUMS",     params.nums);
    smspec.write("UNITS",    params.units);

    smspec.write("STARTDAT", makeStartDate(this->startDate_));

    this->flushStream();
}

void Opm::EclIO::OutputStream::SummarySpecification::rewindStream()
{
    // Benefits from EclOutput friendship
    const auto position = std::ofstream::pos_type{0};

    this->stream().ofileH.seekp(position, std::ios_base::beg);
}

void Opm::EclIO::OutputStream::SummarySpecification::flushStream()
{
    this->stream().flushStream();
}

Opm::EclIO::EclOutput&
Opm::EclIO::OutputStream::SummarySpecification::stream()
{
    return *this->stream_;
}

// =====================================================================

std::unique_ptr<Opm::EclIO::EclOutput>
Opm::EclIO::OutputStream::createSummaryFile(const ResultSet& rset,
                                            const int        seqnum,
                                            const Formatted& fmt,
                                            const Unified&   unif)
{
    const auto ext = FileExtension::summary(seqnum, fmt.set, unif.set);

    return std::unique_ptr<Opm::EclIO::EclOutput> {
        new Opm::EclIO::EclOutput {
            outputFileName(rset, ext),
            fmt.set, std::ios_base::out
        }
    };
}

// =====================================================================

std::string
Opm::EclIO::OutputStream::outputFileName(const ResultSet&   rsetDescriptor,
                                         const std::string& ext)
{
    namespace fs = std::filesystem;

    // Allow baseName = "CASE", "CASE.", "CASE.N", or "CASE.N.".
    auto fname = fs::path {
        rsetDescriptor.baseName
        + (rsetDescriptor.baseName.back() == '.' ? "" : ".")
        + "REPLACE"
    }.replace_extension(ext);

    return (fs::path { rsetDescriptor.outputDir } / fname)
        .generic_string();
}

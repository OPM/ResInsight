/*
  Copyright (c) 2021 Equinor ASA

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

#include <opm/io/eclipse/RestartFileView.hpp>

#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
    template <typename T>
    struct ArrayType;

    template<>
    struct ArrayType<int>
    {
        static Opm::EclIO::eclArrType T;
    };

    template<>
    struct ArrayType<bool>
    {
        static Opm::EclIO::eclArrType T;
    };

    template<>
    struct ArrayType<float>
    {
        static Opm::EclIO::eclArrType T;
    };

    template<>
    struct ArrayType<double>
    {
        static Opm::EclIO::eclArrType T;
    };

    template<>
    struct ArrayType<std::string>
    {
        static Opm::EclIO::eclArrType T;
    };

    Opm::EclIO::eclArrType ArrayType<int>::T         = ::Opm::EclIO::eclArrType::INTE;
    Opm::EclIO::eclArrType ArrayType<bool>::T        = ::Opm::EclIO::eclArrType::LOGI;
    Opm::EclIO::eclArrType ArrayType<float>::T       = ::Opm::EclIO::eclArrType::REAL;
    Opm::EclIO::eclArrType ArrayType<double>::T      = ::Opm::EclIO::eclArrType::DOUB;
    Opm::EclIO::eclArrType ArrayType<std::string>::T = ::Opm::EclIO::eclArrType::CHAR;
}

class Opm::EclIO::RestartFileView::Implementation
{
public:
    explicit Implementation(std::shared_ptr<ERst> restart_file,
                            const int             report_step);

    ~Implementation() = default;

    Implementation(const Implementation& rhs) = delete;
    Implementation(Implementation&& rhs);

    Implementation& operator=(const Implementation& rhs) = delete;
    Implementation& operator=(Implementation&& rhs);

    std::size_t simStep() const
    {
        return this->sim_step_;
    }

    int reportStep() const
    {
        return this->report_step_;
    }

    int occurrenceCount(const std::string& vector) const
    {
        return this->rst_file_->occurrence_count(vector, this->report_step_);
    }

    template <typename ElmType>
    bool hasKeyword(const std::string& vector) const
    {
        if (this->rst_file_ == nullptr) { return false; }

        auto coll_iter = this->vectors_.find(ArrayType<ElmType>::T);
        return (coll_iter != this->vectors_.end())
            && this->collectionContains(coll_iter->second, vector);
    }

    template <typename ElmType>
    const std::vector<ElmType>&
    getKeyword(const std::string& vector, const int occurrence)
    {
        return this->rst_file_->
            getRestartData<ElmType>(vector, this->report_step_, occurrence);
    }

    const std::vector<int>& intehead()
    {
        const auto ihkw = std::string { "INTEHEAD" };

        if (! this->hasKeyword<int>(ihkw)) {
            throw std::domain_error {
                "Purported Restart File Does not Have Integer Header"
            };
        }

        return this->getKeyword<int>(ihkw, 0);
    }

    const std::vector<bool>& logihead()
    {
        const auto lhkw = std::string { "LOGIHEAD" };

        if (! this->hasKeyword<bool>(lhkw)) {
            throw std::domain_error {
                "Purported Restart File Does not Have Logical Header"
            };
        }

        return this->getKeyword<bool>(lhkw, 0);
    }

    const std::vector<double>& doubhead()
    {
        const auto dhkw = std::string { "DOUBHEAD" };

        if (! this->hasKeyword<double>(dhkw)) {
            throw std::domain_error {
                "Purported Restart File Does not Have Double Header"
            };
        }

        return this->getKeyword<double>(dhkw, 0);
    }

private:
    using RstFile = std::shared_ptr<ERst>;

    using VectorColl = std::unordered_set<std::string>;
    using TypedColl  = std::unordered_map<
        eclArrType, VectorColl, std::hash<int>
        >;

    RstFile     rst_file_;
    int         report_step_;
    std::size_t sim_step_;
    TypedColl   vectors_;

    bool collectionContains(const VectorColl&  coll,
                            const std::string& vector) const
    {
        return coll.find(vector) != coll.end();
    }
};

Opm::EclIO::RestartFileView::Implementation::
Implementation(std::shared_ptr<ERst> restart_file,
               const int             report_step)
    : rst_file_   { std::move(restart_file) }
    , report_step_(report_step)
    , sim_step_   (std::max(report_step - 1, 0))
{
    if (! this->rst_file_->hasReportStepNumber(this->report_step_)) {
        this->rst_file_.reset();
        return;
    }

    this->rst_file_->loadReportStepNumber(this->report_step_);

    for (const auto& vector : this->rst_file_->listOfRstArrays(this->report_step_)) {
        const auto& type = std::get<1>(vector);

        switch (type) {
        case ::Opm::EclIO::eclArrType::MESS:
            // Currently ignored
            continue;

        default:
            this->vectors_[type].emplace(std::get<0>(vector));
            break;
        }
    }
}

Opm::EclIO::RestartFileView::Implementation::
Implementation(Implementation&& rhs)
    : rst_file_   (std::move(rhs.rst_file_))
    , report_step_(rhs.report_step_)
    , sim_step_   (rhs.sim_step_)            // Scalar (size_t)
    , vectors_    (std::move(rhs.vectors_))
{}

Opm::EclIO::RestartFileView::Implementation&
Opm::EclIO::RestartFileView::Implementation::operator=(Implementation&& rhs)
{
    this->rst_file_    = std::move(rhs.rst_file_);
    this->report_step_ = rhs.report_step_;         // Scalar (int)
    this->sim_step_    = rhs.sim_step_;            // Scalar (size_t)
    this->vectors_     = std::move(rhs.vectors_);

    return *this;
}

Opm::EclIO::RestartFileView::RestartFileView(std::shared_ptr<ERst> restart_file,
                                             const int             report_step)
    : pImpl_{ new Implementation{ std::move(restart_file), report_step } }
{}

Opm::EclIO::RestartFileView::~RestartFileView()
{}

Opm::EclIO::RestartFileView::RestartFileView(RestartFileView&& rhs)
    : pImpl_{ std::move(rhs.pImpl_) }
{}

Opm::EclIO::RestartFileView&
Opm::EclIO::RestartFileView::operator=(RestartFileView&& rhs)
{
    this->pImpl_ = std::move(rhs.pImpl_);
    return *this;
}

std::size_t Opm::EclIO::RestartFileView::simStep() const
{
    return this->pImpl_->simStep();
}

int Opm::EclIO::RestartFileView::reportStep() const
{
    return this->pImpl_->reportStep();
}

int Opm::EclIO::RestartFileView::occurrenceCount(const std::string& vector) const
{
    return this->pImpl_->occurrenceCount(vector);
}

const std::vector<int>& Opm::EclIO::RestartFileView::intehead() const
{
    return this->pImpl_->intehead();
}

const std::vector<bool>& Opm::EclIO::RestartFileView::logihead() const
{
    return this->pImpl_->logihead();
}

const std::vector<double>& Opm::EclIO::RestartFileView::doubhead() const
{
    return this->pImpl_->doubhead();
}

template <typename ElmType>
bool Opm::EclIO::RestartFileView::hasKeyword(const std::string& vector) const
{
    return this->pImpl_->template hasKeyword<ElmType>(vector);
}

template <typename ElmType>
const std::vector<ElmType>&
Opm::EclIO::RestartFileView::getKeyword(const std::string& vector,
                                        const int          occurrence) const
{
    return this->pImpl_->template getKeyword<ElmType>(vector, occurrence);
}

// =====================================================================

namespace Opm { namespace EclIO {

template bool RestartFileView::hasKeyword<int>        (const std::string&) const;
template bool RestartFileView::hasKeyword<bool>       (const std::string&) const;
template bool RestartFileView::hasKeyword<float>      (const std::string&) const;
template bool RestartFileView::hasKeyword<double>     (const std::string&) const;
template bool RestartFileView::hasKeyword<std::string>(const std::string&) const;

template const std::vector<int>&
RestartFileView::getKeyword<int>(const std::string&, const int) const;

template const std::vector<bool>&
RestartFileView::getKeyword<bool>(const std::string&, const int) const;

template const std::vector<float>&
RestartFileView::getKeyword<float>(const std::string&, const int) const;

template const std::vector<double>&
RestartFileView::getKeyword<double>(const std::string&, const int) const;

template const std::vector<std::string>&
RestartFileView::getKeyword<std::string>(const std::string&, const int) const;

}} // Opm::EclIO

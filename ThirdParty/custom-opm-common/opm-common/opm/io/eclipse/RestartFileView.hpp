/*
  Copyright 2021 Equinor ASA.

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

#ifndef OPM_RESTART_FILE_VIEW_HPP
#define OPM_RESTART_FILE_VIEW_HPP

#include <cstddef>
#include <memory>
#include <vector>

namespace Opm { namespace EclIO {
    class ERst;
}} // Opm::EclIO

namespace Opm { namespace EclIO {

class RestartFileView
{
public:
    explicit RestartFileView(std::shared_ptr<ERst> restart_file,
                             const int             report_step);

    ~RestartFileView();

    RestartFileView(const RestartFileView& rhs) = delete;
    RestartFileView(RestartFileView&& rhs);

    RestartFileView& operator=(const RestartFileView& rhs) = delete;
    RestartFileView& operator=(RestartFileView&& rhs);

    std::size_t simStep() const;
    int reportStep() const;

    int occurrenceCount(const std::string& vector) const;

    template <typename ElmType>
    bool hasKeyword(const std::string& vector) const;

    template <typename ElmType>
    const std::vector<ElmType>&
    getKeyword(const std::string& vector, const int occurrence = 0) const;

    const std::vector<int>& intehead() const;
    const std::vector<bool>& logihead() const;
    const std::vector<double>& doubhead() const;

private:
    class Implementation;
    std::unique_ptr<Implementation> pImpl_;
};

}} // Opm::RestartIO

#endif // OPM_RESTART_FILE_VIEW_HPP

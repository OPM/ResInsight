/*
  Copyright 2019 Equinor ASA

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

#include <opm/common/utility/FileSystem.hpp>

#include <algorithm>
#include <random>

namespace Opm
{

std::string unique_path(const std::string& input)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    auto randchar = [&gen]()
    {
        const std::string set = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::uniform_int_distribution<> select(0, set.size()-1);
        return set[select(gen)];
    };

    std::string ret;
    ret.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(ret),
                   [&randchar](const char c)
                   {
                       return (c == '%') ? randchar() : c;
                   });

    return ret;
}

#if !defined(_WIN32) && (__cplusplus < 201703L || \
    (defined(__GNUC__) && __GNUC__ < 8 && !defined(__clang__)))

// The following code has been extracted from libstdc++,
// and slightly modified for use here.
// License is replicated here for attribution.

// Copyright (C) 2014-2021 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

namespace {

bool is_dot(Opm::filesystem::path::value_type c) { return c == '.'; }

bool is_dot(const Opm::filesystem::path& path)
{
    const auto& filename = path.native();
    return filename.size() == 1 && is_dot(filename[0]);
}

bool is_dotdot(const Opm::filesystem::path& path)
{
   const auto& filename = path.native();
   return filename.size() == 2 && is_dot(filename[0]) && is_dot(filename[1]);
}

Opm::filesystem::path lexically_relative(const Opm::filesystem::path& p,
                                         const Opm::filesystem::path& base)
{
    Opm::filesystem::path ret;
    if (p.root_name() != base.root_name())
        return ret;

    if (p.is_absolute() != base.is_absolute())
        return ret;
    if (!p.has_root_directory() && base.has_root_directory())
        return ret;
    auto [a, b] = std::mismatch(p.begin(), p.end(), base.begin(), base.end());
    if (a == p.end() && b == base.end())
        ret = ".";
    else
    {
        int n = 0;
        for (; b != base.end(); ++b)
        {
            const Opm::filesystem::path& p2 = *b;
            if (is_dotdot(p2))
                --n;
            else if (!p2.empty() && !is_dot(p2))
                ++n;
        }
        if (n == 0 && (a == p.end() || a->empty()))
            ret = ".";
        else if (n >= 0)
        {
            const Opm::filesystem::path dotdot("..");
            while (n--)
                ret /= dotdot;
            for (; a != p.end(); ++a)
                ret /= *a;
        }
    }
    return ret;
}

Opm::filesystem::path
lexically_proximate(const Opm::filesystem::path& p, const Opm::filesystem::path& base)
{
  Opm::filesystem::path rel = lexically_relative(p, base);
  return rel.empty() ? p : rel;
}

Opm::filesystem::path
weakly_canonical(const Opm::filesystem::path& p)
{
    Opm::filesystem::path result;
    if (exists(status(p)))
        return canonical(p);

    Opm::filesystem::path tmp;
    auto iter = p.begin(), end = p.end();
    // find leading elements of p that exist:
    while (iter != end)
    {
        tmp = result / *iter;
        if (exists(status(tmp)))
            swap(result, tmp);
        else
            break;
        ++iter;
    }
    // canonicalize:
    if (!result.empty())
        result = canonical(result);
    // append the non-existing elements:
    while (iter != end)
        result /= *iter++;

    return result;
}

}


filesystem::path proximate(const filesystem::path& p, const filesystem::path& base)
{
    return lexically_proximate(weakly_canonical(p), weakly_canonical(base));
}

#endif

}

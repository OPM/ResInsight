//===========================================================================
//
// File: sparsevector_test.cpp
//
// Created: Mon Jun 29 21:00:53 2009
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//            Bård Skaflestad     <bard.skaflestad@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of The Open Reservoir Simulator Project (OpenRS).

  OpenRS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenRS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#define NVERBOSE // to suppress our messages when throwing

#define BOOST_TEST_MODULE SparseVectorTest
#include <boost/test/unit_test.hpp>

#include <opm/common/utility/numeric/SparseVector.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(construction_and_queries)
{
    const SparseVector<int> sv1;
    BOOST_CHECK(sv1.empty());
    BOOST_CHECK_EQUAL(sv1.size(), 0);
    BOOST_CHECK_EQUAL(sv1.nonzeroSize(), 0);

    const int size = 100;
    const int num_elem = 9;
    const int elem[num_elem] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };
    const int indices[num_elem] = { 1, 2, 3, 5, 8, 13, 21, 34, 55 };
    const SparseVector<int> sv2(size, elem, elem + num_elem, indices, indices + num_elem);
    BOOST_CHECK(!sv2.empty());
    BOOST_CHECK_EQUAL(sv2.size(), size);
    BOOST_CHECK_EQUAL(sv2.element(0), 0);
    BOOST_CHECK_EQUAL(sv2.element(1), 9);
    BOOST_CHECK_EQUAL(sv2.element(2), 8);
    BOOST_CHECK_EQUAL(sv2.element(3), 7);
    BOOST_CHECK_EQUAL(sv2.element(4), 0);
    BOOST_CHECK_EQUAL(sv2.element(5), 6);
    BOOST_CHECK_EQUAL(sv2.element(55), 1);
    BOOST_CHECK_EQUAL(sv2.element(99), 0);
    BOOST_CHECK_EQUAL(sv2.nonzeroSize(), num_elem);
    for (int i = 0; i < num_elem; ++i) {
	BOOST_CHECK_EQUAL(sv2.nonzeroElement(i), elem[i]);
    }
    const SparseVector<int> sv2_again(size, elem, elem + num_elem, indices, indices + num_elem);
    BOOST_CHECK(sv2 == sv2_again);
    SparseVector<int> sv2_append(size, elem, elem + num_elem - 1, indices, indices + num_elem - 1);
    BOOST_CHECK_EQUAL(sv2_append.nonzeroSize(), num_elem - 1);
    sv2_append.addElement(elem[num_elem - 1], indices[num_elem - 1]);
    BOOST_CHECK(sv2 == sv2_append);
    SparseVector<int> sv2_append2(size);
    for (int i = 0; i < num_elem; ++i) {
	sv2_append2.addElement(elem[i], indices[i]);
    }
    BOOST_CHECK(sv2 == sv2_append2);
    sv2_append2.clear();
    SparseVector<int> sv_empty;
    BOOST_CHECK(sv2_append2 == sv_empty);

    // Tests that only run in debug mode.
#ifndef NDEBUG
    // One element too few.
    BOOST_CHECK_THROW(const SparseVector<int> sv3(size, elem, elem + num_elem - 1, indices, indices + num_elem), std::exception);
    // One element too many.
    BOOST_CHECK_THROW(const SparseVector<int> sv4(size, elem, elem + num_elem, indices, indices + num_elem - 1), std::exception);
    // Indices out of range.
    BOOST_CHECK_THROW(const SparseVector<int> sv5(4, elem, elem + num_elem, indices, indices + num_elem), std::exception);
    // Indices not strictly increasing. Cheating by using the elements as indices.
    BOOST_CHECK_THROW(const SparseVector<int> sv5(size, elem, elem + num_elem, elem, elem + num_elem), std::exception);

    // Do not ask for out of range indices.
    BOOST_CHECK_THROW(sv1.element(0), std::exception);
    BOOST_CHECK_THROW(sv2.element(-1), std::exception);
    BOOST_CHECK_THROW(sv2.element(sv2.size()), std::exception);
    BOOST_CHECK_THROW(sv2.nonzeroElement(sv2.nonzeroSize()), std::exception);
#endif
}


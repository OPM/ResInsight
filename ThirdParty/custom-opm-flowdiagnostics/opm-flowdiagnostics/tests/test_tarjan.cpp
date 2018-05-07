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

#include <config.h>

#define NVERBOSE

#define BOOST_TEST_MODULE TarjanImplementationTest

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/graph/tarjan.h>

#include <memory>

namespace {
    struct DestroyWorkSpace
    {
        void operator()(TarjanWorkSpace* ws);
    };

    struct DestroySCCResult
    {
        void operator()(TarjanSCCResult *scc);
    };

    void DestroyWorkSpace::operator()(TarjanWorkSpace* ws)
    {
        destroy_tarjan_workspace(ws);
    }

    void DestroySCCResult::operator()(TarjanSCCResult* scc)
    {
        destroy_tarjan_sccresult(scc);
    }

    using WorkSpace =
        std::unique_ptr<TarjanWorkSpace, DestroyWorkSpace>;

    using SCCResult =
        std::unique_ptr<TarjanSCCResult, DestroySCCResult>;

    void check_scc(const std::size_t*     expect_size,
                   const int*             expect_vert,
                   const TarjanSCCResult* scc)
    {
        const auto ncomp = tarjan_get_numcomponents(scc);

        for (auto comp = 0*ncomp, k = 0*ncomp; comp < ncomp; ++comp) {
            const auto c = tarjan_get_strongcomponent(scc, comp);

            BOOST_CHECK_EQUAL(c.size, expect_size[comp]);

            for (auto i = 0*c.size; i < c.size; ++i, ++k) {
                BOOST_CHECK_EQUAL(c.vertex[i], expect_vert[k]);
            }
        }
    }
} // Anonymous

BOOST_AUTO_TEST_SUITE(Two_By_Two)

// +-----+-----+
// |  2  |  3  |
// +-----+-----+
// |  0  |  1  |
// +-----+-----+

BOOST_AUTO_TEST_CASE (FullySeparable)
{
    // Quarter five-spot pattern:
    //   0 -> 1
    //   0 -> 2
    //   1 -> 3
    //   2 -> 3
    //
    // Note: (ia,ja) is INFLOW graph whence tarjan() returns SCCs in
    // topological order from sources to sinks.
    const std::size_t ia[] = { 0, 0, 1, 2, 4 };
    const int         ja[] = { 0, 0, 1, 2 };

    const std::size_t nv           = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t expect_ncomp = 4;

    auto scc = SCCResult{ tarjan(nv, ia, ja) };

    const auto ncomp = tarjan_get_numcomponents(scc.get());

    BOOST_CHECK_EQUAL(ncomp, expect_ncomp);

    {
        const std::size_t expect_size[] = { 1, 1, 1, 1 };
        const int         expect_vert[] = { 0, 1, 2, 3 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] = { 1, 1, 1, 1 };
        const int         expect_vert[] = { 3, 2, 1, 0 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (FullySeparableSparse)
{
    // Quarter five-spot pattern:
    //   0 -> 1
    //   0 -> 2
    //   1 -> 3
    //   2 -> 3
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.
    const std::size_t ia[] = { 0, 2, 3, 4, 4 };
    const int         ja[] = { 1, 2, 3, 3 };

    const int         start_pts[] = { 2 };
    const std::size_t nv          = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t nstart      = (sizeof start_pts) / (sizeof start_pts[0]);

    auto work = WorkSpace{ create_tarjan_workspace(nv) };

    auto scc = SCCResult{
        tarjan_reachable_sccs(nv, ia, ja, nstart,
                              start_pts, work.get())
    };

    BOOST_CHECK_EQUAL(tarjan_get_numcomponents(scc.get()), 2);

    {
        const std::size_t expect_size[] = { 1, 1 };
        const int         expect_vert[] = { 3, 2 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] = { 1, 1 };
        const int         expect_vert[] = { 2, 3 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (Loop)
{
    // Circulation:
    //   0 -> 1
    //   1 -> 3
    //   3 -> 2
    //   2 -> 0
    //
    // Note: (ia,ja) is INFLOW graph whence tarjan() returns SCCs in
    // topological order from sources to sinks.
    const std::size_t ia[] = { 0, 1, 2, 3, 4 };
    const int         ja[] = { 2, 0, 3, 1 };

    const std::size_t nv           = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t expect_ncomp = 1;

    auto scc = SCCResult{ tarjan(nv, ia, ja) };

    const auto ncomp = tarjan_get_numcomponents(scc.get());

    BOOST_CHECK_EQUAL(ncomp, expect_ncomp);

    // Cell indices within component returned in (essentially) arbitrary
    // order.  This particular order happened to be correct at the time the
    // test was implemented so the assertion on 'vertex' is only usable as a
    // regression test.
    {
        const std::size_t expect_size[] = { nv };
        const int         expect_vert[] = { 1, 3, 2, 0 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        // Reversing order of components maintains internal order of
        // vertices within each component.

        const std::size_t expect_size[] = { nv };
        const int         expect_vert[] = { 1, 3, 2, 0 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (LoopSparse)
{
    // Circulation:
    //   0 -> 1
    //   1 -> 3
    //   3 -> 2
    //   2 -> 0
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.
    const std::size_t ia[] = { 0, 1, 2, 3, 4 };
    const int         ja[] = { 1, 3, 0, 2 };

    const int         start_pts[] = { 2 };
    const std::size_t nv          = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t nstart      = (sizeof start_pts) / (sizeof start_pts[0]);

    const std::size_t expect_ncomp = 1;

    auto work = WorkSpace{ create_tarjan_workspace(nv) };

    auto scc = SCCResult{
        tarjan_reachable_sccs(nv, ia, ja, nstart,
                              start_pts, work.get())
    };

    BOOST_CHECK_EQUAL(tarjan_get_numcomponents(scc.get()),
                      expect_ncomp);

    {
        const std::size_t expect_size[] = { nv };
        const int         expect_vert[] = { 3, 1, 0, 2 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        // Reversing order of components maintains internal order of
        // vertices within each component.

        const std::size_t expect_size[] = { nv };
        const int         expect_vert[] = { 3, 1, 0, 2 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (DualPath)
{
    // Two flow paths from cell 0 to cell 2:
    //   0 -> 1
    //   1 -> 3
    //   3 -> 2
    //   0 -> 2
    //
    // Note: (ia,ja) is INFLOW graph whence tarjan() returns SCCs in
    // topological order from sources to sinks.
    const std::size_t ia[] = { 0, 0, 2, 3, 4 };
    const int         ja[] = { 0, 0, 3, 1 };

    const std::size_t nv           = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t expect_ncomp = 4;

    auto scc = SCCResult{ tarjan(nv, ia, ja) };

    const auto ncomp = tarjan_get_numcomponents(scc.get());

    BOOST_CHECK_EQUAL(ncomp, expect_ncomp);

    // Cell 0 is a source and cell 2 is a sink so first and last cells must
    // be 0 and 2 respectively.  The order of cells 1 and 3 is determined by
    // the flow path in which 1 precedes 3.

    {
        const std::size_t expect_size[] = { 1, 1, 1, 1 };
        const int         expect_vert[] = { 0, 1, 3, 2 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] = { 1, 1, 1, 1 };
        const int         expect_vert[] = { 2, 3, 1, 0 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (DualPathSparse)
{
    // Two flow paths from cell 0 to cell 2:
    //   0 -> 1
    //   1 -> 3
    //   3 -> 2
    //   0 -> 2
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.
    const std::size_t ia[] = { 0, 2, 3, 3, 4 };
    const int         ja[] = { 1, 2, 3, 2 };

    const int         start_pts[] = { 1 };
    const std::size_t nv          = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t nstart      = (sizeof start_pts) / (sizeof start_pts[0]);

    const std::size_t expect_ncomp = 3;

    auto work = WorkSpace{ create_tarjan_workspace(nv) };

    auto scc = SCCResult{
        tarjan_reachable_sccs(nv, ia, ja, nstart,
                              start_pts, work.get())
    };

    BOOST_CHECK_EQUAL(tarjan_get_numcomponents(scc.get()),
                      expect_ncomp);

    {
        const std::size_t expect_size[] = { 1, 1, 1 };
        const int         expect_vert[] = { 2, 3, 1 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] = { 1, 1, 1 };
        const int         expect_vert[] = { 1, 3, 2 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (IsolatedFlows)
{
    // Compartmentalised reservoir with source and sink in each compartment.
    //   0 -> 2
    //   3 -> 1
    //
    // Note: (ia,ja) is INFLOW graph whence tarjan() returns SCCs in
    // topological order from sources to sinks.
    const std::size_t ia[] = { 0, 0, 1, 2, 2 };
    const int         ja[] = { 3, 0 };

    const std::size_t nv           = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t expect_ncomp = 4;

    auto scc = SCCResult{ tarjan(nv, ia, ja) };

    const auto ncomp = tarjan_get_numcomponents(scc.get());

    BOOST_CHECK_EQUAL(ncomp, expect_ncomp);

    // Sources before sinks, but no a priori ordering between sources or
    // between sinks.  This particular order happened to be correct at the
    // time the test was implemented so the assertion on 'vert' is only
    // usable as a regression test.

    {
        const std::size_t expect_size[] = { 1, 1, 1, 1 };
        const int         expect_vert[] = { 0, 3, 1, 2 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] = { 1, 1, 1, 1 };
        const int         expect_vert[] = { 2, 1, 3, 0 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (IsolatedFlowsSparse)
{
    // Compartmentalised reservoir with source and sink in each compartment.
    //   0 -> 2
    //   3 -> 1
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.
    const std::size_t ia[] = { 0, 1, 1, 1, 2 };
    const int         ja[] = { 2, 1 };

    const int         start_pts[] = { 3 };
    const std::size_t nv          = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t nstart      = (sizeof start_pts) / (sizeof start_pts[0]);

    const std::size_t expect_ncomp = 2;

    auto work = WorkSpace{ create_tarjan_workspace(nv) };

    auto scc = SCCResult{
        tarjan_reachable_sccs(nv, ia, ja, nstart,
                              start_pts, work.get())
    };

    BOOST_CHECK_EQUAL(tarjan_get_numcomponents(scc.get()),
                      expect_ncomp);

    {
        const std::size_t expect_size[] = { 1, 1 };
        const int         expect_vert[] = { 1, 3 };

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] = { 1, 1 };
        const int         expect_vert[] = { 3, 1 };

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(Four_By_Four)

// +-----+-----+-----+-----+
// | 12  | 13  | 14  | 15  |
// +-----+-----+-----+-----+
// |  8  |  9  | 10  | 11  |
// +-----+-----+-----+-----+
// |  4  |  5  |  6  |  7  |
// +-----+-----+-----+-----+
// |  0  |  1  |  2  |  3  |
// +-----+-----+-----+-----+

BOOST_AUTO_TEST_CASE (CentreLoop)
{
    //  From -> To
    //     0 -> [  1, 4 ],
    //     1 -> [  2, 5 ],
    //     2 -> [  3, 6 ],
    //     3 -> [  7 ],
    //     4 -> [  5, 8 ],
    //     5 -> [  6 ],
    //     6 -> [  7, 10 ],
    //     7 -> [ 11 ],
    //     8 -> [  9, 12 ],
    //     9 -> [  5 ],
    //    10 -> [  9, 11 ],
    //    11 -> [ 15 ],
    //    12 -> [ 13 ],
    //    13 -> [  9, 14 ],
    //    14 -> [ 10, 15 ],
    //    15 -> Void (sink cell)
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.

    const std::size_t ia[] = {0, 2, 4, 6, 7, 9, 10, 12,
                              13, 15, 16, 18, 19, 20, 22, 24, 24};
    const int         ja[] = {1, 4, 2, 5, 3, 6, 7, 5, 8, 6, 7, 10, 11,
                              9, 12, 5, 9, 11, 15, 13, 9, 14, 10, 15};

    const std::size_t nv           = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t expect_ncomp = 13;

    auto scc = SCCResult{ tarjan(nv, ia, ja) };

    const auto ncomp = tarjan_get_numcomponents(scc.get());

    BOOST_CHECK_EQUAL(ncomp, expect_ncomp);

    {
        const std::size_t expect_size[] =
            { 1, 1, 1, 4,      // 0 ..  3
              1, 1, 1, 1,      // 4 ..  7
              1, 1, 1, 1, 1 }; // 8 .. 12

        const int expect_vert[] =
            { 15, 11, 7,        // 0 ..  2
              6, 5, 9, 10,      // 3
              14, 13, 12, 8,    // 4 ..  7
              4, 3, 2, 1, 0 };  // 8 .. 12

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] =
            { 1, 1, 1, 1, 1,    // 0 ..  4
              1, 1, 1, 1,       // 5 ..  8
              4, 1, 1, 1 };     // 9 .. 12

        const int expect_vert[] =
            { 0, 1, 2, 3, 4, //  0 ..  4
              8, 12, 13, 14, //  5 ..  8
              6, 5, 9, 10,   //  9
              7, 11, 15 };   // 10 .. 12

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (CentreLoopSparse)
{
    //  From -> To
    //     0 -> [  1, 4 ],
    //     1 -> [  2, 5 ],
    //     2 -> [  3, 6 ],
    //     3 -> [  7 ],
    //     4 -> [  5, 8 ],
    //     5 -> [  6 ],
    //     6 -> [  7, 10 ],
    //     7 -> [ 11 ],
    //     8 -> [  9, 12 ],
    //     9 -> [  5 ],
    //    10 -> [  9, 11 ],
    //    11 -> [ 15 ],
    //    12 -> [ 13 ],
    //    13 -> [  9, 14 ],
    //    14 -> [ 10, 15 ],
    //    15 -> Void (sink cell)
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.

    const std::size_t ia[] = {0, 2, 4, 6, 7, 9, 10, 12,
                              13, 15, 16, 18, 19, 20, 22, 24, 24};
    const int         ja[] = {1, 4, 2, 5, 3, 6, 7, 5, 8, 6, 7, 10, 11,
                              9, 12, 5, 9, 11, 15, 13, 9, 14, 10, 15};

    const int         start_pts[] = { 5, 12 };
    const std::size_t nv          = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t nstart      = (sizeof start_pts) / (sizeof start_pts[0]);

    const std::size_t expect_ncomp = 7;

    auto work = WorkSpace{ create_tarjan_workspace(nv) };

    auto scc = SCCResult{
        tarjan_reachable_sccs(nv, ia, ja, nstart,
                              start_pts, work.get())
    };

    BOOST_CHECK_EQUAL(tarjan_get_numcomponents(scc.get()),
                      expect_ncomp);

    {
        const std::size_t expect_size[] =
            { 1, 1, 1, 4,       // 0 .. 3
              1, 1, 1 };        // 4 .. 6

        const int expect_vert[] =
            { 15, 11, 7,        // 0 .. 2
              9, 10, 6, 5,      // 3
              14, 13, 12 };     // 4 .. 6

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        // Order of vertices preserved within strong component.

        const std::size_t expect_size[] =
            { 1, 1, 1, 4,       // 0 .. 3
              1, 1, 1 };        // 4 .. 6

        const int expect_vert[] =
            { 12, 13, 14,       // 0 .. 2
              9, 10, 6, 5,      // 3
              7, 11, 15 };      // 4 .. 6

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (CentreLoopSource12)
{
    //  From -> To
    //     0 -> [  1 ],
    //     1 -> [  2 ],
    //     2 -> [  3, 6 ],
    //     3 -> [  7 ],
    //     4 -> [  0, 5 ],
    //     5 -> [  6 ],
    //     6 -> [  7, 10 ],
    //     7 -> [ 11 ],
    //     8 -> [  4, 9 ],
    //     9 -> [  5 ],
    //    10 -> [  9, 11 ],
    //    11 -> [ 15 ],
    //    12 -> [  8, 13 ],
    //    13 -> [ 14 ],
    //    14 -> [ 10, 15 ],
    //    15 -> Void (sink cell)

    const std::size_t ia[] = {0, 1, 2, 4, 5, 7, 8, 10, 11,
                              13, 14, 16, 17, 19, 20, 22, 22};

    const int         ja[] = {1, 2, 3, 6, 7, 0, 5, 6, 7, 10, 11,
                              4, 9, 5, 9, 11, 15, 8, 13, 14, 10, 15};

    const std::size_t nv           = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t expect_ncomp = 13;

    auto scc = SCCResult{ tarjan(nv, ia, ja) };

    const auto ncomp = tarjan_get_numcomponents(scc.get());

    BOOST_CHECK_EQUAL(ncomp, expect_ncomp);

    {
        const std::size_t expect_size[] =
            { 1, 1, 1, 4,      // 0 ..  3
              1, 1, 1, 1,      // 4 ..  7
              1, 1, 1, 1, 1 }; // 8 .. 12

        const int expect_vert[] =
            { 15, 11, 7,           // 0 ..  2
              5, 9, 10, 6,         // 3
              3, 2, 1, 0,          // 4 ..  7
              4, 8, 14, 13, 12 };  // 8 .. 12

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        const std::size_t expect_size[] =
            { 1, 1, 1, 1, 1,    // 0 ..  4
              1, 1, 1, 1,       // 5 ..  8
              4, 1, 1, 1 };     // 9 .. 12

        const int expect_vert[] =
            { 12, 13, 14, 8, 4, //  0 ..  4
              0, 1, 2, 3,       //  5 ..  8
              5, 9, 10, 6,      //  9
              7, 11, 15 };      // 10 .. 12

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_CASE (CentreLoopSource12Sparse)
{
    //  From -> To
    //     0 -> [  1 ],
    //     1 -> [  2 ],
    //     2 -> [  3, 6 ],
    //     3 -> [  7 ],
    //     4 -> [  0, 5 ],
    //     5 -> [  6 ],
    //     6 -> [  7, 10 ],
    //     7 -> [ 11 ],
    //     8 -> [  4, 9 ],
    //     9 -> [  5 ],
    //    10 -> [  9, 11 ],
    //    11 -> [ 15 ],
    //    12 -> [  8, 13 ],
    //    13 -> [ 14 ],
    //    14 -> [ 10, 15 ],
    //    15 -> Void (sink cell)
    //
    // Note: (ia,ja) is OUTFLOW graph.  We use tarjan_reverse_sccresult() to
    // access the SCCs in topological order.

    const std::size_t ia[] = {0, 1, 2, 4, 5, 7, 8, 10, 11,
                              13, 14, 16, 17, 19, 20, 22, 22};

    const int         ja[] = {1, 2, 3, 6, 7, 0, 5, 6, 7, 10, 11,
                              4, 9, 5, 9, 11, 15, 8, 13, 14, 10, 15};

    const int         start_pts[] = { 7, 14 };
    const std::size_t nv          = (sizeof ia) / (sizeof ia[0]) - 1;
    const std::size_t nstart      = (sizeof start_pts) / (sizeof start_pts[0]);

    const std::size_t expect_ncomp = 5;

    auto work = WorkSpace{ create_tarjan_workspace(nv) };

    auto scc = SCCResult{
        tarjan_reachable_sccs(nv, ia, ja, nstart,
                              start_pts, work.get())
    };

    BOOST_CHECK_EQUAL(tarjan_get_numcomponents(scc.get()),
                      expect_ncomp);

    {
        const std::size_t expect_size[] =
            { 1, 1, 1, 4, 1 };  // 0 .. 4

        const int expect_vert[] =
            { 15, 11, 7,           // 0 .. 2
              6, 5, 9, 10,         // 3
              14 };                // 4

        check_scc(expect_size, expect_vert, scc.get());
    }

    if (tarjan_reverse_sccresult(scc.get())) {
        // Order of vertices preserved within each strong component.

        const std::size_t expect_size[] =
            { 1, 4, 1, 1, 1 };  // 0 .. 4

        const int expect_vert[] =
            { 14,               // 0
              6, 5, 9, 10,      // 1
              7, 11, 15 };      // 2 .. 4

        check_scc(expect_size, expect_vert, scc.get());
    }
}

BOOST_AUTO_TEST_SUITE_END()

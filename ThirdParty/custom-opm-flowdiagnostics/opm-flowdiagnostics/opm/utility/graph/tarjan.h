/*
Copyright (C) 2012 (c) Jostein R. Natvig <jostein natvig at gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * \file
 *
 * Simple implementation of of Tarjan's algorithm for computing the
 * strongly connected components of a directed graph, \f$G(V,E)\f$.
 * Run-time complexity is \f$O(|V| + |E|)\f$.
 *
 * The implementation is based on
 * http://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm
 */

#ifndef TARJAN_H_INCLUDED
#define TARJAN_H_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * Abstract datatype providing backing store for working dataset in SCC
 * processing.
 *
 * Intentionally incomplete.
 */
struct TarjanWorkSpace;

/**
 * Abstract datatype providing backing store for SCC result data.
 *
 * Intentionally incomplete.
 */
struct TarjanSCCResult;

/**
 * Contents of single strong component.
 *
 * Essentially a reference type.
 */
struct TarjanComponent
{
    /**
     * Number of vertices in component.
     */
    size_t  size;

    /**
     * IDs of vertices in strong component.  Non-owning pointer.  The
     * vertices of this component are
     * \code
     *   vertex[0 .. size-1]
     * \endcode
     */
    const int *vertex;
};

/**
 * Create work-space for Tarjan algorithm on graph with specified number of
 * nodes (vertices).
 *
 * \param[in] nvert Number of graph vertices.
 *
 * \return Backing store for intermediate data created during SCC
 * processing.  \c NULL in case of allocation failure.  Dispose of
 * work-space using function destroy_tarjan_workspace().
 */
struct TarjanWorkSpace *
create_tarjan_workspace(const size_t nvert);

/**
 * Dispose of backing store for intermediate SCC/Tarjan data.
 *
 * \param[in,out] ws Work-space structure procured in a previous call to
 * function create_tarjan_workspace().  Invalid upon return.
 */
void
destroy_tarjan_workspace(struct TarjanWorkSpace *ws);

/**
 * Dispose of backing store for SCC result data.
 *
 * \param[in,out] scc SCC result data from a previous call to function
 * tarjan() or tarjan_reachable_sccs().  Invalid upon return.
 */
void
destroy_tarjan_sccresult(struct TarjanSCCResult *scc);

/**
 * Retrieve number of strong components in result dataset.
 *
 * \param[in] scc Collection of strongly connected components obtained from
 * a previous call to function tarjan() or tarjan_reachable_sccs().
 *
 * \return Number of strong components in result dataset.
 */
size_t
tarjan_get_numcomponents(const struct TarjanSCCResult *scc);

/**
 * Get access to single strong component from SCC result dataset.
 *
 * \param[in] scc Collection of strongly connected components obtained from
 * a previous call to function tarjan() or tarjan_reachable_sccs().
 *
 * \param[in] compID Linear ID of single strong component.  Must be in the
 * range \code [0 .. tarjan_get_numcomponents(scc) - 1] \endcode.
 *
 * \return Single strong component corresponding to explicit linear ID.
 */
struct TarjanComponent
tarjan_get_strongcomponent(const struct TarjanSCCResult *scc,
                           const size_t                  compID);

/**
 * Compute the strongly connected components of a directed graph,
 * \f$G(V,E)\f$.
 *
 * The components are returned in reverse topological sorted sequence.
 *
 * \param[in] nv Number of graph vertices.
 *
 * \param[in] ia CSR sparse matrix start pointers corresponding to
 *               downstream vertices.
 *
 * \param[in] ja CSR sparse matrix representation of out-neighbours in a
 *               directed graph: vertex \c i has directed edges to vertices
 *               \code ja[ia[i]], ..., ja[ia[i + 1] - 1] \endcode.
 *
 * \return Strong component result dataset.  Owning pointer.  Dispose of
 * associate memory by calling the destructor destroy_tarjan_sccresult().
 * Returns \c NULL in case of failure to allocate the result set or internal
 * work-space.
 */
struct TarjanSCCResult *
tarjan(const size_t  nv,
       const size_t *ia,
       const int    *ja);

/**
 * Compute the strongly connected components of reachable set in a directed
 * graph \f$G(V,E)\$ when starting from a sparse collection of start points.
 *
 * The components are returned in reverse topological order.  Use function
 * tarjan_reverse_sccresult() to access components in topological order.
 *
 * \param[in] nv Number of graph vertices.
 *
 * \param[in] ia CSR sparse matrix start pointers corresponding to
 *               downstream vertices.
 *
 * \param[in] ja CSR sparse matrix representation of out-neighbours in a
 *               directed graph: vertex \c i has directed edges to vertices
 *               \code ja[ia[i]], ..., ja[ia[i + 1] - 1] \endcode.
 *
 * \param[in] nstart Number of start points from which to initiate reachable
 *               set calculations.
 *
 * \param[in] start_pts Vertices from which to initiate reachable set
 *               calculations.  Array of size \p nstart.
 *
 * \param[in,out] ws Backing-store for quantities needed during SCC
 *               processing.  Obtained from a previous call to function
 *               create_tarjan_workspace().
 *
 * \return Strong component result dataset.  Owning pointer.  Dispose of
 * associate memory by calling the destructor destroy_tarjan_sccresult().
 * Returns \c NULL in case of failure to allocate the result set or internal
 * work-space.
 */
struct TarjanSCCResult *
tarjan_reachable_sccs(const size_t            nv,
                      const size_t           *ia,
                      const int              *ja,
                      const size_t            nstart,
                      const int              *start_pts,
                      struct TarjanWorkSpace *ws);

/**
 * Reverse order of SCCs represented by result set.
 *
 * Maintain order of vertices within each strong component.
 *
 * If successful, invalidates any TarjanComponent data held by caller.
 *
 * If unsuccessful, the original component order is maintained in the input
 * argument.  In this case, the requested order can be obtained by
 * traversing the linear component IDs in reverse using accessor function
 * tarjan_get_strongcomponent().
 *
 * \param[in,out] scc On input, collection of strongly connected components
 *                    obtained from a previous call to function tarjan() or
 *                    function tarjan_reachable_sccs().  On output,
 *                    reordered collection of SCCs such that linear access
 *                    in ascending order of component IDs accesses the
 *                    original strong components in reverse order.
 *
 * \return Whether or not the components could be reversed.  One (true) if
 * order reversal successful and zero (false) otherwise--typically due to
 * failure to allocate internal resources for operation.
 */
int
tarjan_reverse_sccresult(struct TarjanSCCResult *scc);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* TARJAN_H_INCLUDED */

/* Local Variables:    */
/* c-basic-offset:4    */
/* End:                */

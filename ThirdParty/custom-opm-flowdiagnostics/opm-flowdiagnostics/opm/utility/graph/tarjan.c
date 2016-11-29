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

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <opm/utility/graph/tarjan.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum VertexMark { DONE = -2, REMAINING = -1 };

struct TarjanWorkSpace
{
    size_t nvert;               /**< Number of vertices */

    int *status;                /**< Vertex processing status */
    int *link;                  /**< Vertex low-link */
    int *time;                  /**< Vertex DFS discovery time */
};

struct TarjanSCCResult
{
    size_t  ncomp;              /**< Running number of SCCs */
    size_t *start;              /**< SCC start pointers (CSR) */
    int    *vert;               /**< SCC contents (CSR format) */

    size_t *vstack, *vstart;    /**< Vertex stack (reuse 'start') */
    int    *cstack, *cstart;    /**< Component stack (reuse 'vert') */
};

static void
initialise_stacks(const size_t nvert, struct TarjanSCCResult *scc)
{
    /* Vertex and component stacks grow from high end downwards */

    scc->vstack = scc->vstart = scc->start + (nvert + 0);
    scc->cstack = scc->cstart = scc->vert  + (nvert - 1);
}

static void
assign_int_vector(size_t n, const int val, int *v)
{
    size_t i;

    for (i = 0; i < n; i++) { v[i] = val; }
}

static void
copy_int_vector(const size_t n, const int* src, int *dst)
{
    size_t i, thres;

    /* Cut-off point from thin air (i.e., no measurement) */
    thres = (size_t) (1LU << 10);

    if (n < thres) {
        for (i = 0; i < n; i++) {
            dst[i] = src[i];
        }
    }
    else {
        memcpy(dst, src, n * sizeof *dst);
    }
}

static struct TarjanSCCResult *
allocate_scc_result(const size_t nvert)
{
    struct TarjanSCCResult *scc, scc0 = { 0 };

    scc = malloc(1 * sizeof *scc);

    if (scc != NULL) {
        *scc = scc0;

        scc->start = malloc((nvert + 1) * sizeof *scc->start);
        scc->vert  = malloc(nvert       * sizeof *scc->vert);

        if ((scc->start == NULL) || (scc->vert == NULL)) {
            destroy_tarjan_sccresult(scc);

            scc = NULL;
        }
    }

    return scc;
}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#define SWAP(x,y,tmp)                           \
    do {                                        \
        (tmp) = (x);                            \
        (x)   = (y);                            \
        (y)   = (tmp);                          \
    } while (0)

/* Stack grows to lower addresses */
#define peek(stack) (*((stack) + 1))
#define push(stack) *(stack)--

static void
capture_scc(const size_t            c,
            struct TarjanWorkSpace *work,
            struct TarjanSCCResult *scc)
{
    int    vertex;
    size_t v;

    /* Initialise strong component end pointer.
     *
     * Supports the equivalent of push_back() on the
     * component. */
    scc->start[ scc->ncomp + 1 ] =
        scc->start[ scc->ncomp + 0 ];

    do {
        assert (scc->cstack != scc->cstart);

        /* Pop strong component stack */
        v = vertex = *++scc->cstack;
        work->status[v] = DONE;

        /* Capture component vertex in VERT while
         * advancing component end pointer */
        scc->vert[ scc->start[ scc->ncomp + 1 ] ++ ] = vertex;
    } while (v != c);

    /* Record completion of strong component.
     * Prepare for next. */
    scc->ncomp += 1;
}

static void
complete_dfs_from_vertex(const size_t            c,
                         struct TarjanWorkSpace *work,
                         struct TarjanSCCResult *scc)
{
    size_t v;

    /* Record strong component if 'c' is root */
    if (work->link[c] == work->time[c]) {
        capture_scc(c, work, scc);
    }

    /* Pop 'c' from DFS (vertex) stack */
    ++scc->vstack;

    if (scc->vstack != scc->vstart) {
        v = peek(scc->vstack);

        work->link[v] = MIN(work->link[v], work->link[c]);
    }
}

static void
dfs_from_descendant(const size_t            c,
                    const size_t           *ia,
                    const int              *ja,
                    struct TarjanWorkSpace *work,
                    struct TarjanSCCResult *scc)
{
    size_t child;

    assert (work->status[c] > 0);

    child = ja[ia[c] + (work->status[c] - 1)];

    /* decrement descendant count of c*/
    --work->status[c];

    if (work->status[child] == REMAINING) {
        /* push child */
        push(scc->vstack) = child;
    }
    else if (work->status[child] >= 0) {
        work->link[c] = MIN(work->link[c], work->time[child]);
    }
    else {
        assert (work->status[child] == DONE);
    }
}

static void
discover_vertex(const size_t            c,
                const size_t           *ia,
                int                    *time,
                struct TarjanWorkSpace *work,
                struct TarjanSCCResult *scc)
{
    /* number of descendants of c */
    work->status[c] = (int) (ia[c + 1] - ia[c]);
    work->time[c]   = work->link[c] = (*time)++;

    push(scc->cstack) = (int) c;
}

/*
 * Note: 'work->status' serves dual purpose during processing.
 *
 *   status[c] = DONE      -> Vertex 'c' fully processed.
 *               REMAINING -> Vertex 'c' undiscovered.
 *               0         -> Vertex 'c' not fully classified--there are no
 *                            remaining descendants for this vertex but we
 *                            do not yet know if the vertex is an SCC all of
 *                            itself or if it is part of a larger component.
 *
 *   status[c] > 0 -> Vertex 'c' has 'status[c]' remaining
 *                    descendants (i.e., successors/children).
 */

static void
tarjan_initialise_csr(struct TarjanSCCResult *scc)
{
    scc->ncomp                   = 0;
    scc->start[ scc->ncomp + 0 ] = 0;
}

static void
tarjan_initialise(struct TarjanWorkSpace *work,
                  struct TarjanSCCResult *scc)
{
    initialise_stacks(work->nvert, scc);

    /* Initialise status for all vertices */
    assign_int_vector(work->nvert, REMAINING, work->status);

    tarjan_initialise_csr(scc);
}

static void
tarjan_local(const size_t            start,
             const size_t           *ia,
             const int              *ja,
             struct TarjanWorkSpace *work,
             struct TarjanSCCResult *scc)
{
    int    time;
    size_t c;

    push(scc->vstack) = start;

    time = 0;

    while (scc->vstack != scc->vstart) {
        c = peek(scc->vstack);

        assert (work->status[c] != DONE);
        assert (work->status[c] >= -2);

        if (work->status[c] == REMAINING) {
            /* Discover() writes to work->status[c] */

            discover_vertex(c, ia, &time, work, scc);
        }

        if (work->status[c] == 0) {
            /* All descendants of 'c' processed */

            complete_dfs_from_vertex(c, work, scc);
        }
        else {
            /* Process next descendant of 'c' */

            dfs_from_descendant(c, ia, ja, work, scc);
        }
    }

    assert (scc->cstack == scc->cstart);
}

static size_t
tarjan_find_start(const size_t                  seed,
                  const struct TarjanWorkSpace *work)
{
    size_t start = seed;

    while ((start < work->nvert) &&
           (work->status[start] == DONE))
    {
        ++start;
    }

    return start;
}

static void
tarjan_global(const size_t           *ia,
              const int              *ja,
              struct TarjanWorkSpace *work,
              struct TarjanSCCResult *scc)
{
    size_t start;

    assert ((work != NULL) && "Work array must be non-NULL");
    assert ((scc  != NULL) && "Result array must be non-NULL");

    tarjan_initialise(work, scc);

    for (start = tarjan_find_start(  0  , work);
         start < work->nvert;
         start = tarjan_find_start(start, work))
    {
        tarjan_local(start, ia, ja, work, scc);
    }
}

static void
reverse_scc_core(const struct TarjanSCCResult *src,
                 struct TarjanSCCResult       *dst)
{
    size_t comp, n;

    tarjan_initialise_csr(dst);

    for (comp = src->ncomp; comp > 0; --comp, ++dst->ncomp) {
        n = src->start[ comp - 0 ] - src->start[ comp - 1 ];

        dst->start[ dst->ncomp + 1 ] =
            dst->start[ dst->ncomp + 0 ];

        copy_int_vector(n, src->vert + src->start[ comp - 1 ],
                        dst->vert + dst->start[ dst->ncomp + 1 ]);

        dst->start[ dst->ncomp + 1 ] += n;
    }
}

static void
swap_scc(struct TarjanSCCResult *scc1,
         struct TarjanSCCResult *scc2)
{
    SWAP(scc1->start, scc2->start, scc1->vstart);
    SWAP(scc1->vert , scc2->vert , scc1->cstart);
}

/* ======================================================================
 * Public interface below separator
 * ====================================================================== */

struct TarjanWorkSpace *
create_tarjan_workspace(const size_t nvert)
{
    struct TarjanWorkSpace *ws, ws0 = { 0 };

    ws = malloc(1 * sizeof *ws);

    if (ws != NULL) {
        *ws = ws0;

        ws->status = malloc(3 * nvert * sizeof *ws->status);

        if (ws->status == NULL) {
            destroy_tarjan_workspace(ws);

            ws = NULL;
        }
        else {
            ws->nvert = nvert;

            ws->link = ws->status + ws->nvert;
            ws->time = ws->link   + ws->nvert;
        }
    }

    return ws;
}

void
destroy_tarjan_workspace(struct TarjanWorkSpace *ws)
{
    if (ws != NULL) {
        free(ws->status);
    }

    free(ws);
}

void
destroy_tarjan_sccresult(struct TarjanSCCResult *scc)
{
    if (scc != NULL) {
        /* Reverse order of acquisition. */
        free(scc->vert);
        free(scc->start);
    }

    free(scc);
}

size_t
tarjan_get_numcomponents(const struct TarjanSCCResult *scc)
{
    return scc->ncomp;
}

struct TarjanComponent
tarjan_get_strongcomponent(const struct TarjanSCCResult *scc,
                           const size_t                  compID)
{
    size_t start;

    assert ((compID < tarjan_get_numcomponents(scc)) &&
            "Component ID out of bounds");

    struct TarjanComponent c = { 0 };

    start = scc->start[compID];

    c.size   = scc->start[compID + 1] - start;
    c.vertex = &scc->vert[start];

    return c;
}

/*
  Compute the strong components of directed graph G(edges, vertices),
  return components in reverse topological sorted sequence.
  Complexity O(|vertices|+|edges|). See "http://en.wikipedia.org/wiki/
  Tarjan's_strongly_connected_components_algorithm".

  nv    - number of vertices

  ia,ja - adjacency matrix for directed graph in compressed sparse row
          format: vertex i has directed edges to vertices ja[ia[i]],
          ..., ja[ia[i+1]-1].
 */

/*--------------------------------------------------------------------*/
struct TarjanSCCResult *
tarjan(const size_t  nv,
       const size_t *ia,
       const int    *ja)
/*--------------------------------------------------------------------*/
{
    struct TarjanWorkSpace *work = NULL;
    struct TarjanSCCResult *scc  = NULL;

    work = create_tarjan_workspace(nv);
    scc  = allocate_scc_result(nv);

    if ((work == NULL) || (scc == NULL)) {
        destroy_tarjan_sccresult(scc);
        destroy_tarjan_workspace(work);

        work = NULL;
        scc  = NULL;
    }

    if (scc != NULL) {
        tarjan_global(ia, ja, work, scc);
    }

    destroy_tarjan_workspace(work);

    return scc;
}

struct TarjanSCCResult *
tarjan_reachable_sccs(const size_t            nv,
                      const size_t           *ia,
                      const int              *ja,
                      const size_t            nstart,
                      const int              *start_pts,
                      struct TarjanWorkSpace *work)
{
    size_t                  i, start;
    struct TarjanSCCResult *scc;

    assert ((work != NULL) && "Work-space must be non-NULL");
    assert ((work->nvert >= nv) &&
            "Work-space must be large enough to accommodate graph");

#if defined(NDEBUG)
    /* Suppress 'unused argument' diagnostic ('nv' only in assert()) */
    (void) nv;
#endif  /* defined(NDEBUG) */

    scc = allocate_scc_result(work->nvert);

    if (scc != NULL) {
        tarjan_initialise(work, scc);

        for (i = 0; i < nstart; i++) {
            start = start_pts[i];

            if (work->status[start] == DONE) {
                continue;
            }

            tarjan_local(start, ia, ja, work, scc);
        }
    }

    return scc;
}

int
tarjan_reverse_sccresult(struct TarjanSCCResult *scc)
{
    int ok;

    struct TarjanSCCResult *rev;

    rev = allocate_scc_result(scc->start[ scc->ncomp ]);

    ok  = rev != NULL;

    if (ok) {
        reverse_scc_core(scc, rev);

        swap_scc(rev, scc);

        destroy_tarjan_sccresult(rev);
    }

    return ok;
}

/* Local Variables:    */
/* c-basic-offset:4    */
/* End:                */

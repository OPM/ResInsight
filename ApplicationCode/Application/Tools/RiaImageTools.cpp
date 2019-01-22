#include "RiaImageTools.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// Meijster, Roerdink, Hesselink
/// A  GENERAL  ALGORITHM  FOR  COMPUTING  DISTANCE TRANSFORMS IN LINEAR TIME
/// http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf
/// Currently Euclidean only, but can be easily extended by replacing the lambda functions.
//--------------------------------------------------------------------------------------------------
void RiaImageTools::distanceTransform2d(std::vector<std::vector<unsigned int>>& image)
{
    if (image.empty())
    {
        return;
    }
    if (image.front().empty())
    {
        return;
    }
    const int64_t M = (int64_t)image.size();
    const int64_t N = (int64_t)image.front().size();

    unsigned int uinf = M + N;

    // First phase
    std::vector<std::vector<unsigned int>> g(M);

#pragma omp parallel for
    for (int64_t x = 0; x < M; ++x)
    {
        g[x].resize(N, uinf);
        if (image[x][0])
        {
            g[x][0] = 0;
        }
        for (int64_t y = 1; y < N - 1; ++y)
        {
            if (image[x][y])
            {
                g[x][y] = 0;
            }
            else
            {
                g[x][y] = 1 + g[x][y - 1];
            }
        }
        for (int64_t y = N - 2; y > 0; --y)
        {
            if (g[x][y + 1] < g[x][y])
            {
                g[x][y] = 1 + g[x][y + 1];
            }
        }
    }

    auto f = [](int64_t x, int64_t i, const std::vector<std::vector<unsigned int>>& g, int64_t y) {
        return (x - i) * (x - i) + g[i][y] * g[i][y];
    };

    auto sep = [](int64_t i, int64_t u, const std::vector<std::vector<unsigned int>>& g, int64_t y) {
        if (i == u) return (int64_t)0;

        int64_t numerator = u * u - i * i + g[u][y] * g[u][y] - g[i][y] * g[i][y];
        int64_t divisor   = 2 * (u - i);
        return numerator / divisor;
    };

    // Second phase
#pragma omp parallel for
    for (int64_t y = 0; y < N; ++y)
    {
        int64_t           q = 0;
        std::vector<unsigned int> s(std::max(N, M), 0u);
        std::vector<unsigned int> t(std::max(N, M), 0u);

        for (int64_t u = 1; u < M - 1; ++u)
        {
            while (q >= 0 && f(t[q], s[q], g, y) > f(t[q], u, g, y))
            {
                q--;
            }
            if (q < 0)
            {
                q    = 0;
                s[0] = u;
            }
            else
            {
                int64_t w = 1 + sep((int64_t)s[q], u, g, y);
                if (w < M)
                {
                    q++;
                    s[q] = u;
                    t[q] = w;
                }
            }
        }
        for (int64_t u = M - 1; u > 0; --u)
        {
            image[u][y] = f(u, s[q], g, y);
            if (u == t[q])
            {
                q = q - 1;
            }
        }
    }
}



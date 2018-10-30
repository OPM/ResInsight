#include "cafContourLines.h"

#include <algorithm>

//===========================================================================
// Note that castab is arranged differently from the FORTRAN code because
// Fortran and C/C++ arrays are transposed of each other, in this case
// it is more tricky as castab is in 3 dimension
//===========================================================================
const int caf::ContourLines::s_castab[3][3][3] =
{
    {
        { 0,0,8 },{ 0,2,5 },{ 7,6,9 }
    },
    {
        { 0,3,4 },{ 1,3,1 },{ 4,3,0 }
    },
    {
        { 9,6,7 },{ 5,2,0 },{ 8,0,0 }
    }
};



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::ContourLines::create(const std::vector<double>& dataXY, const std::vector<double>& xCoords, const std::vector<double>& yCoords, const std::vector<double>& contourLevels, std::vector<std::vector<cvf::Vec2d>>* polygons)
{
    CVF_ASSERT(!contourLevels.empty());
    int nContourLevels = static_cast<int>(contourLevels.size());
    std::vector<int> sh(5, 0);
    std::vector<double> h(5, 0.0), xh(5, 0.0), yh(5, 0.0);

    int nx = static_cast<int>(xCoords.size());
    int ny = static_cast<int>(yCoords.size());

    CVF_ASSERT(static_cast<int>(dataXY.size()) == nx * ny);

    polygons->resize(nContourLevels);

    //===========================================================================
    // The indexing of im and jm should be noted as it has to start from zero
    // unlike the fortran counter part
    //===========================================================================
    int im[4] = { 0,1,1,0 }, jm[4] = { 0,0,1,1 };

    for (int j = (ny - 2); j >= 0; j--) {
        for (int i = 0; i < nx - 1; i++) {
            double temp1, temp2;
            temp1 = std::min(saneValue(gridIndex1d(i, j, nx), dataXY, contourLevels),
                             saneValue(gridIndex1d(i, j + 1, nx), dataXY, contourLevels));
            temp2 = std::min(saneValue(gridIndex1d(i + 1, j, nx), dataXY, contourLevels),
                             saneValue(gridIndex1d(i + 1, j + 1, nx), dataXY, contourLevels));
            double dmin  = std::min(temp1, temp2);
            temp1 = std::max(saneValue(gridIndex1d(i, j, nx), dataXY, contourLevels),
                             saneValue(gridIndex1d(i, j + 1, nx), dataXY, contourLevels));
            temp2 = std::max(saneValue(gridIndex1d(i + 1, j, nx), dataXY, contourLevels),
                             saneValue(gridIndex1d(i + 1, j + 1, nx), dataXY, contourLevels));
            double dmax  = std::max(temp1, temp2);
            if (dmax > contourLevels[0] && dmin < contourLevels[nContourLevels - 1]) {
                for (int k = 0; k < nContourLevels; k++) {
                    if (contourLevels[k] >= dmin && contourLevels[k] <= dmax) {
                        for (int m = 4; m >= 0; m--) {
                            if (m > 0) {
                                //=============================================================
                                // The indexing of im and jm should be noted as it has to
                                // start from zero
                                //=============================================================
                                h[m] = saneValue(gridIndex1d(i + im[m - 1], j + jm[m - 1], nx), dataXY, contourLevels) - contourLevels[k];
                                xh[m] = xCoords[i + im[m - 1]];
                                yh[m] = yCoords[j + jm[m - 1]];
                            }
                            else {
                                h[0] = 0.25*(h[1] + h[2] + h[3] + h[4]);
                                xh[0] = 0.5*(xCoords[i] + xCoords[i + 1]);
                                yh[0] = 0.5*(yCoords[j] + yCoords[j + 1]);
                            }
                            if (h[m] > 0.0) {
                                sh[m] = 1;
                            }
                            else if (h[m] < 0.0) {
                                sh[m] = -1;
                            }
                            else
                                sh[m] = 0;
                        }
                        //=================================================================
                        //
                        // Note: at this stage the relative heights of the corners and the
                        // centre are in the h array, and the corresponding coordinates are
                        // in the xh and yh arrays. The centre of the box is indexed by 0
                        // and the 4 corners by 1 to 4 as shown below.
                        // Each triangle is then indexed by the parameter m, and the 3
                        // vertices of each triangle are indexed by parameters m1,m2,and
                        // m3.
                        // It is assumed that the centre of the box is always vertex 2
                        // though this isimportant only when all 3 vertices lie exactly on
                        // the same contour level, in which case only the side of the box
                        // is drawn.
                        //
                        //
                        //      vertex 4 +-------------------+ vertex 3
                        //               | \               / |
                        //               |   \    m-3    /   |
                        //               |     \       /     |
                        //               |       \   /       |
                        //               |  m=2    X   m=2   |       the centre is vertex 0
                        //               |       /   \       |
                        //               |     /       \     |
                        //               |   /    m=1    \   |
                        //               | /               \ |
                        //      vertex 1 +-------------------+ vertex 2
                        //
                        //
                        //
                        //               Scan each triangle in the box
                        //
                        //=================================================================
                        for (int m = 1; m <= 4; m++) {
                            int m1 = m;
                            int m2 = 0;
                            int m3 = 0;
                            if (m != 4)
                                m3 = m + 1;
                            else
                                m3 = 1;
                            int case_value = s_castab[sh[m1] + 1][sh[m2] + 1][sh[m3] + 1];

                            double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

                            if (case_value != 0) {
                                switch (case_value) {
                                    //===========================================================
                                    //     Case 1 - Line between vertices 1 and 2
                                    //===========================================================
                                case 1:
                                    x1 = xh[m1];
                                    y1 = yh[m1];
                                    x2 = xh[m2];
                                    y2 = yh[m2];
                                    break;
                                    //===========================================================
                                    //     Case 2 - Line between vertices 2 and 3
                                    //===========================================================
                                case 2:
                                    x1 = xh[m2];
                                    y1 = yh[m2];
                                    x2 = xh[m3];
                                    y2 = yh[m3];
                                    break;
                                    //===========================================================
                                    //     Case 3 - Line between vertices 3 and 1
                                    //===========================================================
                                case 3:
                                    x1 = xh[m3];
                                    y1 = yh[m3];
                                    x2 = xh[m1];
                                    y2 = yh[m1];
                                    break;
                                    //===========================================================
                                    //     Case 4 - Line between vertex 1 and side 2-3
                                    //===========================================================
                                case 4:
                                    x1 = xh[m1];
                                    y1 = yh[m1];
                                    x2 = xsect(m2, m3, h, xh, yh);
                                    y2 = ysect(m2, m3, h, xh, yh);
                                    break;
                                    //===========================================================
                                    //     Case 5 - Line between vertex 2 and side 3-1
                                    //===========================================================
                                case 5:
                                    x1 = xh[m2];
                                    y1 = yh[m2];
                                    x2 = xsect(m3, m1, h, xh, yh);
                                    y2 = ysect(m3, m1, h, xh, yh);
                                    break;
                                    //===========================================================
                                    //     Case 6 - Line between vertex 3 and side 1-2
                                    //===========================================================
                                case 6:
                                    x1 = xh[m3];
                                    y1 = yh[m3];
                                    x2 = xsect(m1, m2, h, xh, yh);
                                    y2 = ysect(m1, m2, h, xh, yh);
                                    break;
                                    //===========================================================
                                    //     Case 7 - Line between sides 1-2 and 2-3
                                    //===========================================================
                                case 7:
                                    x1 = xsect(m1, m2, h, xh, yh);
                                    y1 = ysect(m1, m2, h, xh, yh);
                                    x2 = xsect(m2, m3, h, xh, yh);
                                    y2 = ysect(m2, m3, h, xh, yh);
                                    break;
                                    //===========================================================
                                    //     Case 8 - Line between sides 2-3 and 3-1
                                    //===========================================================
                                case 8:
                                    x1 = xsect(m2, m3, h, xh, yh);
                                    y1 = ysect(m2, m3, h, xh, yh);
                                    x2 = xsect(m3, m1, h, xh, yh);
                                    y2 = ysect(m3, m1, h, xh, yh);
                                    break;
                                    //===========================================================
                                    //     Case 9 - Line between sides 3-1 and 1-2
                                    //===========================================================
                                case 9:
                                    x1 = xsect(m3, m1, h, xh, yh);
                                    y1 = ysect(m3, m1, h, xh, yh);
                                    x2 = xsect(m1, m2, h, xh, yh);
                                    y2 = ysect(m1, m2, h, xh, yh);
                                    break;
                                default:
                                    break;
                                }
                                //=============================================================
                                // Put your processing code here and comment out the printf
                                //=============================================================
                                polygons->at(k).push_back(cvf::Vec2d(x1, y1));
                                polygons->at(k).push_back(cvf::Vec2d(x2, y2));
                            }
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double caf::ContourLines::saneValue(int index, const std::vector<double>& dataXY, const std::vector<double>& contourLevels)
{
    CVF_ASSERT(index >= 0 && index < static_cast<int>(dataXY.size()));
    double range = std::max(1.0, contourLevels.back() - contourLevels.front());
    // Place all invalid values below the bottom contour level.
    if (dataXY[index] == -std::numeric_limits<double>::infinity() ||
        dataXY[index] == std::numeric_limits<double>::infinity())
    {
        return contourLevels.front() - range;
    }
    return dataXY[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double caf::ContourLines::xsect(int p1, int p2, const std::vector<double>& h, const std::vector<double>& xh, const std::vector<double>& yh)
{
    return (h[p2] * xh[p1] - h[p1] * xh[p2]) / (h[p2] - h[p1]);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double caf::ContourLines::ysect(int p1, int p2, const std::vector<double>& h, const std::vector<double>& xh, const std::vector<double>& yh)
{
    return (h[p2] * yh[p1] - h[p1] * yh[p2]) / (h[p2] - h[p1]);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::ContourLines::gridIndex1d(int i, int j, int nx)
{
    return j * nx + i;
}

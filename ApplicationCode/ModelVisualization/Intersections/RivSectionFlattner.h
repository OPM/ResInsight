#pragma once


class RivSectionFlattner
{
public:
    //--------------------------------------------------------------------------------------------------
    /// Returns the next index higher than idxToStartOfLineSegment that makes the line 
    //  polyline[idxToStartOfLineSegment] .. polyline[nextIdx] not parallel to extrDir 
    /// 
    /// Returns size_t(-1) if no point is found
    //--------------------------------------------------------------------------------------------------
    static size_t  indexToNextValidPoint(const std::vector<cvf::Vec3d>& polyLine,
                                         const cvf::Vec3d extrDir,
                                         size_t idxToStartOfLineSegment)
    {
        size_t lineCount = polyLine.size();
        if ( !(idxToStartOfLineSegment + 1 < lineCount) ) return -1;


        cvf::Vec3d p1 = polyLine[idxToStartOfLineSegment];

        for ( size_t lIdx = idxToStartOfLineSegment+1; lIdx < lineCount; ++lIdx )
        {
            cvf::Vec3d p2 = polyLine[lIdx];
            cvf::Vec3d p1p2 = p2 - p1;

            if ( (p1p2 - (p1p2 * extrDir)*extrDir).length() > 0.1 )
            {
                return lIdx;
            }
        }

        return -1;
    }

    //--------------------------------------------------------------------------------------------------
    /// Returns one CS pr point, valid for the next segment
    //--------------------------------------------------------------------------------------------------
    static std::vector<cvf::Mat4d> calculateFlatteningCSsForPolyline(const std::vector<cvf::Vec3d> & polyLine,
                                                                     const cvf::Vec3d& extrusionDir,
                                                                     const cvf::Vec3d& startOffset,
                                                                     cvf::Vec3d* endOffset)
    {
        CVF_ASSERT(endOffset);
        size_t pointCount = polyLine.size();
        CVF_ASSERT(pointCount > 1);

        std::vector<cvf::Mat4d> segmentTransforms;
        segmentTransforms.reserve(pointCount);

        // Find initial transform, used if all is vertical

        cvf::Mat4d invSectionCS;
        {
            cvf::Vec3d p1 = polyLine[0];
            cvf::Vec3d p2 = polyLine[1];

            cvf::Mat4d sectionLocalCS = calculateSectionLocalFlatteningCS(p1, p2, extrusionDir);
            cvf::Mat4d invSectionCS = sectionLocalCS.getInverted();
            invSectionCS.setTranslation(invSectionCS.translation() + startOffset);
        }

        cvf::Vec3d previousFlattenedSectionEndPoint = startOffset;

        size_t lIdx = 0;
        while ( lIdx < pointCount )
        {
            size_t idxToNextP = indexToNextValidPoint(polyLine, extrusionDir, lIdx);

            // If the rest is nearly parallel to extrusionDir, use the current inverse matrix for the rest of the points

            if ( idxToNextP == size_t(-1) )
            {
                size_t inc = 0;
                while ( (lIdx + inc) < pointCount )
                {
                    segmentTransforms.push_back(invSectionCS);
                    ++inc;
                }
                break;
            }

            cvf::Vec3d p1 = polyLine[lIdx];
            cvf::Vec3d p2 = polyLine[idxToNextP];

            cvf::Mat4d sectionLocalCS = calculateSectionLocalFlatteningCS(p1, p2, extrusionDir);
            invSectionCS = sectionLocalCS.getInverted();
            cvf::Vec3d flattenedSectionEndPoint = p2.getTransformedPoint(invSectionCS);

            invSectionCS.setTranslation(invSectionCS.translation() + previousFlattenedSectionEndPoint );

            previousFlattenedSectionEndPoint += flattenedSectionEndPoint;

            // Assign the matrix to the points in between

            size_t inc = 0;
            while ( (lIdx + inc) < idxToNextP )
            {
                segmentTransforms.push_back(invSectionCS);
                inc++;
            }

            lIdx = idxToNextP;
        }

        *endOffset = previousFlattenedSectionEndPoint;
        return segmentTransforms;
    }

private:

    //--------------------------------------------------------------------------------------------------
    /// Origo in P1
    /// Ez in upwards extrusionDir
    /// Ey normal to the section plane
    /// Ex in plane along p1-p2
    //--------------------------------------------------------------------------------------------------
    static cvf::Mat4d calculateSectionLocalFlatteningCS(const cvf::Vec3d& p1,
                                                        const cvf::Vec3d& p2,
                                                        const cvf::Vec3d& extrusionDir)
    {
        using namespace cvf;

        Vec3d Ez = extrusionDir.z() > 0.0 ? extrusionDir: -extrusionDir;

        Vec3d sectionLineDir = p2 - p1;

        if ( cvf::GeometryTools::getAngle(sectionLineDir, extrusionDir) < 0.01 )
        {
            sectionLineDir = Ez.perpendicularVector();
        }

        Vec3d Ey = Ez ^ sectionLineDir;
        Ey.normalize();
        Vec3d Ex = Ey ^ Ez;
        Ex.normalize();

        return Mat4d(Ex[0], Ey[0], Ez[0], p1[0],
                     Ex[1], Ey[1], Ez[1], p1[1],
                     Ex[2], Ey[2], Ez[2], p1[2],
                     0.0,   0.0,   0.0, 1.0);
    }


};



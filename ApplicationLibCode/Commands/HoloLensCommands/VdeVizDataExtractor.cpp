/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "VdeVizDataExtractor.h"
#include "VdeArrayDataPacket.h"
#include "VdeCachingHashedIdFactory.h"
#include "VdePacketDirectory.h"

#include "RicHoloLensExportImpl.h"

#include "RifJsonEncodeDecode.h"

#include "RiaLogging.h"

#include "cvfAssert.h"
#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSet.h"
#include "cvfTimer.h"
#include "cvfTrace.h"
#include "cvfTransform.h"

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeVizDataExtractor::VdeVizDataExtractor( const RimGridView& view, VdeCachingHashedIdFactory* cachingIdFactory )
    : m_view( view )
    , m_cachingIdFactory( cachingIdFactory )
{
    CVF_ASSERT( m_cachingIdFactory );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeVizDataExtractor::extractViewContents( QString*            modelMetaJsonStr,
                                               std::vector<int>*   allReferencedArrayIds,
                                               VdePacketDirectory* packetDirectory )
{
    cvf::Timer tim;

    // First extract the parts (cvfPart + info) to be exported from from the ResInsight view
    const std::vector<VdeExportPart> exportPartsArr = RicHoloLensExportImpl::partsForExport( m_view );

    // Convert this to an array of export ready meshes
    const std::vector<std::unique_ptr<VdeMesh>> meshArr        = buildMeshArray( exportPartsArr );
    const int                                   buildMeshes_ms = static_cast<int>( tim.lapTime() * 1000 );

    const size_t meshCount = meshArr.size();
    cvf::Trace::show( "Analyzing and generating array packet data for %d meshes", meshCount );

    std::vector<VdeMeshArrayIds> allMeshesArrayIdsArr;
    size_t                       totNumPrimitives = 0;
    for ( size_t i = 0; i < meshCount; i++ )
    {
        const VdeMesh* mesh = meshArr[i].get();

        const size_t primCount = mesh->connArr.size() / mesh->verticesPerPrimitive;
        totNumPrimitives += primCount;
        cvf::Trace::show( "  mesh %2d:  primCount=%d  vertsPerPrim=%d  meshSourceObjName='%s'  meshSourceObjType='%s'",
                          i,
                          primCount,
                          mesh->verticesPerPrimitive,
                          mesh->meshSourceObjName.toLatin1().constData(),
                          mesh->meshSourceObjTypeStr.toLatin1().constData() );

        VdeMeshArrayIds arrayIdsThisMesh;

        {
            const float* floatArr        = reinterpret_cast<const float*>( mesh->vertexArr->ptr() );
            const size_t arrElementCount = 3 * mesh->vertexArr->size();
            arrayIdsThisMesh.vertexArrId =
                m_cachingIdFactory->getOrCreateIdForFloatArr( VdeCachingHashedIdFactory::VertexArr,
                                                              floatArr,
                                                              arrElementCount );

            if ( !packetDirectory->lookupPacket( arrayIdsThisMesh.vertexArrId ) )
            {
                cvf::Trace::show( "    generating vertices, arrayId=%d", arrayIdsThisMesh.vertexArrId );
                std::unique_ptr<VdeArrayDataPacket> dataPacket =
                    VdeArrayDataPacket::fromFloat32Arr( arrayIdsThisMesh.vertexArrId, floatArr, arrElementCount );

                // Debug testing of decoding
                // debugComparePackets(*dataPacket,
                // VdeArrayDataPacket::fromRawPacketBuffer(dataPacket->fullPacketRawPtr(), dataPacket->fullPacketSize(),
                // nullptr));

                packetDirectory->addPacket( std::move( dataPacket ) );
            }
        }
        {
            const unsigned int* uintArr         = mesh->connArr.data();
            const size_t        arrElementCount = mesh->connArr.size();
            arrayIdsThisMesh.connArrId =
                m_cachingIdFactory->getOrCreateIdForUint32Arr( VdeCachingHashedIdFactory::ConnArr, uintArr, arrElementCount );

            if ( !packetDirectory->lookupPacket( arrayIdsThisMesh.connArrId ) )
            {
                cvf::Trace::show( "    generating connectivities, arrayId=%d", arrayIdsThisMesh.connArrId );
                std::unique_ptr<VdeArrayDataPacket> dataPacket =
                    VdeArrayDataPacket::fromUint32Arr( arrayIdsThisMesh.connArrId, uintArr, arrElementCount );

                // Debug testing of decoding
                // debugComparePackets(*dataPacket,
                // VdeArrayDataPacket::fromRawPacketBuffer(dataPacket->fullPacketRawPtr(), dataPacket->fullPacketSize(),
                // nullptr));

                packetDirectory->addPacket( std::move( dataPacket ) );
            }
        }

        if ( mesh->texCoordArr.notNull() && mesh->texImage.notNull() )
        {
            {
                const float* floatArr        = reinterpret_cast<const float*>( mesh->texCoordArr->ptr() );
                const size_t arrElementCount = 2 * mesh->texCoordArr->size();
                arrayIdsThisMesh.texCoordsArrId =
                    m_cachingIdFactory->getOrCreateIdForFloatArr( VdeCachingHashedIdFactory::TexCoordsArr,
                                                                  floatArr,
                                                                  arrElementCount );

                if ( !packetDirectory->lookupPacket( arrayIdsThisMesh.texCoordsArrId ) )
                {
                    cvf::Trace::show( "    generating texture coords, arrayId=%d", arrayIdsThisMesh.texCoordsArrId );
                    std::unique_ptr<VdeArrayDataPacket> dataPacket =
                        VdeArrayDataPacket::fromFloat32Arr( arrayIdsThisMesh.texCoordsArrId, floatArr, arrElementCount );

                    // Debug testing of decoding
                    // debugComparePackets(*dataPacket,
                    // VdeArrayDataPacket::fromRawPacketBuffer(dataPacket->fullPacketRawPtr(),
                    // dataPacket->fullPacketSize(), nullptr));

                    packetDirectory->addPacket( std::move( dataPacket ) );
                }
            }
            {
                cvf::ref<cvf::UByteArray> byteArr = mesh->texImage->toRgb();
                arrayIdsThisMesh.texImageArrId =
                    m_cachingIdFactory->getOrCreateIdForUint8Arr( VdeCachingHashedIdFactory::TexImage,
                                                                  byteArr->ptr(),
                                                                  byteArr->size() );

                if ( !packetDirectory->lookupPacket( arrayIdsThisMesh.texImageArrId ) )
                {
                    cvf::Trace::show( "    generating texture image, arrayId=%d", arrayIdsThisMesh.texImageArrId );
                    std::unique_ptr<VdeArrayDataPacket> dataPacket =
                        VdeArrayDataPacket::fromUint8ImageRGBArr( arrayIdsThisMesh.texImageArrId,
                                                                  mesh->texImage->width(),
                                                                  mesh->texImage->height(),
                                                                  byteArr->ptr(),
                                                                  byteArr->size() );

                    // Debug testing of decoding
                    // debugComparePackets(*dataPacket,
                    // VdeArrayDataPacket::fromRawPacketBuffer(dataPacket->fullPacketRawPtr(),
                    // dataPacket->fullPacketSize(), nullptr));

                    packetDirectory->addPacket( std::move( dataPacket ) );
                }
            }
        }

        allMeshesArrayIdsArr.push_back( arrayIdsThisMesh );
    }

    const int fillPacketDir_ms = static_cast<int>( tim.lapTime() * 1000 );

    // Extract any exportable labels present in the view
    const std::vector<std::pair<cvf::Vec3f, cvf::String>> labelAndPositionsArr =
        RicHoloLensExportImpl::labelsForExport( m_view );

    // Actually create the JSON containing model meta data
    *modelMetaJsonStr = createModelMetaJsonString( meshArr, allMeshesArrayIdsArr, labelAndPositionsArr );

    // Find all unique packet array IDs referenced
    std::set<int> referencedIdsSet;
    for ( const VdeMeshArrayIds& meshArrayIds : allMeshesArrayIdsArr )
    {
        if ( meshArrayIds.vertexArrId != -1 ) referencedIdsSet.insert( meshArrayIds.vertexArrId );
        if ( meshArrayIds.connArrId != -1 ) referencedIdsSet.insert( meshArrayIds.connArrId );
        if ( meshArrayIds.texImageArrId != -1 ) referencedIdsSet.insert( meshArrayIds.texImageArrId );
        if ( meshArrayIds.texCoordsArrId != -1 ) referencedIdsSet.insert( meshArrayIds.texCoordsArrId );
    }

    allReferencedArrayIds->assign( referencedIdsSet.begin(), referencedIdsSet.end() );

    RiaLogging::debug( QString( "HoloLens: Extracted %1 meshes (total of %2 primitives) in %3ms  (buildMeshes=%4ms, "
                                "fillPacketDir=%5ms)" )
                           .arg( meshCount )
                           .arg( totNumPrimitives )
                           .arg( static_cast<int>( tim.time() * 1000 ) )
                           .arg( buildMeshes_ms )
                           .arg( fillPacketDir_ms ) );

    // cvf::Trace::show("Total number of primitives extracted: %d in %dms", totNumPrimitives,
    // static_cast<int>(tim.time()*1000));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::unique_ptr<VdeMesh>> VdeVizDataExtractor::buildMeshArray( const std::vector<VdeExportPart>& exportPartsArr )
{
    std::vector<std::unique_ptr<VdeMesh>> meshArr;
    for ( const VdeExportPart& exportPart : exportPartsArr )
    {
        std::unique_ptr<VdeMesh> mesh = createMeshFromExportPart( exportPart );
        if ( mesh )
        {
            meshArr.push_back( std::move( mesh ) );
        }
    }

    return meshArr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<VdeMesh> VdeVizDataExtractor::createMeshFromExportPart( const VdeExportPart& exportPart )
{
    // cvf::Timer tim;

    const cvf::Part*        cvfPart = exportPart.part();
    const cvf::DrawableGeo* geo     = dynamic_cast<const cvf::DrawableGeo*>( cvfPart ? cvfPart->drawable() : nullptr );
    if ( !geo )
    {
        return nullptr;
    }

    if ( geo->primitiveSetCount() != 1 )
    {
        RiaLogging::debug( "Only geometries with exactly one primitive set is supported" );
        return nullptr;
    }

    const cvf::Vec3fArray*   vertexArr = geo->vertexArray();
    const cvf::PrimitiveSet* primSet   = geo->primitiveSet( 0 );
    if ( !vertexArr || !primSet || primSet->faceCount() == 0 )
    {
        return nullptr;
    }

    // Support 2 or 3 vertices per primitive
    const cvf::PrimitiveType primType = primSet->primitiveType();
    if ( primType != cvf::PT_TRIANGLES && primType != cvf::PT_LINES )
    {
        RiaLogging::debug(
            QString( "Currently only triangle and line primitive sets are supported (saw primitive type: %1)" ).arg( primType ) );
        return nullptr;
    }

    const int vertsPerPrimitive = ( primType == cvf::PT_TRIANGLES ) ? 3 : 2;

    std::unique_ptr<VdeMesh> mesh( new VdeMesh );
    mesh->verticesPerPrimitive = vertsPerPrimitive;

    // Possibly transform the vertices
    if ( cvfPart->transform() )
    {
        const cvf::Mat4f m = cvf::Mat4f( cvfPart->transform()->worldTransform() );

        cvf::ref<cvf::Vec3fArray> transVertexArr = new cvf::Vec3fArray( *vertexArr );
        const size_t              vertexCount    = transVertexArr->size();
        for ( size_t i = 0; i < vertexCount; i++ )
        {
            transVertexArr->ptr( i )->transformPoint( m );
        }

        mesh->vertexArr = transVertexArr.p();
    }
    else
    {
        mesh->vertexArr = vertexArr;
    }

    // Fetch connectivities
    // Using getFaceIndices() allows us to access strips and fans in the same way as triangles
    // Note that HoloLens visualization wants triangles in clockwise order so we try and fix the winding
    // This point might be moot if the HoloLens visualization always has to use two-sided lighting to get good results
    const size_t faceCount = primSet->faceCount();
    mesh->connArr.reserve( faceCount * vertsPerPrimitive );

    cvf::UIntArray faceConn;
    for ( size_t iface = 0; iface < faceCount; iface++ )
    {
        primSet->getFaceIndices( iface, &faceConn );

        if ( vertsPerPrimitive == 3 && exportPart.winding() == VdeExportPart::COUNTERCLOCKWISE )
        {
            // Reverse the winding
            const size_t numConn = faceConn.size();
            for ( size_t i = 0; i < numConn; i++ )
            {
                mesh->connArr.push_back( faceConn[numConn - i - 1] );
            }
        }
        else
        {
            mesh->connArr.insert( mesh->connArr.end(), faceConn.begin(), faceConn.end() );
        }
    }

    if ( exportPart.textureImage() && geo->textureCoordArray() )
    {
        mesh->texCoordArr = geo->textureCoordArray();
        mesh->texImage    = exportPart.textureImage();
    }

    QString srcObjType = "unknown";
    if ( exportPart.sourceObjectType() == VdeExportPart::OBJ_TYPE_GRID )
        srcObjType = "grid";
    else if ( exportPart.sourceObjectType() == VdeExportPart::OBJ_TYPE_PIPE )
        srcObjType = "pipe";
    mesh->meshSourceObjTypeStr = srcObjType;

    mesh->meshSourceObjName = exportPart.sourceObjectName();

    mesh->color   = exportPart.color();
    mesh->opacity = exportPart.opacity();

    if ( exportPart.cullFace() != VdeExportPart::CF_NONE )
    {
        if ( exportPart.cullFace() == VdeExportPart::CF_FRONT )
            mesh->cullFaceModeStr = "front";
        else if ( exportPart.cullFace() == VdeExportPart::CF_BACK )
            mesh->cullFaceModeStr = "back";
        else
            mesh->cullFaceModeStr = "none";
    }

    // cvf::Trace::show("createMeshFromExportPart(): numFaces=%d, time=%dms", faceCount,
    // static_cast<int>(tim.time()*1000));

    return mesh;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VdeVizDataExtractor::createModelMetaJsonString( const std::vector<std::unique_ptr<VdeMesh>>& meshArr,
                                                        const std::vector<VdeMeshArrayIds>&          meshContentIdsArr,
                                                        const std::vector<std::pair<cvf::Vec3f, cvf::String>>& labelAndPositionsArr )
{
    QVariantList jsonMeshMetaList;
    for ( size_t i = 0; i < meshArr.size(); i++ )
    {
        const VdeMesh*         mesh    = meshArr[i].get();
        const VdeMeshArrayIds& meshIds = meshContentIdsArr[i];

        QMap<QString, QVariant> jsonMeshMeta;
        jsonMeshMeta["meshSourceObjType"] = mesh->meshSourceObjTypeStr;
        jsonMeshMeta["meshSourceObjName"] = mesh->meshSourceObjName;

        jsonMeshMeta["verticesPerPrimitive"] = mesh->verticesPerPrimitive;
        jsonMeshMeta["vertexArrId"]          = meshIds.vertexArrId;
        jsonMeshMeta["connArrId"]            = meshIds.connArrId;

        if ( meshIds.texCoordsArrId >= 0 && meshIds.texImageArrId >= 0 )
        {
            jsonMeshMeta["texCoordsArrId"] = meshIds.texCoordsArrId;
            jsonMeshMeta["texImageArrId"]  = meshIds.texImageArrId;
        }
        else
        {
            QMap<QString, QVariant> jsonColor;
            jsonColor["r"] = mesh->color.r();
            jsonColor["g"] = mesh->color.g();
            jsonColor["b"] = mesh->color.b();

            jsonMeshMeta["color"] = jsonColor;
        }

        jsonMeshMeta["opacity"] = mesh->opacity;

        if ( !mesh->cullFaceModeStr.isEmpty() )
        {
            jsonMeshMeta["cullFaceMode"] = mesh->cullFaceModeStr;
        }

        jsonMeshMetaList.push_back( jsonMeshMeta );
    }

    QVariantList jsonLabelList;
    for ( size_t i = 0; i < labelAndPositionsArr.size(); i++ )
    {
        const cvf::Vec3f&  pos = labelAndPositionsArr[i].first;
        const cvf::String& txt = labelAndPositionsArr[i].second;

        QMap<QString, QVariant> jsonPos;
        jsonPos["x"] = pos.x();
        jsonPos["y"] = pos.y();
        jsonPos["z"] = pos.z();

        QMap<QString, QVariant> jsonLabelEntry;
        jsonLabelEntry["position"] = jsonPos;
        jsonLabelEntry["text"]     = txt.toAscii().ptr();

        jsonLabelList.push_back( jsonLabelEntry );
    }

    QMap<QString, QVariant> jsonModelMeta;
    jsonModelMeta["modelName"] = "ResInsightExport";
    jsonModelMeta["meshArr"]   = jsonMeshMetaList;
    jsonModelMeta["labelsArr"] = jsonLabelList;

    ResInsightInternalJson::Json jsonCodec;
    const bool                   prettifyJson = true;
    QString                      jsonStr      = jsonCodec.encode( jsonModelMeta, prettifyJson );
    return jsonStr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeVizDataExtractor::debugComparePackets( const VdeArrayDataPacket& packetA, const VdeArrayDataPacket& packetB )
{
    CVF_ASSERT( packetA.elementCount() == packetB.elementCount() );
    CVF_ASSERT( packetA.elementSize() == packetB.elementSize() );
    CVF_ASSERT( packetA.elementType() == packetB.elementType() );

    const char* arrA = packetA.arrayData();
    const char* arrB = packetB.arrayData();
    for ( size_t i = 0; i < packetA.elementCount(); i++ )
    {
        CVF_ASSERT( arrA[i] == arrB[i] );
    }
}

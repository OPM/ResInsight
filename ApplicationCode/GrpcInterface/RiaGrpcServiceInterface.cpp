/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////
#include "RiaGrpcServiceInterface.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "RimCase.h"

#include "RicfFieldHandle.h"
#include "RicfMessages.h"

#include "cafPdmDataValueField.h"
#include "cafPdmObject.h"
#include "cafPdmXmlFieldHandle.h"

#include <grpcpp/grpcpp.h>

#include <PdmObject.pb.h>
#include <QXmlStreamReader>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RiaGrpcServiceInterface::findCase(int caseId)
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    for (RimCase* rimCase : cases)
    {
        if (caseId == rimCase->caseId())
        {
            return rimCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Find the number of messages that will fit in the given bytes.
/// The default argument is meant to be a sensible size for GRPC.
//--------------------------------------------------------------------------------------------------
size_t RiaGrpcServiceInterface::numberOfMessagesForByteCount(size_t messageSize,
                                                             size_t numBytesWantedInPackage /*= 64 * 1024u*/)
{
    size_t messageCount = numBytesWantedInPackage / messageSize;
    return messageCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServiceInterface::copyPdmObjectFromCafToRips(const caf::PdmObject* source, rips::PdmObject* destination)
{
    CAF_ASSERT(source && destination);

    destination->set_class_keyword(source->classKeyword().toStdString());
    destination->set_address(reinterpret_cast<uint64_t>(source));
    std::vector<caf::PdmFieldHandle*> fields;
    source->fields(fields);

    auto parametersMap = destination->mutable_parameters();
    for (auto field : fields)
    {
        auto pdmValueField = dynamic_cast<const caf::PdmValueField*>(field);
        if (pdmValueField)
        {
            QString keyword                         = pdmValueField->keyword();
            auto    ricfHandle = field->template capability<RicfFieldHandle>();
            if (ricfHandle != nullptr)
            {
                QString text;
                QTextStream outStream(&text);
                ricfHandle->writeFieldData(outStream, false);
                (*parametersMap)[keyword.toStdString()] = text.toStdString();
            }            
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServiceInterface::copyPdmObjectFromRipsToCaf(const rips::PdmObject* source, caf::PdmObject* destination)
{
    CAF_ASSERT(source && destination);
    CAF_ASSERT(source->class_keyword() == destination->classKeyword().toStdString());
    CAF_ASSERT(source->address() == reinterpret_cast<uint64_t>(destination));
    std::vector<caf::PdmFieldHandle*> fields;
    destination->fields(fields);

    auto parametersMap = source->parameters();
    for (auto field : fields)
    {
        auto pdmValueField = dynamic_cast<caf::PdmValueField*>(field);
        if (pdmValueField)
        {
            QString keyword = pdmValueField->keyword();
            QString value = QString::fromStdString(parametersMap[keyword.toStdString()]);
            
            assignFieldValue(value, pdmValueField);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServiceInterface::assignFieldValue(const QString& stringValue, caf::PdmValueField* field)
{
    auto ricfHandle = field->template capability<RicfFieldHandle>();
    if (field && ricfHandle != nullptr)
    {
        QTextStream stream(stringValue.toLatin1());
        RicfMessages messages;
        ricfHandle->readFieldData(stream, nullptr, &messages, false);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiaGrpcServiceInterface::emplaceChildArrayField(caf::PdmObject* parent, const QString& fieldLabel, const QString& classKeyword)
{
    std::vector<caf::PdmFieldHandle*> fields;
    parent->fields(fields);

    QString childClassKeyword = classKeyword;
   
    for (auto field : fields)
    {
        auto pdmChildArrayField = dynamic_cast<caf::PdmChildArrayFieldHandle*>(field);
        if (pdmChildArrayField && pdmChildArrayField->keyword() == fieldLabel)
        {
            if (childClassKeyword.isEmpty())
            {
                childClassKeyword = pdmChildArrayField->xmlCapability()->childClassKeyword();
            }

            auto pdmObjectHandle =
                caf::PdmDefaultObjectFactory::instance()->create(childClassKeyword);
            caf::PdmObject* pdmObject = dynamic_cast<caf::PdmObject*>(pdmObjectHandle);
            CAF_ASSERT(pdmObject);
            if (pdmObject)
            {
                pdmChildArrayField->insertAt(-1, pdmObject);
                return pdmObject;
            }
        }
    }
    return nullptr;
}


#include "RimDataSourceSteppingTools.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSourceSteppingTools::modifyCurrentIndex(caf::PdmValueField* valueField, const QList<caf::PdmOptionItemInfo>& options, int indexOffset)
{
    if (valueField && !options.isEmpty())
    {
        QVariant currentValue = valueField->toQVariant();
        caf::PdmPointer<caf::PdmObjectHandle> currentHandle = currentValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
        int currentIndex = -1;
        for (int i = 0; i < options.size(); i++)
        {
            QVariant optionValue = options[i].value();
            // First try pointer variety. They are not supported by QVariant::operator==
            caf::PdmPointer<caf::PdmObjectHandle> optionHandle = optionValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            if (optionHandle)
            {
                if (currentHandle == optionHandle)
                {
                    currentIndex = i;
                }
            }
            else if (currentValue == optionValue)
            {                
                currentIndex = i;
            }
        }

        if (currentIndex == -1)
        {
            currentIndex = 0;
        }

        int nextIndex = currentIndex + indexOffset;
        if (nextIndex < options.size() && nextIndex > -1)
        {
            QVariant newValue = options[nextIndex].value();
            valueField->setFromQVariant(newValue);
            valueField->uiCapability()->notifyFieldChanged(currentValue, newValue);
        }
    }
}

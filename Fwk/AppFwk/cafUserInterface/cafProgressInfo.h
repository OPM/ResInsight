//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include <cstddef>

class QString;

namespace caf {

class ProgressInfo
{
public:
    ProgressInfo(size_t maxProgressValue, const QString& title);

    ~ProgressInfo();
    void setProgressDescription(const QString& description);
    void setProgress(size_t progressValue);
    void incrementProgress();
    void incrementProgressAndUpdateNextStep(size_t nextStepSize, const QString& nextDescription);
    void setNextProgressIncrement(size_t nextStepSize);

};


class ProgressInfoStatic 
{
public:
    static void start(size_t maxProgressValue, const QString& title);

    static void setProgressDescription(const QString& description);
    static void setProgress(size_t progressValue);
    static void incrementProgress();
    static void setNextProgressIncrement(size_t nextStepSize);

    static void finished();
};

}

#pragma once

#include "cafFilePath.h"

#include <QTextStream>

#include <vector>

QTextStream& operator<<( QTextStream& str, const std::vector<caf::FilePath>& sobj );
QTextStream& operator>>( QTextStream& str, std::vector<caf::FilePath>& sobj );

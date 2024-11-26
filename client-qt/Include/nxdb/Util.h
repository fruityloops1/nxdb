#pragma once

#include <QString>
#include "pe/Enet/Types.h"

namespace nxdb::util {

    template <typename T>
    inline QString hexStrDigit(T number, int numDigits = 16, int base = 16)
    {
        return QString("%1").arg(QString::number(number, base).toUpper(), numDigits, '0').toUpper();
    }

} // namespace nxdb::util

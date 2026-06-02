#ifndef CRASH_REPORTER_H
#define CRASH_REPORTER_H

#include <types.h>

namespace CrashReporter {
void ShowUserCrashDialog(uint32_t pid, uint32_t tid, uint8_t exceptionNumber, uint32_t errorCode,
                         uint32_t eip, uint32_t cr2, uint32_t tickSnapshot);
}

#endif  // CRASH_REPORTER_H

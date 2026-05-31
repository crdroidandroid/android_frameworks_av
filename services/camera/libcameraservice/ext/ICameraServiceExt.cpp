#include "ICameraServiceExt.h"
#include <log/log.h>

namespace android {

    ICameraServiceExt::ICameraServiceExt() {
        // Do nothing – the real object will call this as part of its construction.
        // No member initialisation to avoid corrupting the real object's memory.
        ALOGV("ICameraServiceExt constructor (stub, no-op)");
    }

    ICameraServiceExt::~ICameraServiceExt() {
        ALOGV("ICameraServiceExt destructor (stub, no-op)");
    }

} // namespace android

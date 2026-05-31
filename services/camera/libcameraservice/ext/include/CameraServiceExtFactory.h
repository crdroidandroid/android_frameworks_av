#pragma once

#include <binder/Parcel.h>

namespace android {

// Forward declaration – we will not define this class
class ICameraServiceExt;

class CameraServiceExtFactory {
public:
    // Returns a pointer to a function table (as required by OxygenOS)
    static void* getInstance();
    static int onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);
    virtual ~CameraServiceExtFactory();

private:
    static void ensureLoaded();
    static void* sFunctionTable;   // pointer to function pointer
    static void* sExtObject;        // the real extension object (as void*)
    static int (*sOnTransactFunc)(void*, uint32_t, const Parcel&, Parcel*, uint32_t);
};

} // namespace android

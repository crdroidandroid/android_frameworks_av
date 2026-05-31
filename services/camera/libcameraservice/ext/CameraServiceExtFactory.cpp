#include "CameraServiceExtFactory.h"
#include <dlfcn.h>
#include <log/log.h>

namespace android {

void* CameraServiceExtFactory::sFunctionTable = nullptr;
void* CameraServiceExtFactory::sExtObject = nullptr;
int (*CameraServiceExtFactory::sOnTransactFunc)(void*, uint32_t, const Parcel&, Parcel*, uint32_t) = nullptr;

void CameraServiceExtFactory::ensureLoaded() {
    if (sFunctionTable != nullptr) return;

    const char* libPath = "system_ext/lib64/libcsextimpl.so";
    void* handle = dlopen(libPath, RTLD_NOW);
    if (handle == nullptr) {
        ALOGE("CameraServiceExtFactory: dlopen failed: %s", dlerror());
        return;
    }
    ALOGI("CameraServiceExtFactory: dlopen succeeded, handle=%p", handle);

    typedef void* (*GetFactoryFunc)();
    GetFactoryFunc getExtFactoryImpl = (GetFactoryFunc)dlsym(handle, "getExtFactoryImpl");
    if (getExtFactoryImpl == nullptr) {
        ALOGE("CameraServiceExtFactory: dlsym getExtFactoryImpl failed: %s", dlerror());
        dlclose(handle);
        return;
    }
    ALOGI("CameraServiceExtFactory: getExtFactoryImpl at %p", getExtFactoryImpl);

    // Triple indirection as determined from logs: getExtFactoryImpl returns ptr to ptr to ptr to function
    void* ptrToPtr = getExtFactoryImpl();
    if (ptrToPtr == nullptr) {
        ALOGE("CameraServiceExtFactory: getExtFactoryImpl returned null");
        dlclose(handle);
        return;
    }

    void* ptrToFunc = *(void**)ptrToPtr;
    if (ptrToFunc == nullptr) {
        ALOGE("CameraServiceExtFactory: first deref gave null");
        dlclose(handle);
        return;
    }

    void* actualFunc = *(void**)ptrToFunc;
    if (actualFunc == nullptr) {
        ALOGE("CameraServiceExtFactory: second deref gave null");
        dlclose(handle);
        return;
    }
    ALOGI("CameraServiceExtFactory: actual factory function at %p", actualFunc);

    sFunctionTable = operator new(8);
    *(void**)sFunctionTable = actualFunc;
    ALOGI("CameraServiceExtFactory: function table at %p", sFunctionTable);

    // Resolve onTransact (for direct call via vtable)
    sOnTransactFunc = (int (*)(void*, uint32_t, const Parcel&, Parcel*, uint32_t))
        dlsym(handle, "_ZN7android20CameraServiceExtImpl10onTransactEjRKNS_6ParcelEPS1_j");
    if (sOnTransactFunc == nullptr) {
        ALOGE("CameraServiceExtFactory: dlsym onTransact failed: %s", dlerror());
    } else {
        ALOGI("CameraServiceExtFactory: onTransact found at %p", sOnTransactFunc);
    }
}

void* CameraServiceExtFactory::getInstance() {
    ensureLoaded();
    return sFunctionTable;   // may be null
}

int CameraServiceExtFactory::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    ensureLoaded();
    if (sExtObject == nullptr) {
        if (sFunctionTable == nullptr) {
            ALOGE("CameraServiceExtFactory::onTransact: extension not loaded");
            return -1;
        }
        void* actualFunc = *(void**)sFunctionTable;
        if (actualFunc == nullptr) return -1;
        typedef void* (*GetObjectFunc)();
        sExtObject = ((GetObjectFunc)actualFunc)();
        if (sExtObject == nullptr) {
            ALOGE("CameraServiceExtFactory: factory returned null");
            return -1;
        }
        ALOGI("CameraServiceExtFactory: real extension object at %p", sExtObject);
    }

    if (sOnTransactFunc == nullptr) {
        ALOGE("CameraServiceExtFactory::onTransact: no function pointer");
        return -1;
    }
    return sOnTransactFunc(sExtObject, code, data, reply, flags);
}

CameraServiceExtFactory::~CameraServiceExtFactory() {
    // No cleanup needed – the extension library manages its own singleton.
    ALOGV("CameraServiceExtFactory destructor (stub)");
}

} // namespace android

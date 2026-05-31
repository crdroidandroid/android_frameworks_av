#pragma once

namespace android {

    // Empty placeholder class – no members, no base classes.
    // The real implementation from the extension library will provide its own.
    class ICameraServiceExt {
    public:
        ICameraServiceExt();
        ~ICameraServiceExt();
        // No other methods – the real object's vtable will be used.
    };

} // namespace android

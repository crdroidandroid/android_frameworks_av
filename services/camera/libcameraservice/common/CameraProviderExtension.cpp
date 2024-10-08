/*
 * Copyright 2024 The LibreMobileOS Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/CameraProviderExtension.h"

__attribute__((weak)) bool supportsTorchStrengthControlExt() {
    return false;
}

__attribute__((weak)) int32_t getTorchDefaultStrengthLevelExt() {
    // Without extension, assume only one level of torch strength
    return 1;
}

__attribute__((weak)) int32_t getTorchMaxStrengthLevelExt() {
    // Without extension, assume only one level of torch strength
    return 1;
}

__attribute__((weak)) int32_t getTorchStrengthLevelExt() {
    return 0;
}

__attribute__((weak)) void setTorchStrengthLevelExt(__unused int32_t torchStrength) {
    // Nothing
}

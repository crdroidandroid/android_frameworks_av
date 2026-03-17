/*
 * Copyright (C) 2022 Project Kaleidoscope
 * Copyright (C) 2025 AxionOS
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

#pragma once

#include <map>
#include <set>
#include <vector>
#include <media/AppVolume.h>
#include "IAfThread.h"
#include "IAfTrack.h"

namespace android::audioflinger {

inline float appVolumeAdjust(float volume, const sp<IAfTrackBase>& track) {
    if (track->isAppMuted()) return 0.f;
    return volume * track->getAppVolume();
}

class AppVolumeHelper {
public:
    void applyToTrack(const sp<IAfTrack>& track) const {
        String8 pkg = track->getPackageName();
        if (pkg.empty()) return;
        auto it = mConfigs.find(pkg);
        if (it == mConfigs.end()) return;
        track->setAppMute(it->second.muted);
        track->setAppVolume(it->second.volume);
    }

    status_t setVolume(const String8& packageName, float value,
            const std::map<audio_io_handle_t, sp<IAfPlaybackThread>>& threads) {
        for (auto& [handle, thread] : threads) {
            if (thread) thread->setAppVolume(packageName, value);
        }
        auto it = mConfigs.find(packageName);
        if (it == mConfigs.end()) {
            media::AppVolume vol;
            vol.packageName = packageName;
            vol.volume = value;
            vol.muted = false;
            mConfigs[packageName] = vol;
        } else {
            it->second.volume = value;
        }
        return NO_ERROR;
    }

    status_t setMute(const String8& packageName, bool value,
            const std::map<audio_io_handle_t, sp<IAfPlaybackThread>>& threads) {
        for (auto& [handle, thread] : threads) {
            if (thread) thread->setAppMute(packageName, value);
        }
        auto it = mConfigs.find(packageName);
        if (it == mConfigs.end()) {
            media::AppVolume vol;
            vol.packageName = packageName;
            vol.volume = 1.0f;
            vol.muted = value;
            mConfigs[packageName] = vol;
        } else {
            it->second.muted = value;
        }
        return NO_ERROR;
    }

    status_t listVolumes(std::vector<media::AppVolume>* vols,
            const std::map<audio_io_handle_t, sp<IAfPlaybackThread>>& threads) const {
        std::set<media::AppVolume> volSet;
        for (auto& [handle, thread] : threads) {
            if (thread) thread->listAppVolumes(volSet);
        }
        vols->insert(vols->begin(), volSet.begin(), volSet.end());
        return NO_ERROR;
    }

private:
    std::map<String8, media::AppVolume> mConfigs;
};

} // namespace android::audioflinger

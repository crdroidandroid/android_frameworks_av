/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_C2_SOFT_AAC_DEC_H_
#define ANDROID_C2_SOFT_AAC_DEC_H_

#include <SimpleC2Component.h>


#include "aacdecoder_lib.h"
#include "DrcPresModeWrap.h"

namespace android {

struct C2SoftAacDec : public SimpleC2Component {
    class IntfImpl;

    C2SoftAacDec(const char *name, c2_node_id_t id, const std::shared_ptr<IntfImpl> &intfImpl);
    virtual ~C2SoftAacDec();

    // From SimpleC2Component
    c2_status_t onInit() override;
    c2_status_t onStop() override;
    void onReset() override;
    void onRelease() override;
    c2_status_t onFlush_sm() override;
    void process(
            const std::unique_ptr<C2Work> &work,
            const std::shared_ptr<C2BlockPool> &pool) override;
    c2_status_t drain(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool) override;

private:
    enum {
        kNumDelayBlocksMax      = 8,
    };

    std::shared_ptr<IntfImpl> mIntf;

    HANDLE_AACDECODER mAACDecoder;
    CStreamInfo *mStreamInfo;
    bool mIsFirst;
    size_t mInputBufferCount;
    size_t mOutputBufferCount;
    bool mSignalledError;
    size_t mOutputPortDelay;
    struct Info {
        uint64_t frameIndex;
        size_t bufferSize;
        uint64_t timestamp;
        std::vector<int32_t> decodedSizes;
    };
    std::list<Info> mBuffersInfo;

    CDrcPresModeWrapper mDrcWrap;

    enum {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    } mOutputPortSettingsChange;

    void initPorts();
    status_t initDecoder();
    bool isConfigured() const;
    void drainDecoder();

    void drainDelayBuffer(
            const std::unique_ptr<C2Work> &work,
            const std::shared_ptr<C2BlockPool> &pool,
            bool eos);
    c2_status_t drainInternal(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool,
            const std::unique_ptr<C2Work> &work);

    struct DelayBuffer {
        int32_t mNumFilledSamples;
        int32_t mReadOffset;
        int32_t mSampleCapacity;
        INT_PCM *mBuffer;
        explicit DelayBuffer(INT_PCM *samples, int32_t numSamples)
            : mNumFilledSamples(numSamples),
            mReadOffset(0),
            mSampleCapacity(numSamples),
            mBuffer(nullptr) {
            mBuffer = new INT_PCM[numSamples];
            if (samples != nullptr) {
                memcpy(mBuffer, samples, numSamples * sizeof(INT_PCM));
            }
        }
        ~DelayBuffer() {
            delete[] mBuffer;
        }

        void putSamples(INT_PCM *samples, int32_t numSamples) {
            if (mSampleCapacity < numSamples) {
                delete[] mBuffer;
                mBuffer = new INT_PCM[numSamples];
                mSampleCapacity = numSamples;
            }
            if (samples != nullptr) {
                memcpy(mBuffer, samples, numSamples * sizeof(INT_PCM));
            }
            mNumFilledSamples = numSamples;
            mReadOffset = 0;
        }

        int32_t getSamples(INT_PCM *samples, int32_t numSamples) {
            int32_t numRead = numSamples;
            if (mNumFilledSamples < numSamples) {
                numRead = mNumFilledSamples;
            }
            if (samples != nullptr) {
                memcpy(samples, &mBuffer[mReadOffset], numRead * sizeof(INT_PCM));
            }
            mNumFilledSamples -= numRead;
            mReadOffset += numRead;
            return numRead;
        }
    };


//      delay compensation
    bool mEndOfInput;
    bool mEndOfOutput;
    int32_t mOutputDelayCompensated;
    std::list<std::shared_ptr<DelayBuffer>> mOutputDelayBuffers;
    std::list<std::shared_ptr<DelayBuffer>> mOutputDelayBufferPool;
    int32_t mOutputDelayBufferFilled;
    int mDeviceApiLevel;
    bool outputDelayBufferPutSamples(INT_PCM *samples, int numSamples);
    int32_t outputDelayBufferGetSamples(INT_PCM *samples, int numSamples);
    int32_t outputDelayBufferSamplesAvailable();
    uint32_t maskFromCount(uint32_t channelCount);

    C2_DO_NOT_COPY(C2SoftAacDec);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_AAC_DEC_H_

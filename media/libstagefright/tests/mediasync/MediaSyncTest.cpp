/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/IProducerListener.h>
#include <gui/mock/GraphicBufferProducer.h>
#include <media/stagefright/MediaSync.h>
#include <system/window.h>
#include <utils/Errors.h>

using namespace android;
using ::testing::_;
using ::testing::Return;

class MediaSyncTest : public ::testing::Test {
  protected:
    MediaSyncTest() {
        ON_CALL(*mMockProducer, connect(_, _, _, _))
                .WillByDefault([this](const sp<IProducerListener>& listener, int, bool,
                                      IGraphicBufferProducer::QueueBufferOutput* output) {
                    mCapturedListener = listener;
                    *output = IGraphicBufferProducer::QueueBufferOutput();
                    return NO_ERROR;
                });

        ON_CALL(*mMockProducer, query(_, _)).WillByDefault([](int what, int* value) {
            switch (what) {
                case NATIVE_WINDOW_CONSUMER_USAGE_BITS:
                    *value = 0;
                    return NO_ERROR;
                case NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS:
                    *value = 1;
                    return NO_ERROR;
                default:
                    return BAD_VALUE;
            }
        });
    }

    void setSurfaceAndCaptureListener() { ASSERT_EQ(OK, mMediaSync->setSurface(mMockProducer)); }

    sp<mock::GraphicBufferProducer> mMockProducer =
            sp<::testing::NiceMock<mock::GraphicBufferProducer>>::make();
    sp<MediaSync> mMediaSync = MediaSync::create();
    sp<IProducerListener> mCapturedListener;
};

// Verify that onBufferReleasedByOutput handles DEAD_OBJECT from
// detachNextBuffer without crashing.
TEST_F(MediaSyncTest, DetachNextBufferReturnsDeadObject) {
    setSurfaceAndCaptureListener();
    EXPECT_CALL(*mMockProducer, detachNextBuffer(_, _)).WillOnce(Return(DEAD_OBJECT));
    mCapturedListener->onBufferReleased();
}

// Verify that onBufferReleasedByOutput handles FAILED_TRANSACTION from
// detachNextBuffer without crashing.
TEST_F(MediaSyncTest, DetachNextBufferReturnsFailedTransaction) {
    setSurfaceAndCaptureListener();
    EXPECT_CALL(*mMockProducer, detachNextBuffer(_, _)).WillOnce(Return(FAILED_TRANSACTION));
    mCapturedListener->onBufferReleased();
}

// Verify that onBufferReleasedByOutput handles NO_INIT from
// detachNextBuffer without crashing.
TEST_F(MediaSyncTest, DetachNextBufferReturnsNoInit) {
    setSurfaceAndCaptureListener();
    sp<IGraphicBufferProducer> inputProducer;
    ASSERT_EQ(OK, mMediaSync->createInputSurface(&inputProducer));
    EXPECT_CALL(*mMockProducer, detachNextBuffer(_, _)).WillOnce(Return(NO_INIT));
    mCapturedListener->onBufferReleased();
}

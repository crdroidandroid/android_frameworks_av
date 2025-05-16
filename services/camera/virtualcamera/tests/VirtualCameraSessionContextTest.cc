/*
 * Copyright 2025 The Android Open Source Project
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

#include <memory>

#include "VirtualCameraSessionContext.h"
#include "aidl/android/hardware/camera/common/Status.h"
#include "aidl/android/hardware/camera/device/StreamConfiguration.h"
#include "aidl/android/hardware/graphics/common/PixelFormat.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/EglDisplayContext.h"
#include "util/MetadataUtil.h"

namespace android {
namespace companion {
namespace virtualcamera {
namespace {

constexpr int kVgaWidth = 640;
constexpr int kVgaHeight = 480;
constexpr int kSvgaWidth = 800;
constexpr int kSvgaHeight = 600;
constexpr int kStreamId = 0;
constexpr int kSecondStreamId = 1;

using ::aidl::android::companion::virtualcamera::Format;
using ::aidl::android::hardware::camera::common::Status;
using ::aidl::android::hardware::camera::device::CaptureRequest;
using ::aidl::android::hardware::camera::device::Stream;
using ::aidl::android::hardware::camera::device::StreamBuffer;
using ::aidl::android::hardware::camera::device::StreamConfiguration;
using ::aidl::android::hardware::graphics::common::PixelFormat;
using ::testing::_;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Not;
using ::testing::SizeIs;

Stream createStream(int streamId, int width, int height, PixelFormat format) {
  Stream s;
  s.id = streamId;
  s.width = width;
  s.height = height;
  s.format = format;
  return s;
}

class VirtualCameraSessionContextTest : public ::testing::Test {
 public:
  void SetUp() override {
  }

 protected:
  EglDisplayContext mEglDisplayContext;
};

TEST_F(VirtualCameraSessionContextTest, scratchBufferCreatedAndCached) {
  VirtualCameraSessionContext sessionContext;
  auto blobStream =
      createStream(kStreamId, kVgaWidth, kVgaHeight, PixelFormat::BLOB);
  sessionContext.initializeStream(blobStream);

  auto framebuffer = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kVgaWidth, kVgaHeight));
  ASSERT_THAT(framebuffer, Not(IsNull()));
  EXPECT_THAT(framebuffer->getWidth(), Eq(kVgaWidth));
  EXPECT_THAT(framebuffer->getHeight(), Eq(kVgaHeight));

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));

  // Expect second query for the same stream and resolution to return the same
  // framebuffer.
  auto framebuffer2 = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kVgaWidth, kVgaHeight));
  EXPECT_THAT(framebuffer2, Eq(framebuffer));
}

TEST_F(VirtualCameraSessionContextTest,
       newScratchBufferCreatedForDifferentResolution) {
  VirtualCameraSessionContext sessionContext;
  auto blobStream =
      createStream(kStreamId, kVgaWidth, kVgaHeight, PixelFormat::BLOB);
  sessionContext.initializeStream(blobStream);

  auto framebuffer = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kVgaWidth, kVgaHeight));
  ASSERT_THAT(framebuffer, Not(IsNull()));
  EXPECT_THAT(framebuffer->getWidth(), Eq(kVgaWidth));
  EXPECT_THAT(framebuffer->getHeight(), Eq(kVgaHeight));

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));

  // Expect second query for the different resolution should allocate a new buffer.
  auto framebuffer2 = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kSvgaWidth, kSvgaHeight));
  EXPECT_THAT(framebuffer2, Not(Eq(framebuffer)));
  ASSERT_THAT(framebuffer2, Not(IsNull()));
  EXPECT_THAT(framebuffer2->getWidth(), Eq(kSvgaWidth));
  EXPECT_THAT(framebuffer2->getHeight(), Eq(kSvgaHeight));

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kSvgaWidth, kSvgaHeight)));
}

TEST_F(VirtualCameraSessionContextTest, closeAllStreamsRemovesScratchBuffers) {
  VirtualCameraSessionContext sessionContext;
  auto blobStream =
      createStream(kStreamId, kVgaWidth, kVgaHeight, PixelFormat::BLOB);
  sessionContext.initializeStream(blobStream);

  auto framebuffer = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kVgaWidth, kVgaHeight));
  ASSERT_THAT(framebuffer, Not(IsNull()));

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));

  sessionContext.closeAllStreams();

  EXPECT_FALSE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));
}

TEST_F(VirtualCameraSessionContextTest,
       discardingStreamRemovesAssociatedScratchBuffer) {
  VirtualCameraSessionContext sessionContext;
  auto blobStream =
      createStream(kStreamId, kVgaWidth, kVgaHeight, PixelFormat::BLOB);
  sessionContext.initializeStream(blobStream);

  auto framebuffer = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kVgaWidth, kVgaHeight));
  ASSERT_THAT(framebuffer, Not(IsNull()));

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));

  auto yuvStream = createStream(kSecondStreamId, kVgaWidth, kVgaHeight,
                                PixelFormat::YCBCR_420_888);

  sessionContext.removeStreamsNotInStreamConfiguration(
      StreamConfiguration{.streams = {yuvStream}});

  EXPECT_FALSE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));
}

TEST_F(VirtualCameraSessionContextTest,
       persistingStreamKeepsAssociatedScratchBuffer) {
  VirtualCameraSessionContext sessionContext;
  auto blobStream =
      createStream(kStreamId, kVgaWidth, kVgaHeight, PixelFormat::BLOB);
  sessionContext.initializeStream(blobStream);

  auto framebuffer = sessionContext.fetchOrCreateScratchEglFramebuffer(
      mEglDisplayContext.getEglDisplay(), kStreamId,
      Resolution(kVgaWidth, kVgaHeight));
  ASSERT_THAT(framebuffer, Not(IsNull()));

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));

  auto yuvStream = createStream(kSecondStreamId, kVgaWidth, kVgaHeight,
                                PixelFormat::YCBCR_420_888);

  sessionContext.removeStreamsNotInStreamConfiguration(
      StreamConfiguration{.streams = {blobStream, yuvStream}});

  EXPECT_TRUE(sessionContext.hasCachedScratchEglFramebuffer(
      kStreamId, Resolution(kVgaWidth, kVgaHeight)));
}

}  // namespace
}  // namespace virtualcamera
}  // namespace companion
}  // namespace android

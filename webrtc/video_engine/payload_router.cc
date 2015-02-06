/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/video_engine/payload_router.h"

#include "webrtc/base/checks.h"
#include "webrtc/modules/rtp_rtcp/interface/rtp_rtcp.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

namespace webrtc {

PayloadRouter::PayloadRouter()
    : crit_(CriticalSectionWrapper::CreateCriticalSection()),
      active_(false) {}

PayloadRouter::~PayloadRouter() {}

void PayloadRouter::SetSendingRtpModules(
    const std::list<RtpRtcp*>& rtp_modules) {
  CriticalSectionScoped cs(crit_.get());
  rtp_modules_.clear();
  rtp_modules_.reserve(rtp_modules.size());
  for (auto* rtp_module : rtp_modules) {
    rtp_modules_.push_back(rtp_module);
  }
}

void PayloadRouter::set_active(bool active) {
  CriticalSectionScoped cs(crit_.get());
  active_ = active;
}

bool PayloadRouter::active() {
  CriticalSectionScoped cs(crit_.get());
  return active_;
}

bool PayloadRouter::RoutePayload(FrameType frame_type,
                                 int8_t payload_type,
                                 uint32_t time_stamp,
                                 int64_t capture_time_ms,
                                 const uint8_t* payload_data,
                                 size_t payload_size,
                                 const RTPFragmentationHeader* fragmentation,
                                 const RTPVideoHeader* rtp_video_hdr) {
  CriticalSectionScoped cs(crit_.get());
  DCHECK(rtp_video_hdr == NULL ||
         rtp_video_hdr->simulcastIdx <= rtp_modules_.size());

  if (!active_ || rtp_modules_.empty())
    return false;

  int stream_idx = 0;
  if (rtp_video_hdr != NULL)
    stream_idx = rtp_video_hdr->simulcastIdx;
  return rtp_modules_[stream_idx]->SendOutgoingData(
      frame_type, payload_type, time_stamp, capture_time_ms, payload_data,
      payload_size, fragmentation, rtp_video_hdr) == 0 ? true : false;
}

}  // namespace webrtc

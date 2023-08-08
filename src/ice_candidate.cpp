//
//  ice_candidate.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "ice_candidate.h"
#include "base.h"

const webrtc::IceCandidateInterface* from_c(RTCIceCandidate* ice_candidate)
{
	int index = ice_candidate->sdp_mline_index;
	const std::string mid = std::string(ice_candidate->sdp_mid);
	const std::string candidate = std::string(ice_candidate->candidate);
	return webrtc::CreateIceCandidate(mid, index, candidate, nullptr);
}

void free_ice_candidate(RTCIceCandidate* candidate)
{
	free_incomplete_ptr(candidate->candidate);
	free_incomplete_ptr(candidate->sdp_mid);
	delete candidate;
}

RTCIceCandidate* into_c(webrtc::IceCandidateInterface* candidate)
{
	auto c_candidate = new RTCIceCandidate;

	auto sdp_mid = candidate->sdp_mid();
	c_candidate->sdp_mid = copy_c_str(sdp_mid);
	if (!c_candidate->sdp_mid)
	{
		free_ice_candidate(c_candidate);
		return nullptr;
	}

	std::string _candidate;
	candidate->ToString(&_candidate);
	c_candidate->candidate = copy_c_str(_candidate);
	if (!c_candidate->candidate)
	{
		free_ice_candidate(c_candidate);
		return nullptr;
	}

	c_candidate->sdp_mline_index = candidate->sdp_mline_index();
	return c_candidate;
}

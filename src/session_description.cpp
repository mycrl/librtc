//
//  session_description.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "session_description.h"
#include "base.h"

webrtc::SdpType from_c(RTCSessionDescriptionType type)
{
	if (type == RTCSessionDescriptionTypeAnswer)
	{
		return webrtc::SdpType::kAnswer;
	}
	else if (type == RTCSessionDescriptionTypeOffer)
	{
		return webrtc::SdpType::kOffer;
	}
	else if (type == RTCSessionDescriptionTypePrAnswer)
	{
		return webrtc::SdpType::kPrAnswer;
	}
	else
	{
		return webrtc::SdpType::kRollback;
	}
}

std::unique_ptr<webrtc::SessionDescriptionInterface> from_c(RTCSessionDescription* desc)
{
	webrtc::SdpType type = from_c(desc->type);
	const std::string sdp = std::string((char*)desc->sdp);
	return webrtc::CreateSessionDescription(type, sdp);
}

void free_session_description(RTCSessionDescription* desc)
{
	free_incomplete_ptr((void*)desc->sdp);
	free_incomplete_ptr(desc);
}

RTCSessionDescription* into_c(webrtc::SessionDescriptionInterface* desc)
{
	auto c_desc = (RTCSessionDescription*)malloc(sizeof(RTCSessionDescription));
	if (!c_desc)
	{
		return nullptr;
	}

	std::string sdp;
	desc->ToString(&sdp);
	c_desc->sdp = copy_c_str(sdp);
	if (!c_desc->sdp)
	{
		free_session_description(c_desc);
		return nullptr;
	}

	c_desc->type = (RTCSessionDescriptionType)(desc->GetType());
	return c_desc;
}

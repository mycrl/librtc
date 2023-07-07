//
//  data_channel.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_DATA_CHANNEL_H
#define LIBRTC_DATA_CHANNEL_H
#pragma once

#include "api/peer_connection_interface.h"
#include "rtc.h"
#include "base.h"

class IDataChannel
	: public webrtc::DataChannelObserver
	, public rtc::RefCountInterface
{
public:
	IDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);
	static IDataChannel* From(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);
	void Send(uint8_t* buf, int size);
	void OnStateChange();
	void OnMessage(const webrtc::DataBuffer& buffer);
	void OnDataMessage(void* ctx, void(*handler)(void* _ctx, uint8_t* buf, uint64_t size));
	void RemoveOnMessage();

	DataState state;
private:
	rtc::scoped_refptr<webrtc::DataChannelInterface> _channel;
	void(*_handler)(void* ctx, uint8_t* buf, uint64_t size);
	void* _ctx;
};

webrtc::DataChannelInit* from_c(DataChannelOptions* options);
RTCDataChannel* create_data_channel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);

#endif  // LIBRTC_DATA_CHANNEL_H

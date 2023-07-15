//
//  data_channel.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "data_channel.h"
#include "base.h"

void rtc_free_data_channel(RTCDataChannel* channel)
{
	free_incomplete_ptr(channel->label);
	free_incomplete_ptr(channel);
}

IDataChannel::IDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
{
	_ctx = NULL;
	_handler = NULL;
	_channel = data_channel;
	state = DataState::DataStateConnecting;
}

IDataChannel* IDataChannel::From(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
{
	auto self = new rtc::RefCountedObject<IDataChannel>(data_channel);
	self->AddRef();
	return self;
}

void IDataChannel::Send(uint8_t* buf, int size)
{
	if (state != DataState::DataStateOpen)
	{
		return;
	}

	_channel->Send(webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buf, size), true));
}

void IDataChannel::OnStateChange()
{
	state = (DataState)_channel->state();
}

void IDataChannel::OnMessage(const webrtc::DataBuffer& buffer)
{
	if (!_handler)
	{
		return;
	}

	auto data = buffer.data.data();
	auto size = buffer.data.size();
	if (size == 0)
	{
		return;
	}

	_handler(_ctx, (uint8_t*)data, size);
}

void IDataChannel::OnDataMessage(void* ctx,
								 void(*handler)(void* _ctx, uint8_t* buf, uint64_t size))
{
	_channel->RegisterObserver(this);
	_handler = handler;
	_ctx = ctx;
}

void IDataChannel::RemoveOnMessage()
{
	_channel->UnregisterObserver();
	_handler = NULL;
	_ctx = NULL;
}

webrtc::DataChannelInit* from_c(DataChannelOptions* options)
{
	webrtc::DataChannelInit* init = new webrtc::DataChannelInit();
	init->protocol = std::string(options->protocol);
	init->reliable = options->reliable;
	init->ordered = options->ordered;

	if (options->max_retransmit_time)
	{
		init->maxRetransmitTime = options->max_retransmit_time - 1;
	}
	else if (options->max_retransmits)
	{
		init->maxRetransmits = options->max_retransmits - 1;
	}

	if (init->priority)
	{
		init->priority = (webrtc::Priority)(options->priority - 1);
	}

	return init;
}

RTCDataChannel* create_data_channel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
{
	RTCDataChannel* channel = (RTCDataChannel*)malloc(sizeof(RTCDataChannel));
	if (!channel)
	{
		free_incomplete_ptr(channel);
		return NULL;
	}

	auto label = data_channel->label();
	channel->label = copy_c_str(label);
	if (!channel->label)
	{
		free_incomplete_ptr(channel);
		return NULL;
	}

	channel->channel = IDataChannel::From(data_channel);
	channel->remote = true;
	return channel;
}

void rtc_send_data_channel_msg(RTCDataChannel* channel, uint8_t* buf, int size)
{
	channel->channel->Send(buf, size);
}

void rtc_set_data_channel_msg_h(RTCDataChannel* channel,
								void(*handler)(void* ctx, uint8_t* buf, uint64_t size),
								void* ctx)
{
	channel->channel->OnDataMessage(ctx, handler);
}

void rtc_remove_data_channel_msg_h(RTCDataChannel* channel)
{
	channel->channel->RemoveOnMessage();
}

DataState rtc_get_data_channel_state(RTCDataChannel* channel)
{
	return channel->channel->state;
}

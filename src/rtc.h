//
//  base.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_RTC_H
#define LIBRTC_RTC_H
#pragma once

#include "api/scoped_refptr.h"
#include "api/peer_connection_interface.h"
#include "base.h"

#include <stdint.h>
#include <memory>
#include <optional>

// An enum describing the session description's type.
typedef enum
{
    // The session description object describes the initial proposal in an
    // offer/answer exchange. The session negotiation process begins with an offer
    // being sent from the caller to the callee.
    RTCSessionDescriptionTypeOffer,

    // Description must be treated as an SDP answer, but not a final answer.
    RTCSessionDescriptionTypePrAnswer,

    // The SDP contained in the sdp property is the definitive choice in the
    // exchange. In other words, this session description describes the agreed-upon
    // configuration, and is being sent to finalize negotiation.
    RTCSessionDescriptionTypeAnswer,

    // This special type with an empty session description is used to
    // roll back to the previous stable state.
    RTCSessionDescriptionTypeRollback,
} RTCSessionDescriptionType;

// The RTCSessionDescription interface describes one end of a connection or
// potential connection and how it's configured. Each RTCSessionDescription
// consists of a description type indicating which part of the offer/answer
// negotiation process it describes and of the SDP descriptor of the session.
typedef struct
{
    RTCSessionDescriptionType type;

    // A string containing the SDP describing the session.
    const char* sdp;
} RTCSessionDescription;

// Specifies how to handle negotiation of candidates when the remote peer is not
// compatible with the SDP BUNDLE standard. If the remote endpoint is BUNDLE-aware,
// all media tracks and data channels are bundled onto a single transport at the
// completion of negotiation, regardless of policy used, and any superfluous
// transports that were created initially are closed at that point.
//
// In technical terms, a BUNDLE lets all media flow between two peers flow across
// a single 5-tuple; that is, from a single IP and port on one peer to a single IP
// and port on the other peer, using the same transport protocol.
typedef enum
{
    // The ICE agent initially creates one RTCDtlsTransport for each type of
    // content added: audio, video, and data channels. If the remote endpoint is
    // not BUNDLE-aware, then each of these DTLS transports handles all the
    // communication for one type of data.
    BundelPolicyBalanced = 1,

    // The ICE agent initially creates one RTCDtlsTransport per media track and a
    // separate one for data channels. If the remote endpoint is not BUNDLE-aware,
    // everything is negotiated on these separate DTLS transports.
    BundelPolicyMaxCompat,

    // The ICE agent initially creates only a single RTCDtlsTransport to carry all
    // of the RTCPeerConnection's data. If the remote endpoint is not BUNDLE-aware,
    // then only a single track will be negotiated and the rest ignored.
    BundelPolicyMaxBundle,
} BundelPolicy;


// The current ICE transport policy; if the policy isn't specified, all is assumed
// by default, allowing all candidates to be considered. Possible values are:
typedef enum
{
    IceTransportPolicyNone = 1,
    // Only ICE candidates whose IP addresses are being relayed, such as those
    // being passed through a STUN or TURN server, will be considered.
    IceTransportPolicyRelay,

    // Only ICE candidates with public IP addresses will be considered.
    // Removed from the specification's May 13, 2016 working draft.
    IceTransportPolicyPublic,

    // All ICE candidates will be considered.
    IceTransportPolicyAll,
} IceTransportPolicy;


// The RTCP mux policy to use when gathering ICE candidates,
// in order to support non-multiplexed RTCP.
// Possible values are:
typedef enum
{
    // Instructs the ICE agent to gather both RTP and RTCP candidates.
    // If the remote peer can multiplex RTCP,
    // then RTCP candidates are multiplexed atop the corresponding RTP candidates.
    // Otherwise, both the RTP and RTCP candidates are returned, separately.
    RtcpMuxPolicyNegotiate = 1,

    // Tells the ICE agent to gather ICE candidates for only RTP,
    // and to multiplex RTCP atop them. If the remote peer doesn't support RTCP
    // multiplexing, then session negotiation fails. This is the default value.
    RtcpMuxPolicyRequire,
} RtcpMuxPolicy;


// RTCIceServer
//
// An array of RTCIceServer objects, each describing one server which may be used
// by the ICE agent; these are typically STUN and/or TURN servers.
// If this isn't specified, the connection attempt will be made with no STUN or
// TURN server available, which limits the connection to local peers.
typedef struct
{
    // The credential to use when logging into the server.
    // This is only used if the RTCIceServer represents a TURN server.
    char* credential;

    // This required property is either a single string or an array of strings,
    // each specifying a URL which can be used to connect to the server.
    char** urls;
    int urls_size;
    int urls_capacity;

    // If the RTCIceServer is a TURN server, then this is the username to use
    // during the authentication process.
    char* username;
} RTCIceServer;


// RTCPeerConnection
//
// The RTCPeerConnection is a newly-created RTCPeerConnection,
// which represents a connection between the local device and a remote peer.
typedef struct
{
    BundelPolicy bundle_policy;
    IceTransportPolicy ice_transport_policy;

    // TODO: not impl
    // A string which specifies the target peer identity for the RTCPeerConnection.
    // If this value is set (it defaults to null), the RTCPeerConnection will not
    // connect to a remote peer unless it can successfully authenticate with the
    // given name.
    char* peer_identity;
    RtcpMuxPolicy rtcp_mux_policy;
    RTCIceServer* ice_servers;
    int ice_servers_size;
    int ice_servers_capacity;

    // An unsigned 16-bit integer value which specifies the size of the prefetched
    // ICE candidate pool.
    // The default value is 0 (meaning no candidate prefetching will occur).
    // You may find in some cases that connections can be established more quickly
    // by allowing the ICE agent to start fetching ICE candidates before you start
    // trying to connect, so that they're already available for inspection when
    // RTCPeerConnection.setLocalDescription() is called.
    int ice_candidate_pool_size;
} RTCPeerConnectionConfigure;

typedef enum
{
    PeerConnectionStateNew,
    PeerConnectionStateConnecting,
    PeerConnectionStateConnected,
    PeerConnectionStateDisconnected,
    PeerConnectionStateClose,
    PeerConnectionStateFailed,
} PeerConnectionState;

typedef enum
{
    SignalingStateStable,
    SignalingStateHaveLocalOffer,
    SignalingStateHaveLocalPrAnswer,
    SignalingStateHaveRemoteOffer,
    SignalingStateHaveRemotePrAnswer,
    SignalingStateClosed,
} SignalingState;

typedef enum
{
    IceGatheringStateNew,
    IceGatheringStateGathering,
    IceGatheringStateComplete,
} IceGatheringState;

typedef enum
{
    IceConnectionStateNew,
    IceConnectionStateChecking,
    IceConnectionStateConnected,
    IceConnectionStateCompleted,
    IceConnectionStateFailed,
    IceConnectionStateDisconnected,
    IceConnectionStateClosed,
    IceConnectionStateMax,
} IceConnectionState;

typedef enum
{
    MediaStreamTrackKindVideo,
    MediaStreamTrackKindAudio,
} MediaStreamTrackKind;

class IVideoTrackSource;
class IVideoTrackSink;
class IAudioTrackSource;
class IAudioTrackSink;

// MediaStreamTrack
//
// The MediaStreamTrack interface represents a single media track within a stream;
// typically, these are audio or video tracks, but other track types may exist as
// well.
typedef struct
{
    // Returns a string set to "audio" if the track is an audio track and to
    // "video", if it is a video track. It doesn't change if the track is
    // disassociated from its source.
    MediaStreamTrackKind kind;

    // Returns a string containing a user agent-assigned label that identifies the
    // track source, as in "internal microphone". The string may be left empty and
    // is empty as long as no source has been connected. When the track is
    // disassociated from its source, the label is not changed.
    char* label;
    std::optional<rtc::scoped_refptr<webrtc::RtpSenderInterface>> sender = std::nullopt;

    /* --------------- video --------------- */
    IVideoTrackSource* video_source;
    IVideoTrackSink* video_sink;

    /* --------------- audio --------------- */
    IAudioTrackSource* audio_source;
    IAudioTrackSink* audio_sink;
} MediaStreamTrack;

// RTCIceCandidate
//
// The RTCIceCandidate interface¡ªpart of the WebRTC API¡ªrepresents a candidate
// Interactive Connectivity Establishment (ICE) configuration which may be used to
// establish an RTCPeerConnection.
//
// An ICE candidate describes the protocols and routing needed for WebRTC to be
// able to communicate with a remote device. When starting a WebRTC peer connection,
// typically a number of candidates are proposed by each end of the connection,
// until they mutually agree upon one which describes the connection they decide
// will be best. WebRTC then uses that candidate's details to initiate the
// connection.
//
// For details on how the ICE process works, see Lifetime of a WebRTC session.
// The article WebRTC connectivity provides additional useful details.
typedef struct
{
    //A string describing the properties of the candidate, taken directly from the
    //SDP attribute "candidate". The candidate string specifies the network
    //connectivity information for the candidate. If the candidate is an empty
    //string (""), the end of the candidate list has been reached; this candidate
    //is known as the "end-of-candidates" marker.
    char* candidate;

    // A string containing the identification tag of the media stream with which
    // the candidate is associated, or null if there is no associated media stream.
    // The default is null.
    char* sdp_mid;

    // A number property containing the zero-based index of the m-line with which
    // the candidate is associated, within the SDP of the media description, or
    // null if no such associated exists. The default is null.
    int sdp_mline_index;
} RTCIceCandidate;

// YUV420p
typedef struct
{
    bool remote;
    uint32_t width;
    uint32_t height;
    int64_t timestamp;
    const uint8_t* planes[4];
    uint32_t strides[4];
} IVideoFrame;

// PCM
typedef struct
{
    bool remote;
    size_t size;
    size_t frames;
    size_t channels;
    int sample_rate;
    int64_t timestamp;
    const int16_t* data;
} IAudioFrame;

typedef enum
{
    DataStateConnecting,
    DataStateOpen,
    DataStateClosing,
    DataStateClosed
} DataState;

typedef enum
{
    PriorityVeryLow = 1,
    PriorityLow,
    PriorityMedium,
    PriorityHigh,
} Priority;

typedef struct
{
    // Deprecated. Reliability is assumed, and channel will be unreliable if
    // maxRetransmitTime or MaxRetransmits is set.
    bool reliable;

    // True if ordered delivery is required.
    bool ordered;

    // The max period of time in milliseconds in which retransmissions will be
    // sent. After this time, no more retransmissions will be sent.
    //
    // Cannot be set along with `maxRetransmits`.
    // This is called `maxPacketLifeTime` in the WebRTC JS API.
    // Negative values are ignored, and positive values are clamped to [0-65535]
    uint64_t max_retransmit_time;

    // The max number of retransmissions.
    //
    // Cannot be set along with `maxRetransmitTime`.
    // Negative values are ignored, and positive values are clamped to [0-65535]
    uint64_t max_retransmits;

    // This is set by the application and opaque to the WebRTC implementation.
    char* protocol;

    // True if the channel has been externally negotiated and we do not send an
    // in-band signalling in the form of an "open" message. If this is true, `id`
    // below must be set; otherwise it should be unset and will be negotiated
    // in-band.

    bool negotiated;
    // The stream id, or SID, for SCTP data channels. -1 if unset (see above).
    int id;
    Priority priority;
} DataChannelOptions;

class IDataChannel;

// RTCDataChannel
//
// The RTCDataChannel interface represents a network channel which can be used for
// bidirectional peer-to-peer transfers of arbitrary data. Every data channel is
// associated with an RTCPeerConnection, and each peer connection can have up to a
// theoretical maximum of 65,534 data
// channels (the actual limit may vary from browser to browser).
typedef struct
{
    char* label;
    IDataChannel* channel;
    bool remote;
} RTCDataChannel;

typedef struct
{
    // A signalingstatechange event is sent to an RTCPeerConnection to notify it that 
    // its signaling state, as indicated by the signalingState property, has changed.
    void (*on_signaling_change)(void* ctx, SignalingState state);
    // A datachannel event is sent to an RTCPeerConnection instance when an RTCDataChannel 
    // has been added to the connection, as a result of the remote peer calling 
    // create_datachannel.
    void (*on_datachannel)(void* ctx, RTCDataChannel* channel);
    // The icegatheringstatechange event is sent to the onicegatheringstatechange 
    // event handler on an RTCPeerConnection when the state of the ICE candidate gathering 
    // process changes.This signifies that the value of the connection's iceGatheringState 
    // property has changed.
    //
    // When ICE first starts to gather connection candidates, the value changes from new 
    // to gathering to indicate that the process of collecting candidate configurations for 
    // the connection has begun.When the value changes to complete, all of the transports 
    // that make up the RTCPeerConnection have finished gathering ICE candidates.
    void (*on_ice_gathering_change)(void* ctx, IceGatheringState state);
    // An icecandidate event is sent to an RTCPeerConnection when an RTCIceCandidate has 
    // been identified and added to the local peer by a call to set_local_description(). 
    // The event handler should transmit the candidate to the remote peer over the signaling 
    // channel so the remote peer can add it to its set of remote candidates.
    void (*on_ice_candidate)(void* ctx, RTCIceCandidate* candidate);
    // A negotiationneeded event is sent to the RTCPeerConnection when negotiation of the 
    // connection through the signaling channel is required. This occurs both during the 
    // initial setup of the connection as well as any time a change to the communication 
    // environment requires reconfiguring the connection.
    void (*on_renegotiation_needed)(void* ctx);
    // An iceconnectionstatechange event is sent to an RTCPeerConnection object each time 
    // the ICE connection state changes during the negotiation process.
    void (*on_ice_connection_change)(void* ctx, IceConnectionState state);
    // The track event is sent to the ontrack event handler on RTCPeerConnections after a 
    // new track has been added to an RTCRtpReceiver which is part of the connection.
    void (*on_track)(void* ctx, MediaStreamTrack* track);
    // The connectionstatechange event is sent to the onconnectionstatechange event handler 
    // on an RTCPeerConnection object after a new track has been added to an RTCRtpReceiver 
    // which is part of the connection. The new connection state can be found in connectionState, 
    // and is one of the string values: new, connecting, connected, disconnected, failed, or closed.
    void (*on_connection_change)(void* ctx, PeerConnectionState state);
} Events;

class RtcThreads;

// RTCPeerConnection
//
// The RTCPeerConnection interface represents a WebRTC connection between the local
// computer and a remote peer. It provides methods to connect to a remote peer,
// maintain and monitor the connection, and close the connection once it's no
// longer needed.
typedef struct
{
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory;
    std::unique_ptr<RtcThreads> threads;
} RTCPeerConnection;

// Returns a newly-created RTCPeerConnection, which represents a
// connection between the local device and a remote peer.
extern "C" EXPORT RTCPeerConnection * rtc_create_peer_connection(RTCPeerConnectionConfigure * config,
                                                                 Events * events,
                                                                 void* ctx);

// The RTCPeerConnection.close() method closes the current peer connection.
//
// Calling this method terminates the RTCPeerConnection's ICE agent, ending any
// ongoing ICE processing and any active streams. This also releases any resources
// in use by the ICE agent, including TURN permissions. All RTCRtpSender objects
// are considered to be stopped once this returns (they may still be in the process
// of stopping, but for all intents and purposes, they're stopped).
extern "C" EXPORT void rtc_close(RTCPeerConnection * peer);

// When a web site or app using RTCPeerConnection receives a new ICE candidate from
// the remote peer over its signaling channel, it delivers the newly-received
// candidate to the browser's ICE agent by calling RTCPeerConnection.addIceCandidate().
// This adds this new remote candidate to the RTCPeerConnection's remote
// description, which describes the state of the remote end of the connection.
//
// If the candidate parameter is missing or a value of null is given when calling
// addIceCandidate(), the added ICE candidate is an "end-of-candidates" indicator.
// The same is the case if the value of the specified object's candidate is either
// missing or an empty string (""), it signals that all remote candidates have been
// delivered.
//
// The end-of-candidates notification is transmitted to the remote peer using a
// candidate with an a-line value of end-of-candidates.
//
// During negotiation, your app will likely receive many candidates which you'll
// deliver to the ICE agent in this way, allowing it to build up a list of
// potential connection methods. This is covered in more detail in the articles
// WebRTC connectivity and Signaling and video calling.
extern "C" EXPORT bool rtc_add_ice_candidate(RTCPeerConnection * peer,
                                             RTCIceCandidate * icecandidate);

typedef void (*CreateDescCallback)(const char* error,
                                   RTCSessionDescription* desc,
                                   void* ctx);

// The createAnswer() method on the RTCPeerConnection interface creates an SDP
// answer to an offer received from a remote peer during the offer/answer
// negotiation of a WebRTC connection. The answer contains information about any
// media already attached to the session, codecs and options supported by the
// browser, and any ICE candidates already gathered. The answer is delivered to the
// returned Promise, and should then be sent to the source of the offer to continue
// the negotiation process.
extern "C" EXPORT void rtc_create_answer(RTCPeerConnection * peer,
                                         CreateDescCallback callback,
                                         void* ctx);

// The createOffer() method of the RTCPeerConnection interface initiates the
// creation of an SDP offer for the purpose of starting a new WebRTC connection to
// a remote peer. The SDP offer includes information about any MediaStreamTrack
// objects already attached to the WebRTC session, codec, and options supported by
// the browser, and any candidates already gathered by the ICE agent, for the
// purpose of being sent over the signaling channel to a potential peer to request
// a connection or to update the configuration of an existing connection.
extern "C" EXPORT void rtc_create_offer(RTCPeerConnection * peer,
                                        CreateDescCallback callback,
                                        void* ctx);

typedef void (*SetDescCallback)(const char* error, void* ctx);

// The RTCPeerConnection method setLocalDescription() changes the local description
// associated with the connection. This description specifies the properties of the
// local end of the connection, including the media format. The method takes a
// single parameter¡ªthe session description¡ªand it returns a Promise which is
// fulfilled once the description has been changed, asynchronously.
//
// If setLocalDescription() is called while a connection is already in place, it
// means renegotiation is underway (possibly to adapt to changing network conditions).
// Because descriptions will be exchanged until the two peers agree on a
// configuration, the description submitted by calling setLocalDescription() does
// not immediately take effect. Instead, the current connection configuration
// remains in place until negotiation is complete. Only then does the agreed-upon
// configuration take effect.
extern "C" EXPORT void rtc_set_local_description(RTCPeerConnection * peer,
                                                 RTCSessionDescription * desc,
                                                 SetDescCallback callback,
                                                 void* ctx);

// The RTCPeerConnection method setRemoteDescription() sets the specified session
// description as the remote peer's current offer or answer. The description
// specifies the properties of the remote end of the connection, including the
// media format. The method takes a single parameter¡ªthe session description and
// it returns a Promise which is fulfilled once the description has been changed,
// asynchronously.
//
// This is typically called after receiving an offer or answer from another peer
// over the signaling server. Keep in mind that if setRemoteDescription() is called
// while a connection is already in place, it means renegotiation is underway
// (possibly to adapt to changing network conditions).
//
// Because descriptions will be exchanged until the two peers agree on a
// configuration, the description submitted by calling setRemoteDescription() does
// not immediately take effect. Instead, the current connection configuration
// remains in place until negotiation is complete. Only then does the agreed-upon
// configuration take effect.
extern "C" EXPORT void rtc_set_remote_description(RTCPeerConnection * peer,
                                                  RTCSessionDescription * desc,
                                                  SetDescCallback callback,
                                                  void* ctx);

// The RTCPeerConnection method addTrack() adds a new media track to the set of
// tracks which will be transmitted to the other peer.
extern "C" EXPORT int rtc_add_media_stream_track(RTCPeerConnection * rtc,
                                                 MediaStreamTrack * track,
                                                 char* stream_id);

// The `rtc_remove_media_stream_track` method tells the local end of the connection 
// to stop sending media from the specified track, without actually removing the 
// corresponding RTCRtpSender from the list of senders as reported by `senders`. 
// 
// If the track is already stopped, or is not in the connection's senders list, 
// this method has no effect.
extern "C" EXPORT int rtc_remove_media_stream_track(RTCPeerConnection * rtc,
                                                    MediaStreamTrack * track);

extern "C" EXPORT RTCDataChannel * rtc_create_data_channel(RTCPeerConnection * rtc,
                                                           char* label,
                                                           DataChannelOptions * options);


extern "C" EXPORT void rtc_set_data_channel_msg_h(RTCDataChannel * channel,
                                                  void(*handler)(void* ctx, uint8_t * buf, uint64_t size),
                                                  void* ctx);

extern "C" EXPORT void rtc_remove_data_channel_msg_h(RTCDataChannel * channel);
extern "C" EXPORT void rtc_send_data_channel_msg(RTCDataChannel * channel, uint8_t * buf, int size);
extern "C" EXPORT DataState rtc_get_data_channel_state(RTCDataChannel * channel);
extern "C" EXPORT void rtc_free_data_channel(RTCDataChannel * channel);
extern "C" EXPORT void rtc_free_frame(void* frame);

extern "C" EXPORT void rtc_free_media_stream_track(MediaStreamTrack * track);
extern "C" EXPORT void rtc_remove_media_stream_track_frame_h(MediaStreamTrack * track);

// video track

extern "C" EXPORT MediaStreamTrack * rtc_create_video_track(char* label);
extern "C" EXPORT void rtc_set_video_track_frame_h(MediaStreamTrack * track,
                                                   void(handler)(void* ctx, IVideoFrame * frame),
                                                   void* ctx);
extern "C" EXPORT void rtc_add_video_track_frame(MediaStreamTrack * track,
                                                 IVideoFrame * frame);

// audio track

extern "C" EXPORT MediaStreamTrack * rtc_create_audio_track(char* label);
extern "C" EXPORT void rtc_set_audio_track_frame_h(MediaStreamTrack * track,
                                                   void(handler)(void* ctx, IAudioFrame * frame),
                                                   void* ctx);
extern "C" EXPORT void rtc_add_audio_track_frame(MediaStreamTrack * track,
                                                 IAudioFrame * frame);

#endif  // LIBRTC_RTC_H
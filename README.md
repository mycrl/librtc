<!--lint disable no-literal-urls-->
<div align="center">
  <h1>libRTC</h1>
</div>
<br/>
<div align="center">
  <strong>C bindings for Google Chrome WebRTC Native</strong><br/>
  <sup>current version: [M110/5481](https://webrtc.googlesource.com/src.git/+log/refs/branch-heads/5481)</sup>
</div>
<div align="center">
  <img src="https://img.shields.io/github/actions/workflow/status/mycrl/librtc/test.yml?branch=main"/>
  <img src="https://img.shields.io/github/license/mycrl/librtc"/>
  <img src="https://img.shields.io/github/issues/mycrl/librtc"/>
  <img src="https://img.shields.io/github/stars/mycrl/librtc"/>
</div>
<br/>
<br/>

C bindings for Google Chrome WebRTC Native. Using C++ ABI in other programming languages is a very difficult thing, but most languages support interacting with stable C ABI, the meaning of this project is just that, abstract some commonly used structures and methods, and make other projects embed WebRTC Native is easier.


## Features

- Basic PeerConection session.
- Custom video track and sink. <sup>(`YUV420 format`)</sup>
- Custom Audio track and sink. <sup>(`PCM format`)</sup>
- DataChannel create and sink.
- Hardware codec support (Nvenc, Qsv, VideoToolBox).


### License
[GPL](./LICENSE) Copyright (c) 2022 Mr.Panda.

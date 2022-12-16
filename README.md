# batrachiatc

Google WebRTC native export c abi.

### Build

#### Install depot_tools

Clone the depot_tools repository:

```bash
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```

Add depot_tools to the front of your PATH.

Compile with native toolchain (only Windows):

```bash
$env:DEPOT_TOOLS_WIN_TOOLCHAIN=0
```

#### Get google webrtc native source code

Clone the batrachia repository:

```bash
git clone https://github.com/mycrl/batrachiatc
```

Create a third party directory, enter it, and run fetch webrtc:

```bash
cd batrachiatc
mkdir third_party
cd third_party
mkdir webrtc
cd webrtc
fetch --nohooks webrtc
cd src
```

Switch to the M99 branch (only Windows):

```bash
git checkout branch-heads/4844
```

If it is linux/mac, please use the M105 branch, because M99 does not support the M1 chip version of macos:

```bash
git checkout branch-heads/5195
```

Sync webrtc toolchain and many dependencies.
The checkout size is large due the use of the Chromium build toolchain and many dependencies. Estimated size:
* Linux: 6.4 GB.
* Linux (with Android): 16 GB (of which ~8 GB is Android SDK+NDK images).
* Mac (with iOS support): 5.6GB

```bash
gclient sync
```

#### Build webrtc library

Compile the Debug target of the webrtc static library:

```bash
gn gen out/Debug --args="is_debug=true is_component_build=false use_lld=false treat_warnings_as_errors=false use_rtti=true rtc_include_tests=false rtc_build_examples=false enable_iterator_debugging=true use_custom_libcxx=false"
ninja -C out/Debug
```

Compile the Release target of the webrtc static library:

```bash
gn gen out/Release --args="is_debug=false is_component_build=false use_lld=false treat_warnings_as_errors=false use_rtti=true rtc_include_tests=false rtc_build_examples=false use_custom_libcxx=false"
ninja -C out/Release
```

#### Build batrachiatc library

Go back to the root directory of batrachia:

```bash
cd batrachiatc
mkdir out
cd out
```

generate the batrachiatc static library project:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug # debug
cmake .. -DCMAKE_BUILD_TYPE=Release # release
cmake --build .
```


### License
[GPL](./LICENSE) Copyright (c) 2022 Mr.Panda.

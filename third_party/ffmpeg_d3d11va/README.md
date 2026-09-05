# RLink FFmpeg H.264 runtime

RLink dynamically loads this FFmpeg runtime for H.264 D3D11VA decoding,
libx264 software encoding, and the available QSV/NVENC/AMF hardware encoders.
It is kept separate from the FFmpeg copy embedded in `webrtc.lib`.

The build script passes `--enable-gpl --enable-libx264`. Consequently, this
FFmpeg build is distributed under **GNU GPL version 2 or later**, rather than
the default FFmpeg LGPL terms. RLink distributes it under the GPL version 3
option. No `--enable-nonfree` component is enabled.

Rebuild it from the WebRTC checkout used by this repository:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\Build-FfmpegD3D11Va.ps1
```

Runtime files copied beside `RLinkAPP.exe`:

- `avcodec-62.dll`
- `avutil-60.dll`
- `libwinpthread-1.dll`
- `libx264-165.dll`
- optional hardware-dispatch runtimes detected by the build script

The applicable GPL v3 text is stored in `COPYING.GPLv3`. The LGPL v2.1 text is
retained for reference because it covers FFmpeg builds made without GPL
components; it does not describe the bundled libx264-enabled DLL set.

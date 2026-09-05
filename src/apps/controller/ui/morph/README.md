# MorphIconCore

`MorphIconCore` is a Qt/C++20 port of the geometry core in
[Morphicons](https://github.com/guillermolg00/morphicons), based on upstream
version 1.7.1 at commit `38d2a7221633a453eeafebd872ee3649b9274b22`.

The port keeps the original algorithmic stages:

1. parse SVG path commands and supported primitives;
2. normalize lines, quadratics, arcs, circles, ellipses and rounded rectangles
   to cubic Bezier paths;
3. resample each subpath by arc length with exact corner anchors;
4. match subpaths and closed-loop correspondences;
5. compute 2D Procrustes rotation/scale alignment;
6. interpolate in polar similarity space with global block transport;
7. advance transitions using the same semi-implicit spring model.

RemoteC paints the interpolated point paths directly with `QPainter`, so no
DOM, JavaScript runtime, generated SVG string or per-frame XML parsing is
required. Source/target plans are cached globally by resource pair and sample
count, so repeated device cards reuse the first calculation; each control then
keeps that plan for subsequent frames. If parsing or planning fails,
the existing static `QIcon` path remains the fallback.

The upstream MIT license is deployed as
`licenses/icons/Morphicons-LICENSE.txt` with Release builds.

OpenSWR-Mesa
============

Overview
--------

This is repository of the integration work combining the high
performance, highly scalable core SWR rasterizer with Mesa.  A more
complete introduction and discussion towards upstreaming to the Mesa
project can be found on the mesa-dev mailing list.

Notes
-----

* SWR is set as the default software renderer.  Use
GALLIUM_DRIVER=llvmpipe to switch to Mesa's standard rasterizer.  This
particular change is to make it easier for people evaluating OpenSWR,
and will not be upstreamed.

* LLVM-3.6 is required.

* To build SWR with autoconf, include the following in the config
line: "--with-gallium-drivers=swr --enable-swr-native".

* Build defaults to AVX2; for a version to run on AVX build with
  "--with-swr-arch=AVX".

* To build SWR with SCons, nothing needs to be done - it is built by
  default.

* Code for the driver is in src/gallium/drivers/swr

* Code for the rasterizer is in src/gallium/drivers/swr/rasterizer

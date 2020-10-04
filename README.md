# What is qpxtool?

Please see the [official website](https://qpxtool.sourceforge.io/)

# Unofficial version

This unofficial version adds the following changes from upstream **v0.8.0**:

- Fix compilation under mingw-w64 (courtesy of [Eli Bildirici](https://sourceforge.net/u/bilditup1/), [view patch](https://sourceforge.net/p/qpxtool/patches/6/))
- Remove HL-DT-ST drives from LiteOn plugin blacklist, to enable error correction scan with HL-DT-ST WH16NS58 drive under firmware 1.V5, and probably others
- Fix LiteOn plugin for CD and DVD error scanning, was broken since upstream v0.7.2
- Fix layer counting and layer size on BD, closes [#3](https://github.com/speed47/qpxtool/issues/3), bits taken from [this patch](https://github.com/artkar0/qpxtool/commit/1213b3a4167246ff81bf008df4e4977a81fb54cb)
- Fix erroneous detection of finalized BD-R as BD-ROM
- Fix read speed computation under high-speed BD drives (`gettimeofday` resolution was not enough, replaced with `clock_gettime`), closes [#1](https://github.com/speed47/qpxtool/issues/1)
- Fix some drives having only the maximum read speed available, closes [#2](https://github.com/speed47/qpxtool/issues/2)
- Fix a few coding errors (thanks to compiler warnings)

# What is qpxtool?

Please see the [official website](https://qpxtool.sourceforge.io/)

# Unofficial version

This unofficial version adds the following changes from upstream **v0.8.0**:

- Fix compilation under mingw-w64 (courtesy of [Eli Bildirici](https://sourceforge.net/u/bilditup1/), [view patch](https://sourceforge.net/p/qpxtool/patches/6/))
- Remove HL-DT-ST drives from LiteOn plugin blacklist, to enable error correction scan with HL-DT-ST WH16NS58 drive under firmware 1.V5, and probably others
- Fix read speed computation under high-speed BD drives (`gettimeofday` resolution was not enough, replaced with `clock_gettime`), closes #1
- Fix some drives having only the maximum read speed available, closes #2
- Fix a few coding errors (thanks to compiler warnings)

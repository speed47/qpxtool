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
- Add a way to force the old LiteOn scanning way (by setting the `LITEON_FORCE_OLD=1` env var)
- Fix a few coding errors (thanks to compiler warnings)

# Screenshots

## Read speed andd error correction scan of a two-layer BD-ROM:
![bdr_dl_speed](https://user-images.githubusercontent.com/218502/95659865-59e77000-0b24-11eb-8dee-7425cf57a35d.PNG)
![bdr_dl_errc](https://user-images.githubusercontent.com/218502/95659867-5a800680-0b24-11eb-9648-1bd475631b26.PNG)
![bdr_dl_errc_detailed](https://user-images.githubusercontent.com/218502/95659866-5a800680-0b24-11eb-9897-7362fee8d9c6.PNG)

## Error correction scan of a DVD-R:
![dvdr_errc](https://user-images.githubusercontent.com/218502/95659864-594ed980-0b24-11eb-987c-b7657e40c5aa.PNG)
![dvdr_errc_detailed](https://user-images.githubusercontent.com/218502/95659868-5b189d00-0b24-11eb-8beb-724ee8b5f5c9.PNG)

## Error correction scan of a CD-RW:
![cd_errc](https://user-images.githubusercontent.com/218502/95660058-a5e6e480-0b25-11eb-891c-62326c7eacf0.PNG)
![cd_errc_detailed](https://user-images.githubusercontent.com/218502/95660056-a54e4e00-0b25-11eb-8600-cc867280c068.PNG)

Stlink V2/3 with original STM firmware as Blackmagic Debug Probes

Recent STM Stlink firmware revision ( V3 and V2 >= J32) expose nearly all
functionallity that BMP needs. This branch implements blackmagic debug probe
for the STM Stlink as a proof of concept. JTAG access is not yet implemented
and handling of Stlink3 V3 not yet tested. Use at your own risk, but report or
better fix problems.

Some sensible name is also needed, as BMP for libftdi already has taken the
name "blackmagic: for the executable.

CrosscCompling for windows with mingw has been tested.


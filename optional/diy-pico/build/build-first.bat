@echo on

setx PICO_SDK_PATH "..\..\pico-sdk"

cmake -DPICO_BOARD=pico -DPICO_PLATFORM=rp2040 -G "NMake Makefiles" ..

nmake

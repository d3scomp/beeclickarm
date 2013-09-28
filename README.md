beeclickarm
===========

A suite for 802.15.4 communication from Java. It offers basic API for broadcasting and receiving a packet. 

The suite consist of a Java (PC-host) library and C++ firmware for STM32F4-DISCOVERY (http://www.st.com/web/en/catalog/tools/FM116/SC959/SS1532/PF252419). The Java library communicates via virtual COM port with STM32F4-DISCOVERY which is connecte with the STM32F4 Discovery Shield (http://www.mikroe.com/stm32/stm32f4-discovery-shield/) to a BEE click module (http://www.mikroe.com/click/bee/ -- MRF24J40 802.15.4 adapter by Microchip).

The hardward is connected to PC via USB-cable plugged to USB-UART port on the Shield. The BEE click module is to be fitted in the mikroBUS 3 slot on the Shield.

The C++ firmware relies on Windows build chain.

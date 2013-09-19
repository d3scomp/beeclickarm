/*
 * MRF24J40.cpp
 *
 *  Created on: 14. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "MRF24J40.h"
#include "main.h"

MRF24J40::MRF24J40(PulseLED recvLed, PulseLED sendLed, uint32_t clkGPIO, uint32_t clkSPI, GPIO_TypeDef* gpioRST, GPIO_TypeDef* gpioCS, GPIO_TypeDef* gpioTXRX, SPI_TypeDef* spi, uint8_t afConfig,
		uint8_t pinSourceRST, uint8_t pinSourceCS, uint8_t pinSourceSCK, uint8_t pinSourceMISO, uint8_t pinSourceMOSI, uint32_t pinRST, uint32_t pinCS, uint32_t pinSCK, uint32_t pinMISO, uint32_t pinMOSI) :

		recvLed(recvLed), sendLed(sendLed), clkGPIO(clkGPIO), clkSPI(clkSPI), gpioRST(gpioRST), gpioCS(gpioCS), gpioTXRX(gpioTXRX), spi(spi), afConfig(afConfig),
		pinSourceRST(pinSourceRST), pinSourceCS(pinSourceCS), pinSourceSCK(pinSourceSCK), pinSourceMISO(pinSourceMISO), pinSourceMOSI(pinSourceMOSI),
		pinRST(pinRST), pinCS(pinCS), pinSCK(pinSCK), pinMISO(pinMISO), pinMOSI(pinMOSI) {
}

MRF24J40::~MRF24J40() {
}

void MRF24J40::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void MRF24J40::init() {
	// Enable the SPI clock
	RCC_APB1PeriphClockCmd(clkSPI, ENABLE);

	// Enable GPIO clocks
	RCC_AHB1PeriphClockCmd(clkGPIO, ENABLE);

	// Connect SPI pins to AF5
	GPIO_PinAFConfig(gpioTXRX, pinSourceSCK, afConfig); // alternative function SPIx_SCK
	GPIO_PinAFConfig(gpioTXRX, pinSourceMISO, afConfig); // alternative function SPIx_MISO
	GPIO_PinAFConfig(gpioTXRX, pinSourceMOSI, afConfig); // alternative function SPIx_MOSI

	// GPIO Deinitialisation
	GPIO_InitTypeDef gpioInitStruct;

	gpioInitStruct.GPIO_Pin = pinSCK | pinMISO | pinMOSI;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(gpioTXRX, &gpioInitStruct);

	gpioInitStruct.GPIO_Pin = pinCS;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(gpioCS, &gpioInitStruct);

	gpioInitStruct.GPIO_Pin = pinRST;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(gpioRST, &gpioInitStruct);

	// SPI configuration
	SPI_InitTypeDef  spiInitStruct;
	SPI_StructInit(&spiInitStruct);
	spiInitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spiInitStruct.SPI_Mode = SPI_Mode_Master;
	spiInitStruct.SPI_DataSize = SPI_DataSize_8b;
	spiInitStruct.SPI_CPOL = SPI_CPOL_Low;
	spiInitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	spiInitStruct.SPI_NSS = SPI_NSS_Soft;
	spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(spi, &spiInitStruct);

	// Default (idle) CS state
	GPIO_SetBits(gpioCS, pinCS);

	// Setting RESET# low creates the reset condition
	GPIO_ResetBits(gpioRST, pinRST);

	delayTimer.uDelay(1000);

	// Setting RESET# high removes the reset condition
	GPIO_SetBits(gpioRST, pinRST);

	delayTimer.uDelay(2000);


	SPI_Cmd(spi, ENABLE);

	/*
	 * We don't use interrupt for the SPI communication. However we keep it here just for case it is needed.

		// Configure the SPI interrupt priority
		NVIC_Init(&NVIC_InitStructure);
		NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  // This is the custom interrupt with the highest priority. Care has to be taken that it does not collide with todHandlerInterruptInit that has priority 1,0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	*/


	// 1. SOFTRST (0x2A) = 0x07 – Perform a software Reset. The bits will be automatically cleared to ‘0’ by hardware.
	writeShort(SOFTRST, 0x07);

	// 2. PACON2 (0x18) = 0x98 – Initialize FIFOEN = 1 and TXONTS = 0x6.
	writeShort(PACON2, 0x98);

	// 3. TXSTBL (0x2E) = 0x95 – Initialize RFSTBL = 0x9.
	writeShort(TXSTBL, 0x95);

	// 4. RFCON0 (0x200) = 0x03 – Initialize RFOPT = 0x03.
	writeLong(RFCON0, 0x03);

	// 5. RFCON1 (0x201) = 0x01 – Initialize VCOOPT = 0x02.
	writeLong(RFCON1, 0x01);

	// 6. RFCON2 (0x202) = 0x80 – Enable PLL (PLLEN = 1).
	writeLong(RFCON2, 0x80);

	// 7. RFCON6 (0x206) = 0x90 – Initialize TXFIL = 1 and 20MRECVR = 1.
	writeLong(RFCON6, 0x90);

	// 8. RFCON7 (0x207) = 0x80 – Initialize SLPCLKSEL = 0x2 (100 kHz Internal oscillator).
	writeLong(RFCON7, 0x80);

	// 9. RFCON8 (0x208) = 0x10 – Initialize RFVCO = 1.
	writeLong(RFCON8, 0x10);

	// 10. SLPCON1 (0x220) = 0x21 – Initialize CLKOUTEN = 1 and SLPCLKDIV = 0x01.
	writeLong(SLPCON1, 0x21);

	// Configuration for nonbeacon-enabled devices (see Section 3.8 “Beacon-Enabled and Nonbeacon-Enabled Networks”):
	// 11. BBREG2 (0x3A) = 0x80 – Set CCA mode to ED.
	writeShort(BBREG2, 0x80);

	// 12. CCAEDTH = 0x60 – Set CCA ED threshold.
	writeShort(CCAEDTH, 0x60);

	// 13. BBREG6 (0x3E) = 0x40 – Set appended RSSI value to RXFIFO.
	writeShort(BBREG6, 0x40);

	// 14. Enable interrupts – See Section 3.3 “Interrupts”.
	writeShort(INTCON, 0xF6); // RXIE and TXNIE interrupts enabled

	// 15. Set channel – See Section 3.4 “Channel Selection”.
	writeLong(RFCON0, 0x00 | 0x03); // Channel 11 .. 0x00 (up to 26 .. 0xF0), note that 0x03 must still be present to keep RFOPT = 0x03

	// 16. Set transmitter power - See “REGISTER 2-62: RF CONTROL 3 REGISTER (ADDRESS: 0x203)”.
	writeLong(RFCON3, 0x40); // -10dB

	// 17. RFCTL (0x36) = 0x04 – Reset RF state machine.
	writeShort(RFCTL, 0x04);

	// 18. RFCTL (0x36) = 0x00.
	writeShort(RFCTL, 0x00);

	// 19. Delay at least 192 μs.
	delayTimer.uDelay(200);


	// Configuring Nonbeacon-Enabled Device
	// 1.Clear the PANCOORD (RXMCR 0x00<3>) bit = 0 to configure as device.
	// + Set promiscuous Mode bit
	writeShort(RXMCR, 0x01);

	// 2.Clear the SLOTTED (TXMCR 0x11<5>) bit = 0 to use Unslotted CSMA-CA mode.
	writeShort(TXMCR, 0x1C);
}

void MRF24J40::writeShort(uint8_t addr, uint8_t value) {
	assert_param(addr < 0x40);

	gpioCS->BSRRH = pinCS;

	spi->DR = (addr << 1) | 0x01;

	while (!(spi->SR & SPI_I2S_FLAG_TXE));
	spi->DR = value;

	while (!(spi->SR & SPI_I2S_FLAG_TXE)); // BSY flag is set only after some delay, so it is compulsory to first wait for TXE, which ensures that BSY flag is set meanwhile
	while (spi->SR & SPI_I2S_FLAG_BSY);
	gpioCS->BSRRL = pinCS;
}

void MRF24J40::writeLong(uint16_t addr, uint8_t value) {
	assert_param(addr < 0x400);

	gpioCS->BSRRH = pinCS;

	spi->DR = (addr >> 3) | 0x80;

	while (!(spi->SR & SPI_I2S_FLAG_TXE));
	spi->DR = (addr << 5) | 0x10;

	while (!(spi->SR & SPI_I2S_FLAG_TXE));
	spi->DR = value;

	while (!(spi->SR & SPI_I2S_FLAG_TXE)); // BSY flag is set only after some delay, so it is compulsory to first wait for TXE, which ensures that BSY flag is set meanwhile
	while (spi->SR & SPI_I2S_FLAG_BSY);
	gpioCS->BSRRL = pinCS;
}

uint8_t MRF24J40::readShort(uint8_t addr) {
	assert_param(addr < 0x40);

	uint8_t result;

	gpioCS->BSRRH = pinCS;

	spi->DR; spi->SR; // Clear the OVR flag

	spi->DR = (addr << 1);

	while (!(spi->SR & SPI_I2S_FLAG_TXE));
	spi->DR = 0;

	while (!(spi->SR & SPI_I2S_FLAG_RXNE));
	spi->DR;

	while (!(spi->SR & SPI_I2S_FLAG_RXNE));
	result = spi->DR;

	gpioCS->BSRRL = pinCS;

	return result;
}

uint8_t MRF24J40::readLong(uint16_t addr) {
	assert_param(addr < 0x400);

	uint8_t result;

	gpioCS->BSRRH = pinCS;

	spi->DR; spi->SR; // Clear the OVR flag

	spi->DR = (addr >> 3) | 0x80;

	while (!(spi->SR & SPI_I2S_FLAG_TXE));
	spi->DR = (addr << 5);

	while (!(spi->SR & SPI_I2S_FLAG_RXNE));
	spi->DR;

	while (!(spi->SR & SPI_I2S_FLAG_TXE));
	spi->DR = 0;

	while (!(spi->SR & SPI_I2S_FLAG_RXNE));
	spi->DR;

	while (!(spi->SR & SPI_I2S_FLAG_RXNE));
	result = spi->DR;

	gpioCS->BSRRL = pinCS;

	return result;
}


void MRF24J40::setChannel(uint8_t channel) {
	this->channel = channel;

	// Set channel – See Section 3.4 “Channel Selection”.
	writeLong(RFCON0, (channel << 4) | 0x03);

	// RFCTL (0x36) = 0x04 – Reset RF state machine.
	writeShort(RFCTL, 0x04);

	// RFCTL (0x36) = 0x00.
	writeShort(RFCTL, 0x00);

	// Delay at least 192 μs.
	delayTimer.uDelay(200);
}

uint8_t MRF24J40::readChannel() {
	return readLong(RFCON0) >> 4;
}

void MRF24J40::setPANId(uint8_t panId[2]) {
	this->panId[0] = panId[0];
	this->panId[1] = panId[1];

	writeShort(PANIDL, panId[0]);
	writeShort(PANIDH, panId[1]);
}

uint16_t MRF24J40::readPANId() {
	return readShort(PANIDL) | (readShort(PANIDH) << 8);
}

void MRF24J40::setSAddr(uint8_t sAddr[2]) {
	this->sAddr[0] = sAddr[0];
	this->sAddr[1] = sAddr[1];

	writeShort(SADRL, sAddr[0]);
	writeShort(SADRH, sAddr[1]);
}

uint16_t MRF24J40::readSAddr() {
	return readShort(SADRL) | (readShort(SADRH) << 8);
}

void MRF24J40::sendPacket(uint8_t* data, uint8_t length) {
	// TODO

	txCount++;
}


/*
 * MRF24J40.cpp
 *
 *  Created on: 14. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "MRF24J40.h"
#include "main.h"

MRF24J40::MRF24J40(Properties& initProps, PulseLED recvLed, PulseLED sendLed) : props(initProps), recvLed(recvLed), sendLed(sendLed) {
}

MRF24J40::~MRF24J40() {
}

void MRF24J40::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void MRF24J40::init() {
	// Enable the SPI clock
	props.clkSPICmdFun(props.clkSPI, ENABLE);

	// Enable GPIO clocks
	RCC_AHB1PeriphClockCmd(props.clkGPIOs, ENABLE);

	// Connect SPI pins to AF5
	GPIO_PinAFConfig(props.gpioTXRX, props.pinSourceSCK, props.afConfig); // alternative function SPIx_SCK
	GPIO_PinAFConfig(props.gpioTXRX, props.pinSourceMISO, props.afConfig); // alternative function SPIx_MISO
	GPIO_PinAFConfig(props.gpioTXRX, props.pinSourceMOSI, props.afConfig); // alternative function SPIx_MOSI

	// GPIO Deinitialisation
	GPIO_InitTypeDef gpioInitStruct;

	gpioInitStruct.GPIO_Pin = props.pinSCK | props.pinMISO | props.pinMOSI;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(props.gpioTXRX, &gpioInitStruct);

	gpioInitStruct.GPIO_Pin = props.pinCS;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(props.gpioCS, &gpioInitStruct);

	gpioInitStruct.GPIO_Pin = props.pinRST;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(props.gpioRST, &gpioInitStruct);

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
	SPI_Init(props.spi, &spiInitStruct);

	// Default (idle) CS state
	GPIO_SetBits(props.gpioCS, props.pinCS);


	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure INT pin as input */
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioInitStruct.GPIO_Pin = props.pinINT;
	GPIO_Init(props.gpioINT, &gpioInitStruct);

	SYSCFG_EXTILineConfig(props.extiPortSourceINT, props.extiPinSourceINT);

	/* Configure INT EXTI line */
	EXTI_InitStructure.EXTI_Line = props.extiLineINT;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set INT EXTI Interrupt to the given priority */
	NVIC_InitStructure.NVIC_IRQChannel = props.irqnINT;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irqPreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irqSubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	SPI_Cmd(props.spi, ENABLE);

	reset();
}

void MRF24J40::interruptHandler() {
	EXTI_ClearITPendingBit(props.extiLineINT);

	uint8_t intStat = readShort(INTSTAT);

	if (intStat & 0x01) { // TXNIF
		if (broadcastCompleteListener) {
			uint8_t txStat = readShort(TXSTAT);
			broadcastCompleteListener(txStat & 0x01);
		}
	}

	if (intStat & 0x08) { // RXIF
		if (recvListener) {
			recvListener();
		}
	}
}

void MRF24J40::setRecvListener(RecvListener recvListener) {
	this->recvListener = recvListener;
}


void MRF24J40::reset() {
	// Setting RESET# low creates the reset condition
	GPIO_ResetBits(props.gpioRST, props.pinRST);

	delayTimer.uDelay(1000);

	// Setting RESET# high removes the reset condition
	GPIO_SetBits(props.gpioRST, props.pinRST);

	delayTimer.uDelay(2000);


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


//TODO: Try out to set TX only and user TXRX mode only for the last byte of read

void MRF24J40::writeShort(uint8_t addr, uint8_t value) {
	assert_param(addr < 0x40);

	props.gpioCS->BSRRH = props.pinCS;

	props.spi->DR = (addr << 1) | 0x01;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE));
	props.spi->DR = value;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE)); // BSY flag is set only after some delay, so it is compulsory to first wait for TXE, which ensures that BSY flag is set meanwhile
	while (props.spi->SR & SPI_I2S_FLAG_BSY);
	props.gpioCS->BSRRL = props.pinCS;
}

void MRF24J40::writeLong(uint16_t addr, uint8_t value) {
	assert_param(addr < 0x400);

	props.gpioCS->BSRRH = props.pinCS;

	props.spi->DR = (addr >> 3) | 0x80;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE));
	props.spi->DR = (addr << 5) | 0x10;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE));
	props.spi->DR = value;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE)); // BSY flag is set only after some delay, so it is compulsory to first wait for TXE, which ensures that BSY flag is set meanwhile
	while (props.spi->SR & SPI_I2S_FLAG_BSY);
	props.gpioCS->BSRRL = props.pinCS;
}

uint8_t MRF24J40::readShort(uint8_t addr) {
	assert_param(addr < 0x40);

	uint8_t result;

	props.gpioCS->BSRRH = props.pinCS;

	props.spi->DR; props.spi->SR; // Clear the OVR flag

	props.spi->DR = (addr << 1);

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE));
	props.spi->DR = 0;

	while (!(props.spi->SR & SPI_I2S_FLAG_RXNE));
	props.spi->DR;

	while (!(props.spi->SR & SPI_I2S_FLAG_RXNE));
	result = props.spi->DR;

	props.gpioCS->BSRRL = props.pinCS;

	return result;
}

void MRF24J40::setBroadcastCompleteListener(
		BroadcastCompleteListener broadcastCompleteListener) {
}

void MRF24J40::recvPacket(uint8_t (&data)[TOHMessage::MAX_RF_PACKET_LENGTH], uint8_t& dataLength, uint8_t (&fcs)[2], uint8_t& lqi, uint8_t& rssi) {
	writeShort(BBREG1, 0x04); // Disable RX
	dataLength = readLong(RXFIFO) - 2; // Read length

	for (int idx = 0; idx < dataLength; idx++) {
		data[idx] = readLong(RXFIFO + 1 + idx);
	}

	// TODO: Try to enable RX sooner - e.g. after 10 received packets
	writeShort(BBREG1, 0x00); // Enable RX

	// TODO: Parse header and return it as distinct from data payload

	fcs[0] = readLong(RXFIFO + 1 + dataLength);
	fcs[1] = readLong(RXFIFO + 1 + dataLength + 1);
	lqi = readLong(RXFIFO + 1 + dataLength + 2);
	rssi = readLong(RXFIFO + 1 + dataLength + 2);
}


uint8_t MRF24J40::readLong(uint16_t addr) {
	assert_param(addr < 0x400);

	uint8_t result;

	props.gpioCS->BSRRH = props.pinCS;

	props.spi->DR; props.spi->SR; // Clear the OVR flag

	props.spi->DR = (addr >> 3) | 0x80;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE));
	props.spi->DR = (addr << 5);

	while (!(props.spi->SR & SPI_I2S_FLAG_RXNE));
	props.spi->DR;

	while (!(props.spi->SR & SPI_I2S_FLAG_TXE));
	props.spi->DR = 0;

	while (!(props.spi->SR & SPI_I2S_FLAG_RXNE));
	props.spi->DR;

	while (!(props.spi->SR & SPI_I2S_FLAG_RXNE));
	result = props.spi->DR;

	props.gpioCS->BSRRL = props.pinCS;

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

void MRF24J40::broadcastPacket(uint8_t* data, uint8_t length) {

	int txReg = TXNFIFO;

	writeLong(txReg++, 7); // Header length
	writeLong(txReg++, 7 + length); // Frame length

	// Frame Control (order low-byte, high-byte; MSb)
	// =============================================
	// Source Addressing Mode [15-14] - 0x10 (Short address 16bit)
	// Frame Version [13-12] - 0x00 (802.15.4-2003)
	// Dest. Addressing Mode [11-10] - 0x00 (PAN identifier and address fields are not present)
	// Reserved [9-7]
	// PAN ID Compression [6] - 0x00 (No PAN ID Compression)
	// Acknowledgement Request [5] - 0x00 (No acknowledgement requested)
	// Frame Pending [4] - 0x00 (No Frame Pending)
	// Security Enabled [3] - 0x00 (No security)
	// Frame Type [2-0] - 0x01 (Data Frame)
	writeLong(txReg++, 0x01);
	writeLong(txReg++, 0x80);

	writeLong(txReg++, 0x00); // Sequence Number

	writeLong(txReg++, panId[0]); // Source PAN ID (low)
	writeLong(txReg++, panId[1]); // Source PAN ID (high)

	writeLong(txReg++, sAddr[0]); // Source Address (low)
	writeLong(txReg++, sAddr[1]); // Source Address (high)

	for (int idx = 0; idx < length; idx++) {
		writeLong(txReg++, data[idx]);
	}

	writeShort(TXNCON, 0x01); // Trigger transmission, set: Frame Pending Status Bit = 0, Activate Indirect Transmission bit = 0, TX Normal FIFO Acknowledgement Request = 0, TX Normal FIFO Security Enabled = 0

	txCount++;
}


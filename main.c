/*
 * The MIT License (MIT)
 *
 * File: main.c
 *
 * Description: Contains the main loop for software.
 *
 * Copyright (c) 2013 Ed Crampton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "msprf24/msprf24.h"
#include "msprf24/nrf_userconfig.h"
#include "messaging/messages.h"
#include "RfMessageHandler.h"
#include "board.h"

//Name: Initialize
//Description: Initializes Msp430
//Args:
//Returns:
static void Initialize();

//Name: InitializePins
//Description: Initializes Pins on Msp430
//Args:
//Returns:
static void InitializePins();

//Name: InitializeRF24
//Description: Initializes The nrf24l01 
//Args:
//Returns:
static void InitializeRF24();

#define SOURCEID 0xFF //< Source ID of this rf device

static INIT_PINGMSG(pingMsg,SOURCEID); //< Create the Ping Message Used

int main()
{
	unsigned long long i = 0;
	Rf24MessageBase* rf_msg;
	PingMessage* p_msg;

	//Initialize the Msp430 and the nrf24l01+
	Initialize();

   
   while(1)
   {
	 i = 0;
	 //Send the ping message and increment counter
	 SendMessage((Rf24MessageBase*)&pingMsg);
	 ++pingMsg.Count;

	 WDTCTL = WDT_ADLY_250;//Reset the Watchdog to 250ms (Ensure tx is sent)
	 LPM3; //what for fin of tx packet this should stop aclk so
		   //no new temperature data while we wait
	 WDTCTL = WDT_ADLY_250; //Reset the watchdog again to ensure it doesnt trigger
	 P1OUT &= ~GREEN_LED_PIN; // Green LED off

	 //Check the irq reason if one is present
	 if (CheckRfIrq() > 0) {
		 if(CheckRfTx() > 0) {
//		 msprf24_get_irq_reason();
//		 if (rf_irq & RF24_IRQ_TX){
			 P1OUT |= GREEN_LED_PIN;  // Green LED on
		 }
		 else {
			 //tx failed do nothing...
		 }

		 msprf24_irq_clear(rf_irq);
	 }

	 // Receive mode
	 ReceiveMessage();
	 WDTCTL = WDT_ADLY_250;
	 LPM3; //what for fin of tx packet this should stop aclk so
		   //no new temperature data while we wait
	 WDTCTL = WDT_ADLY_250;
	 P1OUT &= ~RED_LED_PIN;
	 if (CheckRfIrq() > 0) {

		 if(CheckRfRx() > 0) {
			 ReadMessage(&rf_msg);
			 if(rf_msg->MessageId == PONG_MSG_ID)
				 p_msg = (PingMessage*)rf_msg;
				 if(p_msg->Count == (pingMsg.Count -1))
					 P1OUT |= RED_LED_PIN;  // Green LED on
			}

			msprf24_irq_clear(rf_irq);
		 }

		WDTCTL = WDT_ADLY_250;
		 while( i < 20000) { ++i; }

   }

   return 0;
}



//---------------------------------------------
//Function: Initialize()
//---------------------------------------------
static void Initialize()
{
   dint();
   //Disable watchdog and set clock to 16 Mhz
	WDTCTL = WDTHOLD | WDTPW;
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;
	BCSCTL2 = DIVS_1;  // SMCLK = DCOCLK/2

   InitializePins(); //Set up the pins
   InitializeRF24();

   //WDTCTL = WDT_ADLY_250; // Set Watchdog Timer interval to ~250ms
   IE1 |= WDTIE; // Enable WDT interrupt

   eint();
}

interrupt (WDT_VECTOR) WDT_ISR(void)
{

	__bic_SR_register_on_exit(LPM3_bits);
}

//---------------------------------------------
//Function: InitializePins()
//---------------------------------------------
static void InitializePins()
{
   P1OUT = 0x00;    // Initialize all GPIO
   P1DIR = 0xFF & ~(TEMP_PIN);               // Set all pins but temp pin to output
   P2DIR |= 0x02; //ping 2.6  is xin
}

static void InitializeRF24()
{
	uint8_t addr[5];
	/* Initial values for nRF24L01+ library config variables */
	rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
	rf_addr_width      = 5;
	rf_speed_power     = RF24_SPEED_250KBPS | RF24_POWER_0DBM;
	rf_channel         = 76;

	msprf24_init();  // All RX pipes closed by default
	msprf24_set_retransmit_count(15);
	msprf24_set_retransmit_delay(4000);
	msprf24_set_pipe_packetsize(0, 0);
	msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst enabled for receiving Auto-ACKs

	// Transmit to 'rad01' (0x72 0x61 0x64 0x30 0x31)
	msprf24_standby();
	addr[0] = 0xF0;	addr[1] = 0xF0;	addr[2] = 0xF0;	addr[3] = 0xF0;	addr[4] = 0xE1;
	w_tx_addr(addr);
	w_rx_addr(0, addr);  // Pipe 0 receives auto-ack's, autoacks are sent back to the TX addr so the PTX node
					 // needs to listen to the TX addr on pipe#0 to receive them.
	addr[4] = 0xD2;
	w_rx_addr(1,addr);
   
	return;
}



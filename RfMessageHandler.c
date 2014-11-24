#include "RfMessageHandler.h"
#include "msprf24/msprf24.h"
#include "msprf24/nrf_userconfig.h"

char mRfRxBuffer[32];

int SendMessage(Rf24MessageBase* rfMessage)
{
	uint8_t msg_size = 0;
	//@todo update this to get from the messages library
	switch(rfMessage->MessageId)
	{
	case PONG_MSG_ID:
	case PING_MSG_ID:
		msg_size = MESSAGE_SIZE(PingMessage);
		break;
	case TEMPERATURE_MSG_ID:
		msg_size = MESSAGE_SIZE(TemperatureMessage);
		break;
	}

	if(msg_size == 0)
		return -1; //error message can't be sent

	//Send that payload
	w_tx_payload(msg_size, (char*) rfMessage);
	msprf24_activate_tx();
	return 0;
}

inline void ReceiveMessage()
{
	//Activate rf receive
	msprf24_activate_rx();
}

inline char CheckRfRx()
{
	char ret = 0;
	msprf24_get_irq_reason();

	if(rf_irq & RF24_IRQ_RX) {
		ret = 1;
	}
	return ret;
}

inline char CheckRfTx()
{
	char ret = 0;
	msprf24_get_irq_reason();

	if(rf_irq & RF24_IRQ_TX) {
		ret = 1;
	}
	return ret;
}

inline char CheckRfIrq()
{
	//Check if rf is flagged
	char ret = 0;
	if (rf_irq & RF24_IRQ_FLAGGED) {
		ret = 1;
		rf_irq &= ~RF24_IRQ_FLAGGED;
	}

	return ret;
}

int ReadMessage(Rf24MessageBase** rfMessage)
{
	uint8_t rx_size = r_rx_peek_payload_size();
	r_rx_payload(rx_size, mRfRxBuffer);

	*rfMessage = mRfRxBuffer;

	return (*rfMessage)->MessageId;
}



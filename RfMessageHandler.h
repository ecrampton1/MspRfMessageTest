#include "messaging/messages.h"
#ifndef _RF_MESSAGE_HANDLER_H_
#define _RF_MESSAGE_HANDLER_H_


//Name: SendMessageAsync
//Description: Send a Message and return without blocking
//Args:
//Returns:
int SendMessage(Rf24MessageBase* rfMessage);

//Name: ReceiveMessageAsync
//Description: Set up rf to receive a message without blocking
//				Up to user to check for message
//Args:
//Returns:
extern inline void ReceiveMessage();

//Name: CheckRfIrq
//Description: Check if Rf Interrupt was triggered
//Args:
//Returns:
extern inline char CheckRfIrq();

//Name: CheckRfIrq
//Description: Check if Rf Interrupt was from Rx Interrupt
//Args:
//Returns:
extern inline char CheckRfRx();

//Name: CheckRfTx
//Description: Check if Rf Interrupt was from Tx Interrupt
//Args:
//Returns:
extern inline char CheckRfTx();

//Name: ReceiveMessageAsync
//Description: Return a pointer to a message in memory.  If user
//				wants to keep memory before next call it must be copied.
//Args:
//Returns:
int ReadMessage(Rf24MessageBase** rfMessage);

#endif //_RF_MESSAGE_HANDLER_H_

#ifndef _SERVICE_COMPLETE_HANDLER_H_
#define _SERVICE_COMPLETE_HANDLER_H_

/*ACE headers.*/
#include "ace/Asynch_IO.h"

class ServiceCompleteHandler : public ACE_Service_Handler
{
public:
	ServiceCompleteHandler();
	~ServiceCompleteHandler();

public:
	/**
   * {open} is called by ACE_Asynch_Acceptor to initialize a new
   * instance of ACE_Service_Handler that has been created after the
   * new connection is accepted. The handle for the new connection is
   * passed along with the initial data that may have shown up.
   */
	virtual void open( ACE_HANDLE new_handle, ACE_Message_Block & message_block );

protected:/*These methods are called by the framework.*/
	/*This is called when asynchronous <read> operation from the socket complete.*/
	virtual void handle_read_stream( const ACE_Asynch_Read_Stream::Result & result );
	/*This is called when an asynchronous <write> completes.*/
	virtual void handle_write_stream( const ACE_Asynch_Write_Stream::Result & result );

private:
	/*Initiate an asynchronous <read> operation on the socket.*/
	int initiate_read_stream();

private:
	ACE_Asynch_Read_Stream m_rStream;
	ACE_Asynch_Write_Stream m_wStream;

	ACE_HANDLE m_hSocket;
};

#endif
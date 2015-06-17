#include "ServiceCompleteHanlder.h"

/*ACE headers.*/
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/Asynch_IO_Impl.h"
#include "ace/Proactor.h"


ServiceCompleteHandler::ServiceCompleteHandler()
{

}

ServiceCompleteHandler::~ServiceCompleteHandler()
{

}

void ServiceCompleteHandler::open( ACE_HANDLE new_handle, ACE_Message_Block & message_block )
{
	/*Cache the underlying socket-handle of the new connection.*/
	m_hSocket = new_handle;

	/*Initiate<ACE_Asynch_Read_Stream>, which means constructing read-stream.*/
	if ( -1 == this->m_rStream.open( *this, m_hSocket ) )
	{
		ACE_ERROR( ( LM_ERROR, "%p\n", "ACE_Asynch_Read_Stream::open()." ) );

		return;
	}
	
	/*Initiate<ACE_Asynch_Write_Stream, which means constructing write-stream.>.*/
	if ( -1 == this->m_wStream.open( *this, m_hSocket ) )
	{
		ACE_ERROR( ( LM_ERROR, "%p\n", "ACE_Asynch_Write_Stream::open()." ) );

		return;
	}

	/*If the client submits some messages when it connects to server at the same time,
	there needs to fake a result and call read-complete function(handle_read_stream()).*/
	if ( message_block.length() != 0 )
	{
		ACE_Message_Block & duplicate = *( message_block.duplicate() );

		/*Fake the result so that we can call read-complete function(handle_read_stream()).*/
		ACE_Asynch_Read_Stream_Result_Impl * fake_result = 
			ACE_Proactor::instance()->create_asynch_read_stream_result( this->proxy(),
																		this->m_hSocket,
																		duplicate, 
																		1024,
																		0,
																		ACE_INVALID_HANDLE,
																		0,
																		0 );

		/*Update "message_block" to the beginning position.*/
		size_t bytes_transferred = message_block.length ();
		duplicate.wr_ptr( duplicate.wr_ptr() - bytes_transferred );

		/*This will realy call read-complete function(handle_read_stream()).*/
		fake_result->complete ( message_block.length(), 1, 0 );

		/*Zap the fake result.*/
		delete fake_result;
	}
	else if ( -1 == this->initiate_read_stream() )/*Initiate read-request.*/
	{
		return;
	}
}

void ServiceCompleteHandler::handle_read_stream( const ACE_Asynch_Read_Stream::Result & result )
{
	//Construct strings.
	result.message_block().rd_ptr()[ result.bytes_transferred() ] = '\0';

	ACE_DEBUG ( ( LM_DEBUG, "********************\n" ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "bytes_to_read", result.bytes_to_read() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "handle", result.handle() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "bytes_transfered", result.bytes_transferred() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "act", ( uintptr_t )result.act() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "success", result.success() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "completion_key", ( uintptr_t )result.completion_key() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "error", result.error() ) );
	ACE_DEBUG ( ( LM_DEBUG, "********************\n" ) );

	/*Successfully read.*/
	if ( ( result.success() && result.bytes_transferred() ) != 0  )
	{
		ACE_DEBUG ( ( LM_DEBUG, "Read successfully.\n\n" ) );

		/*Send what we have received to peer.*/
		if ( -1 == this->m_wStream.write( result.message_block(), result.bytes_transferred() ) )
		{
			ACE_ERROR( ( LM_ERROR, "%p\n", "ACE_Asynch_Write_File::write()." ) );

			return;
		}

		// Initiate a new asynch read.
		if ( -1 == this->initiate_read_stream() )
		{
			return;
		}
	}
	else/*Receive completely, which means the connection has closed.*/
	{
		/*No need for this message-block anymore.*/
		result.message_block().release();

		/*Commit suicide.*/
		delete this;
	}
}

void ServiceCompleteHandler::handle_write_stream( const ACE_Asynch_Write_Stream::Result & result )
{
	ACE_DEBUG ( ( LM_DEBUG, "********************\n" ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "bytes_to_write", result.bytes_to_write() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "handle", result.handle() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "bytes_transfered", result.bytes_transferred() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "act", ( uintptr_t )result.act() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "success", result.success() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "completion_key", ( uintptr_t )result.completion_key() ) );
	ACE_DEBUG ( ( LM_DEBUG, "%s = %d\n", "error", result.error() ) );
	ACE_DEBUG ( ( LM_DEBUG, "********************\n" ) );

	/*Write successfully.*/
	if ( result.success() )
	{
		ACE_DEBUG ( ( LM_DEBUG, "Write successfully.\n\n" ) );
	}

	/*No need for this message-block anymore.*/
	result.message_block().release();
}

int ServiceCompleteHandler::initiate_read_stream()
{
	/*New a message-block for asynch reading.*/
	ACE_Message_Block * mb = NULL;
	ACE_NEW_RETURN( mb, ACE_Message_Block( BUFSIZ + 1 ), -1 );

	/*Asynch read.*/
	if ( -1 == this->m_rStream.read( *mb, ( mb->size() - 1 ) ) )
	{
		ACE_ERROR_RETURN ( ( LM_ERROR, "%p\n", "ACE_Asynch_Read_Stream::read()." ), -1 );
	}

	return 0;
}

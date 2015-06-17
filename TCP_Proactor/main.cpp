#include "ServiceCompleteHanlder.h"

/*ACE headers.*/
#include "ace/Asynch_Acceptor.h"
#include "ace/Proactor.h"

int main( int argc, char * argv[] )
{
	ACE_Asynch_Acceptor< ServiceCompleteHandler > acceptor;

	if ( -1 == acceptor.open( ACE_INET_Addr( 8010 ) ) )
	{
		return -1;
	}

	while ( true )
	{
		ACE_Proactor::instance()->handle_events();
	}

	system( "pause" );
	return 0;
}
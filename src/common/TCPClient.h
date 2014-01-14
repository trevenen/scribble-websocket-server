#ifndef TCP_CLIENT_HEADER
#define TCP_CLIENT_HEADER
#include "SocketClass.h"

class TCPClient : public SocketClass {
	public:
		TCPClient ( );
		TCPClient ( int );

		// TCPClient ( TCPClient &client ) {

		// }

		~TCPClient ( );

		int connect ( const char *, const char * );
		int recv ( void *, size_t len );
		int recv ( const void * , size_t len );
		int peek ( ); // check for errors or disconnections
		int send ( const void *, size_t len );
		int isActive ( );
		void * get_in_addr ( struct sockaddr * );
	private:
		int isConnected;
};

#endif
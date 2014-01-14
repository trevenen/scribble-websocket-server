#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER
#include <iostream>
#include <string>
#include <cstring>
#include "./common/TCPClient.h"
#include "./protocols/WSProtocol.h"
#include "./protocols/rfc_6455/RFC_6455.h"
#include "./IOMsg.h"

const int max_recv_size = 1024; // maximum receive size: 65535 (MAX SHORT)

enum ConnectionState { 
	NEW,
	PENDING,
	READY
};

class Connection {
	public:
		Connection ( );
		~Connection ( );

		void setConnection ( int );
		void setConnection ( TCPClient );
		void setState ( ConnectionState );
		ConnectionState getState ( );

		void reset ( );

		TCPClient& getConnection ( );

		// It can contain other protocols... but
		WSProtocol& getProtocol ( );
		int handleHandshake ( IOMsg& );

		std::string getChannel ( );

		int decodedRecv ( IOMsg& );
		int encodedSend ( const char * buffer , int size );
		
		/*		
		inline static void gen_random(char *s, const int len) {
		    static const char alphanum[] =
		        "0123456789"
		        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		        "abcdefghijklmnopqrstuvwxyz";

		    for (int i = 0; i < len; ++i) {
		        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
		    }

		    s[len] = 0;
		}
		*/

	private:
		//std::string uniqueID;
		TCPClient m_connection;
		RFC_6455 m_protocol;
		std::string m_channel;

		ConnectionState m_state;
};

#endif

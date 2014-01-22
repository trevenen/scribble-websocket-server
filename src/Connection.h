#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER
#include <iostream>
#include <string>
#include <cstring>
#include "common/TCPClient.h"
#include "common/LockClass.h"
#include "protocols/WSProtocol.h"
#include "protocols/rfc_6455/RFC_6455.h"
#include "IOMsg.h"

const int max_recv_size = 1024; // maximum receive size: 65535 (MAX SHORT)

enum ConnectionState { 
	NEW,
	PENDING,
	READY,
	INCHANNEL
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

		// It can contain other protocols... but we will keep it a strict
		WSProtocol& getProtocol ( );
		int handleHandshake ( IOMsg& );
		
		std::string getChannel ( );

		int decodedRecv ( IOMsg& );
		int encodedSend ( const char * buffer , int size );

	private:
		TCPClient m_connection;
		RFC_6455 m_protocol;
		std::string m_channel;

		std::string m_incoming_buffer;  // Used for parsing longer packets, that may come from several packets
		LockClass m_incoming_lock;
		ConnectionState m_state;
};

#endif

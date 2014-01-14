#include "Connection.h"

Connection::Connection ( ) {
	m_state = NEW;
}

Connection::~Connection ( ) {

}

void Connection::setConnection ( int socket_fd ) {
	m_state = PENDING;
	m_connection = TCPClient ( socket_fd );
}

void Connection::setConnection ( TCPClient connection ) {
	m_state = PENDING;
	m_connection = connection;
}

void Connection::setState ( enum ConnectionState state ) {
	m_state = state;
}

void Connection::reset ( ) {
	m_state = NEW;
	m_connection.close ( );
}

ConnectionState Connection::getState ( ) {
	return m_state;
}

TCPClient& Connection::getConnection ( ) {
	return m_connection;
}

WSProtocol& Connection::getProtocol ( ) {
	return *(WSProtocol*)&m_protocol;
}

int Connection::decodedRecv ( IOMsg &recv ) {
	std::string tmpBuffer; 
	
	// Prepare for incoming data
	tmpBuffer.resize ( max_recv_size );

	// Receive data into tmpBuffer
	m_connection.recv ( tmpBuffer.data ( ) , max_recv_size );

	// Try and decode
	tmpBuffer = m_protocol.decode ( tmpBuffer );

	// Check to see if the message was complete or we still need to wait.
	if ( tmpBuffer.empty ( ) ) {
		// Nothing was received
		return 0;
	}
		
	// Data decoded now copy it over to the recv payload.
	// prepare recv
	// copy over.
	memcpy ( recv.getBuffer ( tmpBuffer.size ( ) ).data ( ) , tmpBuffer.data ( ) , tmpBuffer.size ( ) );
	
	return 1;
}

int Connection::encodedSend ( const char * buffer , int size ) {
	std::string tmpBuffer ( buffer , size );

	tmpBuffer = m_protocol.encode ( tmpBuffer );
	m_connection.send ( tmpBuffer.c_str ( ) , tmpBuffer.size ( ) );

	return 1;
}

int Connection::handleHandshake ( IOMsg &response ) {
	try {
		int bytes;
		std::vector<char> payload ( max_recv_size );
		WSAttributes attributes;

		bytes = m_connection.recv ( payload.data ( ) , payload.size ( ) );
		if ( bytes > 0 ) {				
			if ( m_protocol.handshake ( payload.data ( ), &attributes ) == 0 ){
				// (DEBUG)
				// std::cout<<"[Version] " << attributes.version << std::endl;
				// std::cout<<"[Channel] " << attributes.channel << std::endl;
				// std::cout<<"[Response] " << std::endl << attributes.response << std::endl;
				
				// Save requested channel
				m_channel = attributes.channel;

				// Prepare response
				::memcpy ( response.getBuffer ( attributes.response.length ( ) ).data ( ), attributes.response.data ( ) , attributes.response.length ( ) );
				m_state = READY;
				// success
				return 1;
			} else {
				throw LogString ( "malformed handshake received." );
			}
		} else {
			throw LogString ( "error occurred while receiving data." );
		}
	} catch ( LogString &e ) {
		Logit ( "Connection: " + e );
	}
	// fail
	return 0;			
}

std::string Connection::getChannel ( ) {
	return m_channel;
}

/*
	std::string Connection::getID ( ) {
		return uniqueID;
	}
*/

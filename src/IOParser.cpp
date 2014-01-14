#include "IOParser.h"

IOParser::IOParser ( ) {
	m_status = 1;
}

IOParser::~IOParser ( ) {

}

void IOParser::setConnections ( std::vector<Connection> * connections ) { 
	m_connections = connections;
}

void IOParser::setQueues ( SafeData<IOMsg> * inQ ,  SafeData<IOMsg> * outQ ,  SafeData<IOMsg> * channelQ ) {
	m_inqueue = inQ;
	m_outqueue = outQ;
	m_channelqueue = channelQ;
}

void IOParser::setStatus ( int status ) {
	m_status = status;
}

void IOParser::newConnection ( int socket_fd ) {
	try {
		(*m_connections)[socket_fd].setConnection ( socket_fd );
		// Notify a Channel Manager of a new user :)
		//....still needs work
	} catch ( LogString &e ) {
		Logit ( "IOParser: " + e );
		disconnectedConnection ( socket_fd );
	}
}

void IOParser::disconnectedConnection ( int socket_fd ) {
	try {
		// Close and reset connection at given socket id for reuse.
		(*m_connections)[socket_fd].reset ( );
		// Create message to notify Channel Manager
	} catch ( LogString &e ) {
		Logit ( "IOParser: " + e );
	}
}

void IOParser::handleIncoming ( int socket_fd ) {
	try {
		IOMsg tmp_packet ( socket_fd );
		
		// recv data
		if ( (*m_connections)[socket_fd].getState ( ) == READY ) {
			// Check if data can be decoded
			if ( (*m_connections)[socket_fd].decodedRecv ( tmp_packet ) ) {
				// Data received and ready inside of tmp_packet
				Logit ( tmp_packet.getBuffer ( ).data ( ) );
			}
		} else {
			// Pending 
			// Process as a handshake ;)
			if ( !(*m_connections)[socket_fd].handleHandshake(tmp_packet) ) {
				disconnectedConnection ( socket_fd );
				throw LogString ( "issue handling handshake occurred." );
			}

			// Queue handshake response
			m_outqueue->push ( tmp_packet );
			m_outqueue->signal ( );
		}
		
		// send to lua accordingly
	} catch ( LogString &e ) {
		Logit ( "IOParser: " + e );
	}
}

void IOParser::handleOutgoing ( IOMsg &packet ) {
	try {
		
		// Check if connection is still open
		if ( !(*m_connections)[packet.getConnectionID ( )].getConnection ( ).isActive ( ) ) {
			throw LogString ( "Invalidated connection." ); // Log it..
		}
		
		if ( packet.getState ( ) == ENCODED ) {
			// Send data out.
			(*m_connections)[packet.getConnectionID ( )].encodedSend ( packet.getBuffer ( ).data ( ), packet.getBuffer ( ).size ( ) );
		} else {
			(*m_connections)[packet.getConnectionID ( )].getConnection ( ).send ( packet.getBuffer ( ).data ( ) , packet.getBuffer ( ).size ( ) );
		}
	} catch ( LogString &e ) {
		Logit ( "IOParser: " + e );
	}
}

void IOParser::Setup ( ) {
	m_status = 1;
}

void IOParser::Execute ( void * args ) {
	try {
		Logit ( "IOParser: Waiting..." );
		while ( m_status ) {
			if ( !m_inqueue->wait ( 1 ) ) { // need try wait.
				IOMsg tmp_packet = m_inqueue->pop ( );

				switch ( tmp_packet.getState ( ) ) {
					case CONNECTED:
						{
							newConnection ( tmp_packet.getConnectionID ( ) );
						}
					break;
					case DISCONNECTED:
						{ 
							// Logit ( "Disconnected User" );
							disconnectedConnection ( tmp_packet.getConnectionID ( ) );
						}
					break;
					default:
						{
							// Logit ( "Normal" );
							handleIncoming ( tmp_packet.getConnectionID ( ) );
						}
				}
			} else 
			if ( !m_outqueue->wait ( 1 ) ) { // need try wait.
				IOMsg tmp_packet = m_outqueue->pop ( );

				handleOutgoing ( tmp_packet );
			}
		}
	} catch ( LogString &e ) {
		Logit ( "IOParser: " + e );
	}
}
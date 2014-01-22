#include "IOParser.h"

IOParser::IOParser ( ) {
	m_status = 1;
}

IOParser::~IOParser ( ) {
	m_status = 0;
}

void IOParser::setConnections ( std::vector<Connection> * connections ) { 
	m_connections = connections;
}

void IOParser::setQueues ( SafeData<IOMsg> * inQ ,  SafeData<IOMsg> * outQ ,  SafeData<IOMsg> * channelQ , SafeData<IOMsg> * disconnectionQ ) {
	m_inqueue = inQ;
	m_outqueue = outQ;
	m_channelqueue = channelQ;
	m_disconnection_queue = disconnectionQ;
}

void IOParser::setStatus ( int status ) {
	m_status = status;
}

void IOParser::handleIncoming ( int socket_fd ) {
	try {
		IOMsg tmp_packet ( socket_fd );
		
		// recv data
		if ( (*m_connections)[socket_fd].getState ( ) == INCHANNEL ) {
			// Check if data can be decoded
			// If it fails it gets saved into an internal buffer inside Connection for the next packet.
			if ( (*m_connections)[socket_fd].decodedRecv ( tmp_packet ) ) {
				// Data received and ready inside of tmp_packet
				Logit ( tmp_packet.getBuffer ( ).data ( ) );

				// Pass it off to a ChannelParser
				m_channelqueue->push ( tmp_packet );
				m_channelqueue->signal ( );
			}
		} else {
			// Pending 
			// Process as a handshake ;)
			if ( !(*m_connections)[socket_fd].handleHandshake(tmp_packet) ) {
				(*m_connections)[socket_fd].reset ( );
				throw LogString ( "issue handling handshake occurred." );
			}
			Logit ( "STATE: READY" );
			// Set Connection state to READY
			(*m_connections)[socket_fd].setState ( READY );

			// notify ChannelParser of new connection
			IOMsg new_user_packet ( socket_fd , CONNECTED );
			m_channelqueue->push ( new_user_packet );
			m_channelqueue->signal ( );

			// Queue handshake response
			tmp_packet.setState ( NOT_ENCODED );
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
		
		if ( packet.getState ( ) != NOT_ENCODED ) {
			// Send data out.
			(*m_connections)[packet.getConnectionID ( )].encodedSend ( packet.getBuffer ( ).data ( ), packet.getBuffer ( ).size ( ) );
		} else {
			(*m_connections)[packet.getConnectionID ( )].getConnection ( ).send ( packet.getBuffer ( ).data ( ) , packet.getBuffer ( ).size ( ) );
		}
	} catch ( LogString &e ) {
		Logit ( "IOParser: " + e );
	}
}

void IOParser::handleDisconnection ( IOMsg &packet ) {
	m_disconnection_queue->push ( packet );
	m_disconnection_queue->signal ( );
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
					case DISCONNECT:
						handleDisconnection ( tmp_packet );
					break;
					default:
						handleIncoming ( tmp_packet.getConnectionID ( ) );
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
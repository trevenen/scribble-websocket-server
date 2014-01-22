#include "ChannelParser.h"

ChannelParser::ChannelParser ( ) {
	m_channels = NULL;
	m_connections = NULL;
	m_channelqueue = NULL;
	m_outqueue = NULL;
	m_database = NULL;
	m_status = 1;
}

ChannelParser::~ChannelParser ( ) {
	m_channels = NULL;
	m_connections = NULL;
	m_channelqueue = NULL;
	m_outqueue = NULL;
	m_database = NULL;
	m_status = 0;
}

void ChannelParser::setChannels ( std::map<std::string,Channel> * channels ) {
	m_channels = channels;
}

void ChannelParser::setConnections ( std::vector<Connection> * connections ) {
	m_connections = connections;
}
		
void ChannelParser::setQueues ( SafeData<IOMsg> * inQ , SafeData<IOMsg> * outQ ) {
	m_channelqueue = inQ;
	m_outqueue = outQ;
}	

void ChannelParser::setDatabase ( MySQL * database ) {
	m_database = database;
}

void ChannelParser::setStatus ( int status ) {
	m_status = status;
}

void ChannelParser::setSync ( LockClass * lock ) {
	m_sync_lock = lock;
}

void ChannelParser::Setup ( ) {
	try {
		// Check environment.
		if ( !m_channels ) {
			throw LogString ( "Channels wasn't set." );
		}
		
		if ( !m_connections ) {
			throw LogString ( "Connections wasn't set." );		
		}

		if ( !m_channelqueue ) {
			throw LogString ( "ChannelQueue wasn't set." );
		}
		
		if ( !m_outqueue ) {
			throw LogString ( "OutQueue wasn't set." );		
		}
	
		if ( !m_database ) {
			throw LogString ( "Database wasn't set." );
		}

		if ( !m_sync_lock ) {
			throw LogString ( "Sync lock wasn't set." );
		}

		// Try and lock the global Channels map
		// Should only be inserted/updated by one ChannelParser
		if ( !m_sync_lock->trylock ( ) ) {
			// Lock Successful
			
			// Query for all Channels
			if ( !m_database->query ( "select * from apps" ) ) {
                    throw LogString ( "Query error occured." );
            }

            // Start inserting them into global Channels map
            while ( m_database->hasNext ( ) ) {
                    std::vector<std::string> l = m_database->next ( );
            		
            		Logit ( "Channel Loaded: " + std::string ( l[2].c_str()) );
            		
            		// Create Channel for App Channel in Database
            		Channel tmp_channel ( atoi(l[0].c_str()), l[9].c_str(), l[1] , l[3] , 1000 );
					
					// Add into all Channels
					m_channels->insert ( std::pair<std::string, Channel> ( l[2] , tmp_channel ) );
            		
            		// Initialize once inside Channels map

            		if ( m_channels->find ( l[2] ) != m_channels->end ( ) ) {
            			(*m_channels)[l[2]].init ( );
					}
            }

			// Unlock once finished
			m_sync_lock->unlock ( );
		}

		// Synchronize all ChannelParsers, and make them wait till they can lock after all Channels are loaded.
		m_sync_lock->lock ( );
		m_sync_lock->unlock ( );
		
	} catch ( LogString &e ) {
		Logit ( "ChannelParser: " + e );
	}
}

void ChannelParser::Execute ( void * args ) {
	try {
		Logit ( "ChannelParser: Waiting..." );
		while ( m_status ) {
			if ( !m_channelqueue->wait ( 1 ) ) { // need try wait.
				IOMsg tmp_packet = m_channelqueue->pop ( );
				switch ( tmp_packet.getState ( ) ) {
					case CONNECTED:
					{
						handleConnected ( tmp_packet );
					}
				break;
				case DISCONNECTED:
					{
						handleDisconnected ( tmp_packet );
					}
				break;
				default:
					{
						handleChannelPacket ( tmp_packet );
					}
				}
			} 
			// Check timeout for next ChannelUpdate

		}
	} catch ( LogString &e ) {
		Logit ( "ChannelParser: " + e );
	}
}

void ChannelParser::handleChannelPacket ( IOMsg &packet ) {
	try {
		// Ingest data and process,
		if ( (*m_connections)[packet.getConnectionID ( )].getState ( ) != INCHANNEL ) {
			throw LogString ( "Shouldn't have arrived here with current STATE" );
		}
		
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( )].lock ( );
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( )].parsePacket ( packet , m_database );
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( )].unlock ( );

	} catch ( LogString &e ) {
		Logit ( "ChannelParser: " + e );
	}
}

void ChannelParser::handleConnected ( IOMsg &packet ) {
	try {
		// Make sure Connection is still valid and this packet isn't old.
		if ( (*m_connections)[packet.getConnectionID ( )].getState ( ) != READY ) {
			throw LogString ( "Connection has been closed, this will be disregarded." );
		}

		// Make sure Channel Exists if not we need to disconnect user.
		if ( m_channels->find((*m_connections)[packet.getConnectionID ( )].getChannel ( ) ) == m_channels->end ( ) ) {
			// Disconnect user
			// Notify Listener
			throw LogString ( "Unable to locate Channel ( " + (*m_connections)[packet.getConnectionID ( )].getChannel ( ) + " )" );
		}

		(*m_connections)[packet.getConnectionID ( )].setState ( INCHANNEL );
		Logit ( "CHANNELPARSER: CONNECTED" );
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( )].lock ( );
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( ) ].addConnection ( packet.getConnectionID ( ) , m_database );
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( )].unlock ( );

	} catch ( LogString &e ) {
		Logit ( "ChannelParser: " + e );
	}
}

void ChannelParser::handleDisconnected ( IOMsg &packet ) {
	try {
		if ( (*m_connections)[packet.getConnectionID ( )].getState ( ) != INCHANNEL ) {
			throw LogString ( "Connection was never added to a Channel, this will be disregarded." );
		}
		
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( ) ].removeConnection ( packet.getConnectionID ( ) , m_database);
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( ) ].removeConnection ( packet.getConnectionID ( ) , m_database);
		(*m_channels)[(*m_connections)[packet.getConnectionID ( )].getChannel ( ) ].removeConnection ( packet.getConnectionID ( ) , m_database);

	} catch ( LogString &e ) {
		Logit ( "ChannelParser: " + e );
	}
}
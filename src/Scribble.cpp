#include "Scribble.h"

Scribble::Scribble ( ) {
	// Setup 
	// Load in LUA file
	m_config.load ( "config/config.lua" );
		
	// Load in config file values
	m_config.focusVar ( "config" );
	m_script_update_delay = m_config.getTableValue_int ( "script_update_delay" );
	m_listener_port = m_config.getTableValue_int ( "port" );
	m_io_parser_count = m_config.getTableValue_int ( "parser_io" );
	m_max_connections = m_config.getTableValue_int ( "max_connections" );
	m_database_host = m_config.getTableValue_str ( "db_host" );	
	m_database_username = m_config.getTableValue_str ( "db_username" );	
	m_database_password = m_config.getTableValue_str ( "db_password" );	
	m_database_name = m_config.getTableValue_str ( "db_database" );	

	// Create specified Connection Containers
	m_connections.resize ( m_max_connections ); 
	
	// Create specified IOParsers
	m_io_parsers.resize ( m_io_parser_count );
	
	// Create specified ChannelParsers
	m_channel_parsers.resize ( m_io_parser_count );

	// Database connections for Parsers
	m_channel_databases.resize ( m_io_parser_count );

	// Set Listener Attributes
	m_listener.setConnections ( &m_connections );
	m_listener.setQueues ( &m_inqueue , &m_disconnection_queue );

	// Set buffers and Connections information
	for ( unsigned i = 0 ; i < m_io_parsers.size ( ) ; i ++ ) {
		m_io_parsers[i].setConnections ( &m_connections );
		m_io_parsers[i].setQueues ( &m_inqueue, &m_outqueue, &m_channelqueue, &m_disconnection_queue );
	}

	// Connect and login each database Connection.
	for ( unsigned i = 0 ; i < m_channel_databases.size ( ) ; i ++ ) {
		// MySQL DB Connection call
        if ( !m_channel_databases[i].connect ( m_database_host , m_database_username , m_database_password , m_database_name ) ) {
            throw "Unable to connect to database" + m_database_host + "," + m_database_username + "," + m_database_password + "," + m_database_name + " )";
        }
	}

	// Set Channels, Connections, and buffers.
	for ( unsigned i = 0 ; i < m_channel_parsers.size ( ) ; i ++ ) {
		m_channel_parsers[i].setSync ( &m_channel_parsers_sync );
		m_channel_parsers[i].setConnections ( &m_connections );
		m_channel_parsers[i].setChannels ( &m_channels );
		m_channel_parsers[i].setQueues ( &m_channelqueue , &m_outqueue );
		m_channel_parsers[i].setDatabase ( &m_channel_databases[i] );
	}

}

Scribble::~Scribble ( ) {
	m_listener.setStatus ( 0 );
}

/*
	run:
		Runner and main start.
*/
void Scribble::run ( ) {
	try {
		// Start IOParsers(WORKERS) for incoming and outgoing data
		for ( unsigned i = 0 ; i < m_io_parsers.size ( ) ; i ++ ) {
			m_io_parsers[i].Start ( &m_io_parsers[i] );
		}

		// Start ChannelParsers(WORKERS) for Channel Packets
		for ( unsigned i = 0 ; i < m_channel_parsers.size ( ) ; i ++ ) {
			m_channel_parsers[i].Start ( &m_channel_parsers[i] );
		}
		
		//	Begin listening for data and new connectionss
		m_listener.start(m_listener_port);
		
	} catch ( LogString &e ) {
		Logit ( "Scribble: " + e );
	}
}
#include "Scribble.h"

Scribble::Scribble ( ) : m_listener ( &m_inqueue ) {
	// Setup 

	// Load in LUA file
	m_config.load ( "config/config.lua" );
		
	// Load in config file values
	m_config.focusVar ( "config" );
	m_script_update_delay = m_config.getTableValue_int ( "script_update_delay" );
	m_listener_port = m_config.getTableValue_int ( "port" );
	m_io_parser_count = m_config.getTableValue_int ( "parser_io" );
	m_max_connections = m_config.getTableValue_int ( "max_connections" );
	
	// Create specified Connection Containers
	m_connections.resize ( m_max_connections ); 
	
	// Create specified IOParsers
	m_io_parsers.resize ( m_io_parser_count );
	
	// Set buffers and Connections information
	for ( unsigned i = 0 ; i < m_io_parsers.size ( ) ; i ++ ) {
		m_io_parsers[i].setConnections ( &m_connections );
		m_io_parsers[i].setQueues ( &m_inqueue, &m_outqueue, &m_channelqueue );
	}
}

Scribble::~Scribble ( ) {
	m_listener.setStatus ( 0 );
	for ( unsigned i = 0 ; i < m_io_parsers.size ( ) ; i ++ ) {
		m_io_parsers[i].setStatus ( 0 );
	}
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
		
		//	Begin listening for data and new connectionss
		m_listener.start(m_listener_port);
		
	} catch ( LogString &e ) {
		Logit ( "Scribble: " + e );
	}
}
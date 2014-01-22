#include "ListenerClass.h"
#include "./common/Logger.h"
#include <cstdio>


ListenerClass::ListenerClass ( ) {

}

ListenerClass::~ListenerClass ( ) {

}

void ListenerClass::setStatus ( int s ) {	
	m_status = s;
}

void ListenerClass::setConnections ( std::vector<Connection> * connections ) {
	m_connections = connections;
}

void ListenerClass::setQueues ( SafeData<IOMsg> * in_queue , SafeData<IOMsg> * disconnection_queue ) {
	m_inqueue = in_queue;
	m_disconnection_queue = disconnection_queue;
}

void ListenerClass::handleConnection ( int socket_fd ){
	try{
		IOMsg packet;
		packet.setConnectionID ( socket_fd );
		// Let IOParser recv the data to distribute work load
		m_inqueue->push ( packet );
		m_inqueue->signal ( ); // Signal workers
	} catch ( LogString &e ) {
		Logit ( "Listener: " + e );
	}
}

void ListenerClass::start ( int listening_port ) {
	try{
		m_status = 1;
		m_port = listening_port;
		setup ( );
		run ( );
	} catch ( LogString &e ) {
		Logit ( "Listener: " + e );
	}
}

void ListenerClass::setup ( ) {
	try{
		// setup maximum events
		m_events_list.resize ( max_proc_events );
		
		// setup maximum connections
		// connections.resize ( max_connections );

		m_event_fd = epoll_create1 ( 0 );//10 Doesn't mean anything after Linux Kern 2.6.8
		
		if ( m_event_fd < 0 ) {
			throw LogString("Unable to create an epoll nest");
		}

		if ( !m_listener_socket.bind(m_port) ) {
			throw LogString("Unable to bind port");
		}
		
		if ( !m_listener_socket.listen(m_queue) ) {
			throw LogString("Unable to listen on port");
		}

		// m_listener_socket.setNonBlocking ( );
		// qm_listener_socket.setTimeout ( m_listener_socket.getSocket() , 25 , 0 );
		
		// Add in listener socket into epoll list for new connections
		addConnection ( m_listener_socket.getSocket ( ) );
		
		Logit ( "Listener: Starting" );
	} catch ( LogString &e ) {
		Logit ( "Listener: " + e );
		std::exit ( 0 );
	}
}

void ListenerClass::run ( ) {
	try{
		int incoming_fd;

		Logit ( "Listener: Started" );
		std::cout << "Listener: Started" << std::endl;
		
		// Leave running while in running status
		while ( m_status ) {
			
			m_events_occuring = epoll_wait ( m_event_fd , m_events_list.data ( ) , m_events_list.size ( ) , -1);
			
			// Check disconnection Queue, and quickly disconnect everyone and remove from queue.
			while ( m_disconnection_queue->wait ( 0 ) == 0 ) {
				IOMsg tmp_packet = m_disconnection_queue->pop ( );
				removeConnection ( tmp_packet.getConnectionID ( ) );
			}

			// make sure there are events
			if ( m_events_occuring <= 0 ) {
				  continue;
			}
			
			for ( int ce = 0 ; ce < m_events_occuring ; ce++ ) {
				// Wrap the descriptor within the TCPClient class so we can organize our calls
				TCPClient tmp_sock(m_events_list[ce].data.fd);
			
				if ( tmp_sock.getSocket() == m_listener_socket.getSocket() ) {
					if ( ( incoming_fd = m_listener_socket.accept() ) > 0) {
						// Logit ( "Listener: New Connection." );
						addConnection ( incoming_fd );
					}
				} else 
				// Peek to see if a disconnection occurred.
				if ( !tmp_sock.peek ( ) ) {
					// Logit ( "Listener: Disconnection" );
					removeConnection ( tmp_sock.getSocket () );
				} else {
					// Logit ( "Listener: Recv data" );
					handleConnection ( tmp_sock.getSocket() );
				}
			}
		}
	} catch ( LogString &e ) {
		Logit ( "Listener: " + e );
		//...hmmm this might not be the best way to handle?
		std::exit ( 0 );
	}
}

void ListenerClass::addConnection ( int socket_fd ) {
	try {
		//Check to see if max connections has been reached.
		int retEv = 0;

		//TCPClient tmp_sock ( socket_fd );
		//tmp_sock.setNonBlocking ( );

		m_ev.events = EPOLLIN | EPOLLET;
		m_ev.data.fd = socket_fd;

		if((retEv = epoll_ctl(m_event_fd, EPOLL_CTL_ADD, socket_fd, &m_ev))!=0) {
			throw LogString ( "Unable to add connection" );
		}
		
		if ( socket_fd == m_listener_socket.getSocket() ) {
			Logit ( "Listener added to epoll.");
			return;
		}

		// Set Connection ( socket_fd ) as a active.
		(*m_connections)[socket_fd].setConnection ( socket_fd );
		// Logit ( "Connection has been added" );
	} catch ( LogString &e ) { 
		Logit("Listener: "+ e);
	}
}

void ListenerClass::removeConnection ( int socket_fd ) {
	try {
		
		if ( epoll_ctl ( m_event_fd , EPOLL_CTL_DEL , socket_fd , NULL ) > 0 ){
			throw LogString ( "Unable to remove connection" );
		}

		// Disconnect socket around the server
		(*m_connections)[socket_fd].reset ( );

		// Logit ( "Disconnection occurred" );
	} catch ( LogString &e ) {
		Logit ( "Listener: " + e );
	}
}

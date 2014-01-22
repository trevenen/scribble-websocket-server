#ifndef LISTENER_HEADER
#define LISTERER_HEADER
#include <iostream>
#include <list>
#include <map>
#include <ctime>
#include <vector>
#include <sys/epoll.h>
#include "common/TCPListener.h"
#include "common/TCPClient.h"
#include "common/SafeData.h"
#include "IOMsg.h"
#include "Connection.h"

// Set max processed events
const int max_proc_events = 50;
const int max_connections = 1000;

class ListenerClass {
	public:
		ListenerClass ( );
		~ListenerClass ( );


		void start ( int );
		void setStatus ( int );
		void setConnections ( std::vector<Connection> * );
		void setQueues ( SafeData<IOMsg> * , SafeData<IOMsg> * );
	private:
		void setup ( );
		void run ( );
		void addConnection ( int );
		void removeConnection ( int );
		void handleConnection ( int );

		int m_status, m_queue, m_port;
		int m_events_occuring, m_event_fd;
		int m_elapse_update_check;

		TCPListener m_listener_socket;
		
		epoll_event m_ev;
		std::vector<epoll_event> m_events_list; 
		std::vector<Connection> * m_connections;
		SafeData<IOMsg> * m_inqueue, * m_disconnection_queue;

};

#endif

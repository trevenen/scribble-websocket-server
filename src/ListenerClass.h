#ifndef LISTENER_HEADER
#define LISTERER_HEADER
#include <iostream>
#include <list>
#include <map>
#include <ctime>
#include <vector>
#include <sys/epoll.h>
#include "./common/TCPListener.h"
#include "./common/TCPClient.h"
#include "./common/SafeData.h"
#include "IOMsg.h"

// Set max processed events
const int max_proc_events = 50;
const int max_connections = 1000;

class ListenerClass {
	public:
		ListenerClass(SafeData<IOMsg>*);
		~ListenerClass();


		void start ( int );
		void setStatus ( int );
		
	private:
		void setup ( );
		void run ( );
		void addConnection ( int );
		void removeConnection ( int );
		void handleConnection ( int );

		int m_incomingFD, m_max_selectors, m_status, m_queue, m_port;
		int m_events_occuring, m_event_fd;
		int m_elapse_update_check;

		TCPListener m_listener_socket;
		
		epoll_event m_ev;
		std::vector<epoll_event> m_events_list; 
		SafeData<IOMsg> * m_inqueue;
};

#endif

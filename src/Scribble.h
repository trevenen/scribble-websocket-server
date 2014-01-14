#ifndef SCRIBBLE_HEADER
#define SCRIBBLE_HEADER
#include <iostream>
#include <vector>
#include "./common/SafeData.h"
#include "./scriptloader/ScriptLoader.h"
#include "IOMsg.h"
#include "IOParser.h"
#include "Connection.h"
#include "ListenerClass.h"

/*
	Scribble:
		Main glue between all core classes.
*/

class Scribble {
	public:
		Scribble ( );
		~Scribble ( );

		void run ( );
	private:
		int m_listener_port;
		int m_script_update_delay;
		int m_io_parser_count;
		int m_max_connections;

		std::vector<IOParser> m_io_parsers;
		std::vector<Connection> m_connections;
		SafeData<IOMsg> m_inqueue, m_outqueue, m_channelqueue;
		ScriptLoader m_config;
		ListenerClass m_listener;	

};


#endif
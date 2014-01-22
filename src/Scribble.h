#ifndef SCRIBBLE_HEADER
#define SCRIBBLE_HEADER
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "common/SafeData.h"
#include "common/LockClass.h"
#include "scriptloader/ScriptLoader.h"
#include "mysql/MySQL.h"
#include "IOMsg.h"
#include "IOParser.h"
#include "Channel.h"
#include "ChannelParser.h"
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

		std::map<std::string,Channel> m_channels;
		
		std::string m_database_host;	
		std::string m_database_username;
		std::string m_database_password;	
		std::string m_database_name; 

		std::vector<IOParser> m_io_parsers;
		std::vector<ChannelParser> m_channel_parsers;

		std::vector<Connection> m_connections; 
		std::vector<MySQL> m_channel_databases;
		
		SafeData<IOMsg> m_inqueue, m_outqueue, m_channelqueue, m_disconnection_queue;
		
		ScriptLoader m_config;
		ListenerClass m_listener;
		LockClass m_channel_parsers_sync;


};


#endif
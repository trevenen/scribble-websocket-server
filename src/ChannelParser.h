#ifndef CHANNEL_PARSER_HEADER
#define CHANNEL_PARSER_HEADER
#include <iostream>
#include <map>
#include <vector>
#include "common/SafeData.h"
#include "common/ThreadClass.h"
#include "common/LockClass.h"
#include "mysql/MySQL.h"
#include "Channel.h"
#include "Connection.h"
#include "IOMsg.h"

class ChannelParser : public ThreadClass {
	public:
		ChannelParser ( );
		~ChannelParser ( );

		void setStatus ( int );
		void setChannels ( std::map<std::string,Channel> * );
		void setConnections ( std::vector<Connection> * );
		void setQueues ( SafeData<IOMsg> * , SafeData<IOMsg> * );
		void setDatabase ( MySQL * );
		void setSync ( LockClass * );
	private:		
		int m_status;
		std::map<std::string,Channel> * m_channels;
		std::vector<Connection> * m_connections;
		SafeData<IOMsg> * m_outqueue, * m_channelqueue;
		MySQL * m_database;
		LockClass * m_sync_lock;

		// Thread Polymorphism
		void Setup ( );
		void Execute ( void* );

		void handleChannelPacket ( IOMsg& );
		void handleConnected ( IOMsg& );
		void handleDisconnected ( IOMsg& );
};

#endif
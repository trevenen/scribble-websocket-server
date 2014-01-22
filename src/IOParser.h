#ifndef IO_PARSER_HEADER
#define IO_PARSER_HEADER
#include <iostream>
#include <vector>
#include "common/ThreadClass.h"
#include "common/Logger.h"
#include "common/SafeData.h"
#include "protocols/WSProtocol.h"
#include "protocols/rfc_6455/RFC_6455.h"
#include "Connection.h"
#include "IOMsg.h"

class IOParser : public ThreadClass {
	public:
		IOParser ( );
		~IOParser ( );

		void setConnections ( std::vector<Connection>* );
		void setQueues ( SafeData<IOMsg>* ,  SafeData<IOMsg>* ,  SafeData<IOMsg>* , SafeData<IOMsg>* );
		void setStatus ( int );

	private:
		int m_status;
		SafeData<IOMsg> *m_inqueue, *m_outqueue, *m_channelqueue, *m_disconnection_queue;
		std::vector<Connection> * m_connections;

		// Data handlers
		void handleIncoming ( int );
		void handleOutgoing ( IOMsg& );
		void handleDisconnection ( IOMsg& );
		// Thread Methods
		void Setup ( );
		void Execute ( void* );

};

#endif
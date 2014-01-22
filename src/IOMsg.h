#ifndef IO_MSG_HEADER
#define IO_MSG_HEADER
#include <vector>
#include <cstdio>
#include "common/SocketClass.h"

enum IOMsgStates {
	NORMAL,
	NOT_ENCODED,
	BROADCAST,
	BROADCAST_ENCODED,
	CONNECTED,
	DISCONNECTED,
	DISCONNECT
};

class IOMsg {
	public:
		IOMsg ( ) {
			m_state = NORMAL; // Default STATE
			m_connection = -1;
			m_payload.clear ( );
		}

		IOMsg ( int connID ) {
			m_state = NORMAL;
			m_connection = connID;
			m_payload.clear ( );
		}

		IOMsg ( int connID , enum IOMsgStates state ) {
			m_connection = connID;
			m_state = state;
			m_payload.clear ( );
		}

		void setConnectionID ( int connID ) {
			m_connection = connID;
		}

		int getConnectionID ( ) {
			return m_connection;
		}

		std::string getConnectionIDstr ( ) {
			std::stringstream stream;
			stream << m_connection;
			return stream.str(); 
		}

		void setState ( enum IOMsgStates state ) {
			m_state = state;
		}

		enum IOMsgStates getState ( ) {
			return m_state;
		}

		std::vector<char>& getBuffer ( ) {
			return m_payload;
		}

		std::vector<char>& getBuffer ( unsigned len ) {
			m_payload.resize ( len );
			return m_payload;
		}

	private:
		int m_connection;
		enum IOMsgStates m_state;
		std::vector<char> m_payload;
};

#endif
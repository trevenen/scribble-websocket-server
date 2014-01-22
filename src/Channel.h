#ifndef CHANNEL_HEADER
#define CHANNEL_HEADER
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include "common/Logger.h"
#include "common/SafeData.h"
#include "common/LockClass.h"
#include "scriptloader/ScriptLoader.h"
#include "mysql/MySQL.h"
#include "IOMsg.h"

/*

Channel:
	Loads in LUA Script which handles interaction between clients
*/

class Channel { 
	public:
		Channel ( );
		Channel ( int , std::string, std::string, std::string, unsigned );
		~Channel ( );
		
		// Setup and Runner methods
		void init ( );

		// Thread-safety
		void lock ( );
		void unlock ( );
		
		// Queue Setup
		void setQueues ( SafeData<IOMsg> * );
		
		// User limit reached
		void limitReached ( int );
		
		// Adding clients
		int addConnection ( int , MySQL * );
		void removeConnection ( int , MySQL * );

		// Events ( Need to be thread safe because updates occur in either )
		void doBeat ( MySQL * );
		void parsePacket ( IOMsg &packet , MySQL * );


		// Writing methods
		int sendTo ( std::string , std::string ); 
		int broadcast ( std::string , std::string );

		// Channel Information
		std::string getName ( );
		std::string getDatabaseKey ( );
		MySQL * getDatabase ( );
		int getID ( );
		
		// LUA Module Interaction
		std::string currentScript ( );
		int updateScript ( std::string );
		
		/*Channel LUA API Callback*/
		static int luaBroadcast ( lua_State * );
		static int luaDisconnectUser ( lua_State * );
		static int luaSendTo ( lua_State * );
		
		/*Logger LUA API Callback*/
		static int luaLog ( lua_State * );
		
		/*DB LUA API Callback*/
		static int luaStore ( lua_State * );
		static int luaGet ( lua_State * );

		static std::string intToString ( int integer ) {
			std::stringstream stream;
			stream << integer;
			return stream.str(); 
		}
	private:
		int m_id;
		unsigned m_max_connections;
		
		std::string m_name;
		std::string m_database_key;
		std::string m_script_file, m_script_update;
		std::list<int> m_channel_connections;
		
		ScriptLoader * m_handler;
		SafeData<IOMsg> * m_outqueue;
		//ScriptLoader m_handler;
		MySQL * m_database;
		LockClass m_lock;

};

#endif

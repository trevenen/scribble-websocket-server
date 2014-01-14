#ifndef CHANNEL_HEADER
#define CHANNEL_HEADER
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <sstream>
#include <sys/epoll.h>
#include "./common/ThreadClass.h"
#include "./common/SemaphoreClass.h"
#include "./common/SocketClass.h"
#include "./common/Logger.h"
#include "./protocols/WSProtocol.h"
#include "./protocols/rfc_6455/RFC_6455.h"
#include "./scriptloader/ScriptLoader.h"
#include "./mysql/MySQL.h"
//#include "./mongodb/DBMongo.h"
#include "Connection.h"

/*

Channel:
	Loads in LUA Script which handles interaction between clients
*/
/*
struct AppDB {
	int auth;
	std::string host;
	std::string dbname;
	std::string username;
	std::string password;
};

typedef struct AppDB AppDB;
*/

class Channel { 
	public:
		//Channel ( int , std::string, std::string, AppDB, std::string, unsigned );
		Channel ( int , std::string, std::string, MySQL* , std::string, unsigned );
		~Channel ( );
		
		//Setup and Runner methods
		void init ( );
		void run ( );
			
		//User limit reached
		void limitReached ( Connection * );
		void setStatus ( int );

		//Adding clients
		void addConnection ( Connection * );
		void removeConnection ( std::string );

		//Events
		void doBeat ( );
		
		//Handling Data
		void handleConnectionBuffers ( );

		//Writing methods
		int sendTo ( std::string , std::string ); 
		int broadcast ( std::string , std::string );

		/*
			MongoDB * getDB();
		*/
		
		//Get DB Handle
		MySQL * getDB();

		//Channel Information
		int usersConnected ( );
		std::string getName ( );
		std::string getDBKey ( );
		int getID ( );
		
		/*LUA Module Interaction*/
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
		
		/*
		static int luaInsert ( lua_State * );
		static int luaUpdate ( lua_State * );
		static int luaQuery ( lua_State * );
		static int luaRemove ( lua_State * );
		static int luaCountQuery ( lua_State * );
		*/

	private:
		int channelID;
		int eventsOccuring, eventFD, incomingFD, status;
		unsigned maxConnections;
		epoll_event ev, *eventsList; 
		ScriptLoader logicModule;

		std::string name; //channel name
		std::string channelDBKey;
		std::string scriptFile, scriptUpdate;
		SemClass sc;
		//DBMongo appDatabase;
		//MySQL appDatabase;
		MySQL * dbConn;

		std::map<std::string, Connection*> connections; 

		//Thread Polymorphism
		//void Setup();
		//void Execute(void*);

};

#endif

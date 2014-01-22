#include "Channel.h"

Channel::Channel ( ) {

}

Channel::Channel ( int id, std::string key, std::string name, std::string script_file, unsigned max_connections ) {
	m_id = id;
	m_name = name;	
	m_database_key = key;
	m_database = NULL;
	m_handler = NULL;
	m_script_file = script_file;
	m_max_connections = max_connections;
}

Channel::~Channel ( ) {
	if ( m_handler ) {
		delete m_handler;
	}
}

void Channel::init ( ) {
	// So ugly.... ew.... refactor this later..
    m_handler = new ScriptLoader ( );

	// Loading in extra libraries
	m_handler->loadLib ( "lualibs/json.lua" );

	// Register Logger LUA API
	m_handler->addProc ( &Channel::luaLog, (void*)this, "log" );
	
	// Register Channel LUA API
	m_handler->addProc ( &Channel::luaBroadcast, (void*)this,"broadcast" ); //Add broadcast
	m_handler->addProc ( &Channel::luaDisconnectUser, (void*)this, "disconnectUser" );
	m_handler->addProc ( &Channel::luaSendTo, (void*)this, "sendTo" );
	
	// Register MYSQL LUA API
	m_handler->addProc ( &Channel::luaStore, (void*)this, "storeid" );
	m_handler->addProc ( &Channel::luaGet, (void*)this, "getid" );
	m_handler->loadText ( m_script_file );

	// Call LUA API Init
	m_handler->call ( "init" );
	
}

void Channel::lock ( ) {
	m_lock.lock ( );
}

void Channel::unlock ( ) {
	m_lock.unlock ( );
}

void Channel::setQueues ( SafeData<IOMsg> * outQ ) {
	m_outqueue = outQ;
}

std::string Channel::currentScript ( ) {
	return m_script_file;
}

int Channel::updateScript ( std::string script ) {
	m_script_update = script;
	return 1;
}

void Channel::parsePacket ( IOMsg &packet , MySQL * database) {
	try {
		
		// Provide a temporary gateway to the Channel Database
		m_database = database;

		// Setup call for API onMessage Event
		SLArg handler_args;
		handler_args.push_back ( packet.getConnectionIDstr ( ) );
		handler_args.push_back ( packet.getBuffer ( ).data ( ) );
		
		// Call API onMessage Event
		//m_handler.call ( "onMessage" , handler_args );		

		// Remove access for the Channel Database
		m_database = NULL;
		
	} catch ( LogString &e ) {
		Logit ( "Channel: " + e );
	}
}

void Channel::doBeat ( MySQL * database ) {
	try {
		// Provide a temporary gateway to the Channel Database
		m_database = database;
		
		// Update current run-time LUA Script.
		if ( !m_script_update.empty () ) {
			m_script_file = m_script_update;
			m_script_update = "";
			// Call LUA and reload script
			m_handler->loadText ( m_script_file );
			
			// Should probably do a call for an onUpdate Event
			// Later. 
		}
			
		// Call onBeat Event to give life to the script :)
		m_handler->call ("onBeat");

		
	} catch ( LogString &e ) {
		Logit ( "Channel: " + e );
	}
}

int Channel::broadcast ( std::string uid, std::string buffer ) {
	std::string msg;
	std::list<int>::iterator it;
	
	for ( it = m_channel_connections.begin(); it != m_channel_connections.end(); it ++ ) {
		//encode message for every connection because the connection version could be different.
		if ( intToString ( *it ).compare ( uid ) != 0 ) {
			IOMsg tmp_packet ( *it );
			::memcpy ( tmp_packet.getBuffer ( buffer.size ( ) ).data ( ) , buffer.data ( ) , buffer.size ( ) );
			m_outqueue->push ( tmp_packet );
			m_outqueue->signal ( );
		}
	}
	return 1;
}

int Channel::sendTo ( std::string uid, std::string buffer ) {
	std::list<int>::iterator it;
	for ( it = m_channel_connections.begin(); it != m_channel_connections.end(); it ++ ) {
		//encode message for every connection because the connection version could be different.
		if ( intToString ( *it ).compare ( uid ) == 0 ) {
			IOMsg tmp_packet ( *it );
			::memcpy ( tmp_packet.getBuffer ( buffer.size ( ) ).data ( ) , buffer.data ( ) , buffer.size ( ) );
			m_outqueue->push ( tmp_packet );
			m_outqueue->signal ( );
			break;
		}
	}

	return 1;
}

void Channel::limitReached ( int socket_fd ) {
	IOMsg tmp_packet ( socket_fd , DISCONNECT );
	m_outqueue->push ( tmp_packet );
	m_outqueue->signal ( );
}

int Channel::addConnection ( int socket_fd , MySQL * database ) {
	try{
		
		m_database = database;
		//Check to see if max connections has been reached.
		if ( m_max_connections <= m_channel_connections.size ( ) ) {
			//If so then disconnect user.
			limitReached ( socket_fd );
			return 0;
		}

		// Add new connection to Channel Connections
		m_channel_connections.push_back ( socket_fd );

		// Send to LUA API onConnect Event
		SLArg args;
		args.push_back ( intToString ( socket_fd ) );
		m_handler->call ( "onConnect", args );
		//Logit ( getName() + ": Connection has been added" );
		
		// Remove database information
		m_database = NULL;
		
	}catch(LogString e){
		Logit ( "Channel: " + e );
	}

	return 1;
}


void Channel::removeConnection ( int socket_fd , MySQL * database ) {
	try {
		
		// Set temporary Database
		m_database = database;

		// Remove Connection from Channel Connections
		m_channel_connections.remove ( socket_fd );

		// Call LUA onDisconnect event with given socket_fd
		SLArg args;
		args.push_back ( intToString ( socket_fd ) );
		m_handler->call ( "onDisconnect", args );

		// Remove database information
		m_database = NULL;
		
		
	} catch ( LogString e ) {
		Logit ( "Channel: " + e );
	}
}

std::string Channel::getName ( ) {
	return m_name;
}

int Channel::getID ( ) {
	return m_id;
}

std::string Channel::getDatabaseKey ( ) {
	return m_database_key;
}

MySQL * Channel::getDatabase ( ) {
	return m_database;
}

// Scribble LUA API Callbacks
int Channel::luaBroadcast ( lua_State* state ) {
	Channel * pthis = (Channel*)lua_touserdata( state, lua_upvalueindex(1));
	std::string connID = lua_tostring ( state, 1 );
	std::string msg = lua_tostring ( state, 2 );

	pthis->broadcast ( connID, msg );
	return 1;
};

int Channel::luaDisconnectUser ( lua_State * state ) {
	Channel * pthis = (Channel*)lua_touserdata ( state, lua_upvalueindex(1) );
	std::string id = lua_tostring ( state, -1 );
	pthis->removeConnection ( atoi ( id.c_str ( ) ) , pthis->getDatabase ( ) );
	return 1;
};


int Channel::luaSendTo ( lua_State * state ) {
	Channel * pthis = (Channel*)lua_touserdata ( state, lua_upvalueindex(1) );
	std::string connID = lua_tostring ( state, 1 );
	std::string msg = lua_tostring ( state, 2 );
	pthis->sendTo ( connID, msg );
	return 1;
};

int Channel::luaLog ( lua_State * state ) {
	Logit ( "Lua: " + std::string ( lua_tostring (state, -1 ) ) );
	return 1;
};

// MySQL API for LUA
// Store a given value as a string with a key and Channel ID
int Channel::luaStore ( lua_State * state ) {
	Channel * pthis = (Channel*) lua_touserdata ( state, lua_upvalueindex (1) );
	MySQL * db = pthis->getDatabase ( );
	
	std::string key = lua_tostring ( state, 1 );
	std::string value = lua_tostring ( state, 2 );

	std::string dbKey = pthis->getDatabaseKey ( );
	
	
	//Check to see if key is inside database
	db->query ( "SELECT * FROM appData WHERE key = '" + key + dbKey + "'" );
	if ( db->hasNext ( ) ) {
		//It has values
		//Update Value
		if ( !db->exec ( "UPDATE appData SET value = '" + value + "' WHERE key = '" +  key + dbKey + "'" ) ) {
			//error
			lua_pushnumber ( state, 0 );
		}
	} else {
		//No values found
		//Insert a new Key with Value
		if ( !db->exec ( "INSERT INTO appData  ( key, value ) VALUES ( '" + key + dbKey + "','" + value + "' )" ) ) {
			//error
			lua_pushnumber ( state, 0 );
		}
	}

	lua_pushnumber ( state, 1 );
	return 1;
}

//Story a given value as a string with a key and Channel ID
int Channel::luaGet ( lua_State * state ) {
	Channel * pthis = (Channel*) lua_touserdata ( state, lua_upvalueindex (1) );
	MySQL * db = pthis->getDatabase ( );
	
	std::string key = lua_tostring ( state, 1 );
	std::string value = lua_tostring ( state, 2 );


	std::string dbKey = pthis->getDatabaseKey ( );

	//Check to see if key is inside database
	if ( db->query ( "SELECT value FROM appData WHERE key = '" + key + dbKey + "'" ) ) {
		//error
		lua_pushnumber ( state, 0 );
	}
	if ( db->hasNext ( ) ) {
		std::vector<std::string> data = db->next ( );
		lua_pushstring ( state, data[0].c_str() );
	} else {
		//Return successfully
		lua_pushnumber ( state, 1 );
	}

	return 1;
}
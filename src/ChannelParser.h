#ifndef CHANNEL_PARSER_HEADER
#define CHANNEL_PARSER_HEADER
#include "common/ThreadClass.h"

class ChannelParser : public ThreadClass {
	public:
		ChannelParser ( );
		~ChannelParser ( );

	private:
		// Pointer to all the connections
		std::map<std::string, Connection*> * connections; 

		// Thread Polymorphism
		void Setup ( );
		void Execute ( void* );


};

#endif
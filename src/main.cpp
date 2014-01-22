#include <iostream>
#include <string>
#include "Scribble.h"

int main ( int argc , char ** args ){
	try{
		
		Scribble scribble;
		scribble.run ( );
		std::cout<<"Scribble is shutting down."<<std::endl;
		
	} catch ( LogString &e ) {
		// Hopefully this will never be called.
		Logit ( "Main: " + e );
	} catch (const char * e) {
		// Severe because error is not typed.
		Logit ( "Main: (SEVERE) " + LogString ( e ) );
	}

	return 0;
}

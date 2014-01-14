#include <iostream>
#include <string>
#include "Scribble.h"

int main ( int argc , char ** args ){
	try{
		
		Scribble scribble;
		scribble.run ( );
		std::cout<<"Scribble is shutting down."<<std::endl;
		
	} catch (const char * e) {
		//Expand on this more.
		std::cout<<"Error: "<<e<<std::endl;
	}

	return 0;
}

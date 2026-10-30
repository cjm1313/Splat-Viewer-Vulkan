#include <app/app.h>





int main(void){

    try{
        App splatViewer;
        splatViewer.run();
    }
    catch (const std::exception &e){
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
    
}
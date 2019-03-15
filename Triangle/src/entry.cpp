//#include "./triangle.hpp"
#include "Pbr.hpp"
int main() {
	//Triangle app;
	Pbr app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
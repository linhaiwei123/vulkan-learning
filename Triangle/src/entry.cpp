//#include "./triangle.hpp"
//#include "./Pbr.hpp"
#include "./GpuParticle.hpp"
int main() {
	//Triangle app;
	//Pbr app;
	GpuParticle app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
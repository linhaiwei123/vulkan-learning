#include <node_api.h>
#include <Triangle.hpp>

namespace nodejsBinding {

	napi_value run(napi_env env, napi_callback_info info) {

		// main
		Triangle app;
		app.run();

		napi_value result;
		assert(napi_create_int32(env, 0, &result) == napi_ok);
		return result;
	}

	static napi_value init(napi_env env, napi_value result) {
		// run 
		napi_property_descriptor desc = { "run", 0, run, 0, 0, 0, napi_default, 0 };
		assert(napi_define_properties(env, result, 1, &desc) == napi_ok);
		
		return result;
	}

	NAPI_MODULE(NodeJsBinding, init)
}
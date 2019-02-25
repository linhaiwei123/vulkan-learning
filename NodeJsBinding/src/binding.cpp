#include <node.h>
#include <Triangle.hpp>

namespace nodejsBinding {

	void run(const v8::FunctionCallbackInfo<v8::Value>& args) {
		// main
		Triangle app;
		app.run();

		v8::Isolate* isolate = args.GetIsolate();
		v8::Local<v8::Number> num = v8::Number::New(isolate, 0);

		args.GetReturnValue().Set(num);
	}

	void init(v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "run", run);
	}

	NODE_MODULE(NodeJsBinding, init)
}
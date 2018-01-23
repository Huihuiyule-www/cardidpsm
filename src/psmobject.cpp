﻿#include <node.h>
#include <iostream>
#include "psm_data.h"
#include "psmobject.h"

using namespace std;

using namespace v8;

Persistent<Function> PsmObject::constructor;

int initFlag = 0;

PsmObject::PsmObject(string dir) {
	if (initFlag<=0){
		initFlag = psm_init(dir);
	}
}

PsmObject::~PsmObject() {
}

void PsmObject::Init(v8::Isolate* isolate) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "PsmObject"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "cardParse", CardParse);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInitFlag", GetInitFlag);

	constructor.Reset(isolate, tpl->GetFunction());
}

void PsmObject::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new MyObject(...)`
		std::string buffer;
		if (args.Length() >= 1 && args[0]->IsString()) {
			v8::String::Utf8Value result(args[0]->ToString());
			buffer = std::string(*result, result.length());
		}
		PsmObject* obj = new PsmObject(buffer);
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else {
		//// Invoked as plain function `MyObject(...)`, turn into construct call.
		// v6/7.x
		//const int argc = 1;
		//Local<Value> argv[argc] = { args[0] };
		//Local<Context> context = isolate->GetCurrentContext();
		//Local<Function> cons = Local<Function>::New(isolate, constructor);
		//Local<Object> result =cons->NewInstance(context, argc, argv).ToLocalChecked();
		//args.GetReturnValue().Set(result);
		// v4.x
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}

void PsmObject::NewInstance(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	// v6/7.x
	/*const unsigned argc = 1;
	Local<Value> argv[argc] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	Local<Context> context = isolate->GetCurrentContext();
	Local<Object> instance =cons->NewInstance(context, argc, argv).ToLocalChecked();*/
	// v4.x
	const unsigned argc = 1;
	Local<Value> argv[argc] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	Local<Object> instance = cons->NewInstance(argc, argv);

	args.GetReturnValue().Set(instance);
}

void PsmObject::CardParse(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (initFlag <= 0) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "initFlag<0,pelese check the ini file dir")));
		return;
	}
	else if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments,should string and int")));
		return;
	}
	Local<Object> reobj = Object::New(isolate);
	v8::String::Utf8Value result(args[0]->ToString());
	std::string buffer = std::string(*result, result.length());
	int policy = args[1]->Int32Value();
	int cnum = 0, bnum = 0;
	std::string cstr, bstr;
	int n = card_search(buffer.c_str(), buffer.length(), policy, bnum, cnum, bstr, cstr);

	Local<Number> numbank = Int32::New(isolate, bnum);
	reobj->Set(String::NewFromUtf8(isolate, "numBank"), numbank);

	Local<Number> numidcard = Int32::New(isolate, cnum);
	reobj->Set(String::NewFromUtf8(isolate, "numIdCard"), numidcard);

	Local<Number> numtotal = Int32::New(isolate, n);
	reobj->Set(String::NewFromUtf8(isolate, "numTotal"), numtotal);

	reobj->Set(String::NewFromUtf8(isolate, "strBank"), String::NewFromUtf8(isolate, bstr.c_str()));

	reobj->Set(String::NewFromUtf8(isolate, "strIdCard"), String::NewFromUtf8(isolate, cstr.c_str()));

	args.GetReturnValue().Set(reobj);
}

void PsmObject::GetInitFlag(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	args.GetReturnValue().Set(Int32::New(isolate, initFlag));
}
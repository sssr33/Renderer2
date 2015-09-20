#pragma once
#include "Operation.h"
#include "..\Containers\ConcurrentQueue.h"

template<class Service, class Data>
class ServiceBaseArgs : public OperationBaseArgs < ServiceBaseArgs<Service, Data> > {
public:
	ServiceBaseArgs(Service &service, Data &data)
		: service(service), data(data){
	}

	Service &service;
	Data &data;
};

template<class Service, class Data>
class ServiceArgs : public OperationArgs<ServiceArgs<Service, Data>, ServiceBaseArgs<Service, Data>>{
public:
	ServiceArgs(Service &service, Data &data)
		: baseArgs(service, data){
	}

	void UnhandledExceptionHandler(std::exception_ptr exception){
		int stop = 342;
	}

	typename OperationBaseArgs<ServiceBaseArgs<Service, Data>>::Impl &GetBaseArgs(){
		return baseArgs;
	}
private:
	ServiceBaseArgs<Service, Data> baseArgs;
};

template<class Data>
class Service{
public:
	typedef ServiceBaseArgs<Service<Data>, Data> OpBaseArgs;
	typedef ServiceArgs<Service<Data>, Data> OpArgs;
	typedef IOperation<OpArgs, OpBaseArgs> IOp;
	template<class Impl, class Ret>
	class OpBase : public OperationBase < Impl, Ret, OpArgs, OpBaseArgs > {
	public:
		OpBase(){
		}

		virtual ~OpBase(){
		}
	};

	Service()
	: opArgs(*this, data){
	}

	void TestRun(IOp *op){
		op->Run(this->opArgs);
	}

private:
	Data data;
	OpArgs opArgs;
};
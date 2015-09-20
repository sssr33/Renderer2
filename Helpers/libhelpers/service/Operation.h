#pragma once

#include <future>
#include <exception>

template<class Impl>
class OperationBaseArgs{
public:
	typedef Impl Impl;
protected:
	~OperationBaseArgs(){
	}
};

template<class Impl, class ImplBase>
class OperationArgs{
public:
	void UnhandledExceptionHandler(std::exception_ptr exception){
		static_cast<Impl *>(this)->UnhandledExceptionHandler(exception);
	}

	typename OperationBaseArgs<ImplBase>::Impl &GetBaseArgs(){
		return static_cast<Impl *>(this)->GetBaseArgs();
	}
protected:
	~OperationArgs(){
	}
};

template<class ArgsImpl, class ArgsBaseImpl>
class IOperation{
public:
	IOperation(){
	}

	virtual ~IOperation(){
	}

	virtual bool UseUnhandledExceptionHandler() = 0;

	virtual void Run(OperationArgs<ArgsImpl, ArgsBaseImpl> &args) = 0;
};

template<class Impl, class Ret, class ArgsImpl, class ArgsBaseImpl>
class OperationBase : public IOperation<ArgsImpl, ArgsBaseImpl>{
public:
	typedef Ret Ret;
	typedef typename OperationBaseArgs<ArgsBaseImpl>::Impl& ServiceArgs;

	OperationBase()
		: useUnhandledExceptionHandler(true){
	}

	virtual ~OperationBase(){
	}

	std::future<Ret> GetResult(){
		this->useUnhandledExceptionHandler = false;
		return this->prom.get_future();
	}

	virtual bool UseUnhandledExceptionHandler() override{
		return this->useUnhandledExceptionHandler;
	}

	virtual void Run(OperationArgs<ArgsImpl, ArgsBaseImpl> &args){
		try{
			this->prom.set_value(std::move(this->RunImpl(args.GetBaseArgs())));
		}
		catch (...){
			if (this->useUnhandledExceptionHandler){
				args.UnhandledExceptionHandler(std::current_exception());
			}
			else{
				this->prom.set_exception(std::current_exception());
			}
		}
	}

	Ret RunImpl(ServiceArgs args){
		return static_cast<Impl *>(this)->RunImpl(args);
	}

private:
	std::promise<Ret> prom;
	bool useUnhandledExceptionHandler;
};
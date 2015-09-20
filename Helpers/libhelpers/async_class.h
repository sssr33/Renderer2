#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

// helper for classes that use async operations and need to wait in destructor while some of it is running.
class async_class{
public:
	async_class()
		: shuttingDown(false), asyncOpCount(0){
	}
protected:
	// wrapper for methods that starts async operations
	template<class Callable> void BeginAsyncOp(const Callable &c){
		bool run = false;

		{
			std::unique_lock<std::mutex> lk(this->mtx);
			if (!this->shuttingDown){
				this->asyncOpCount++;
				run = true;
			}
		}

		// run without mtx lock to avoid recursive locking
		if (run){
			c();
		}
	}

	// wrapper for methods that ends async operations
	template<class Callable> void EndAsyncOp(const Callable &c){
		// run without mtx lock to avoid recursive locking
		c();

		{
			std::unique_lock<std::mutex> lk(this->mtx);
			this->asyncOpCount--;
			if (this->shuttingDown){
				this->cv.notify_all();
			}
		}
	}

	// wrapper for part of the destructor that deletes objects that may be used by async operations.
	template<class Callable> void SafeDtor(const Callable &c){
		// run without mtx lock to avoid recursive locking
		c();

		{
			std::unique_lock<std::mutex> lk(this->mtx);
			this->shuttingDown = true;
			this->cv.wait(lk, [&](){ return this->asyncOpCount == 0; });
		}
	}
private:
	std::mutex mtx;
	std::condition_variable cv;
	bool shuttingDown;
	int asyncOpCount;
};
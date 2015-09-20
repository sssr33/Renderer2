#pragma once

#include <ppltasks.h>

namespace Concurrency{
	template<class T>
	class safe_task{
	public:
		concurrency::critical_section waitCs;

		safe_task()
			: valid(false){
		}
		safe_task(concurrency::task<T> &&t)
			: valid(true), t(std::move(t)){
		}

		~safe_task(){
			concurrency::critical_section::scoped_lock lk(this->waitCs);
		}

		safe_task &operator=(concurrency::task<T> &&t){
			this->valid = true;
			this->t = std::move(t);

			return *this;
		}

		concurrency::task<T> *operator->(){
			return &this->t;
		}

		const concurrency::task<T> *operator->() const{
			return &this->t;
		}

		operator concurrency::task<T>*(){
			return &this->t;
		}

		operator const concurrency::task<T>*() const{
			return &this->t;
		}

		operator bool(){
			return this->valid;
		}

		void wait(){
			concurrency::critical_section::scoped_lock lk(this->waitCs);
		}
	private:
		bool valid;
		concurrency::task<T> t;
	};
}
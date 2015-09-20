#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<class T, class EventsImpl>
class ConcurrentQueueBase{
public:
	ConcurrentQueueBase()
		: waitStopped(false){
	}

	void Push(const T &v){
		std::unique_lock<std::mutex> lk(this->qMtx);

		this->PushInternal(v);
	}

	void Push(T &&v){
		std::unique_lock<std::mutex> lk(this->qMtx);

		this->PushInternal(std::move(v));
	}

	bool Pop(T &v, bool wait){
		bool haveValue = false;
		std::unique_lock<std::mutex> lk(this->qMtx);

		if (wait){
			while (this->q.empty() && !this->waitStopped){
				this->qCv.wait(lk);
			}
		}

		if (!this->q.empty()){
			haveValue = true;
			v = std::move(this->q.front());
			this->q.pop();

			this->PoppedImpl(v);
		}

		return haveValue;
	}

	bool Empty(){
		std::unique_lock<std::mutex> lk(this->qMtx);
		return this->q.empty();
	}


	/*
	Wake-up all threads and disable waiting.
	By default waiting is enabled.
	*/
	void StopWait(){
		std::unique_lock<std::mutex> lk(this->qMtx);
		this->waitStopped = true;
		this->qCv.notify_all();

		this->WaitStoppedImpl();
	}

	/*
	Enable waiting.
	By default waiting is enabled.
	*/
	void ContinueWait(){
		std::unique_lock<std::mutex> lk(this->qMtx);
		this->waitStopped = false;

		this->WaitStartedImpl();
	}

	void Clear(){
		std::unique_lock<std::mutex> lk(this->qMtx);

		this->q = std::queue<T>();
		this->ClearedImpl();
	}
protected:
	std::mutex qMtx;

	void PoppedImpl(T &v){
		static_cast<EventsImpl *>(this)->PoppedImpl(v);
	}
	void WaitStoppedImpl(){
		static_cast<EventsImpl *>(this)->WaitStoppedImpl();
	}
	void WaitStartedImpl(){
		static_cast<EventsImpl *>(this)->WaitStartedImpl();
	}
	void ClearedImpl(){
		static_cast<EventsImpl *>(this)->ClearedImpl();
	}

	void PushInternal(const T &v){
		this->q.push(v);
		this->qCv.notify_one();
	}

	void PushInternal(T &&v){
		this->q.push(std::move(v));
		this->qCv.notify_one();
	}

	bool GetWaitStopped() const{
		return this->waitStopped;
	}
private:
	std::queue<T> q;
	std::condition_variable qCv;
	bool waitStopped;
};

// Default ConcurrentQueue implementation
template<class T>
class ConcurrentQueue : public ConcurrentQueueBase < T, ConcurrentQueue<T> > {
public:
	// Empty events for default ConcurrentQueue implementation
	// When you need to implement EventsImpl use this class(ConcurrentQueueEmptyEvents) as start point.
	// All events will be protected by ConcurrentQueueBase::qMtx
	void PoppedImpl(T &v){}
	void WaitStoppedImpl(){}
	void WaitStartedImpl(){}
	void ClearedImpl(){}
};
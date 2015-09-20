#pragma once

#include <cstdint>
#include <vector>

template<class T, class Alloc = std::allocator<T>>
class vector_pool{
public:
	uint32_t push(const T &v){
		uint32_t idx;

		if (!this->freeMem.empty()){
			idx = this->freeMem.back();
			this->freeMem.pop_back();
			this->mem[idx] = v;
		}
		else{
			idx = static_cast<uint32_t>(this->mem.size());
			this->mem.push_back(v);
		}

		return idx;
	}

	uint32_t push(T &&v){
		uint32_t idx;

		if (!this->freeMem.empty()){
			idx = this->freeMem.back();
			this->freeMem.pop_back();
			this->mem[idx] = std::move(v);
		}
		else{
			idx = static_cast<uint32_t>(this->mem.size());
			this->mem.push_back(std::move(v));
		}

		return idx;
	}

	T &get(uint32_t idx){
		return this->mem[idx];
	}

	void remove(uint32_t idx){
		this->freeMem.push_back(idx);
	}
private:
	std::vector<T, Alloc> mem;
	std::vector<uint32_t> freeMem;
};
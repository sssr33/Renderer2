#pragma once

#include <algorithm>
#include <cstdint>

template <class D, class S>
class array_wrapper{
public:

	array_wrapper()
		: data(nullptr), size(0){
	}

	array_wrapper(D *data, S size)
		: data(data), size(size){
	}

	array_wrapper(const array_wrapper &other)
		: data(other.data), size(other.size){
	}

	array_wrapper(array_wrapper &&other)
		: data(std::move(other.data)), size(std::move(other.size)){
	}

	array_wrapper &operator= (const array_wrapper &other){
		if (this != &other){
			this->data = other.data;
			this->size = other.size;
		}
		return *this;
	}

	array_wrapper &operator= (array_wrapper &&other){
		if (this != &other){
			this->data = std::move(other.data);
			this->size = std::move(other.size);
		}
		return *this;
	}

	D *data;
	S size;
};
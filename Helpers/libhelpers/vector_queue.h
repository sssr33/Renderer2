#pragma once

#include <vector>
#include <cstdint>
#include <assert.h>

template<class T, class Alloc = std::allocator<T>>
class vector_queue{
	typedef std::vector<T, Alloc> vecType;
public:
	typedef typename vecType::value_type                value_type;
	typedef typename vecType::allocator_type            allocator_type;
	typedef typename vecType::size_type                 size_type;
	typedef typename vecType::difference_type           difference_type;
	typedef typename vecType::reference                 reference;
	typedef typename vecType::const_reference           const_reference;
	typedef typename vecType::pointer                   pointer;
	typedef typename vecType::const_pointer             const_pointer;
	typedef typename vecType::iterator                  iterator;
	typedef typename vecType::const_iterator            const_iterator;
	typedef typename vecType::reverse_iterator          reverse_iterator;
	typedef typename vecType::const_reverse_iterator    const_reverse_iterator;

	vector_queue()
		: beginOffset(0){}

	iterator begin(){
		return this->vec.begin() + this->beginOffset;
	}

	const_iterator begin() const{
		return this->vec.begin() + this->beginOffset;
	}

	const_iterator cbegin() const{
		return this->vec.cbegin() + this->beginOffset;
	}

	iterator end(){
		return this->vec.end();
	}

	const_iterator end() const{
		return this->vec.end();
	}

	const_iterator cend() const{
		return this->vec.cend();
	}

	void push(const T& value){
		this->vec.push_back(value);
	}

	void push(T&& value){
		this->vec.push_back(value);
	}

	void push(size_type count, const T& value){
		this->vec.insert(this->vec.end(), count, value);
	}

	template<class InputIt>
	void push(InputIt first, InputIt last){
		this->vec.insert(this->vec.end(), first, last);
	}

	T pop(){
		T v = *this->begin();
		this->beginOffset++;

		if (this->empty()){
			this->clear();
		}

		return v;
	}

	template<class InputIt>
	void pop(InputIt first, InputIt last){
		for (; first != last; ++first, this->beginOffset++){
			*first = *this->begin();
		}

		if (this->empty()){
			this->clear();
		}
	}

	bool empty() const{
		return this->begin() == this->end();
	}

	void shrink_to_fit(){
		this->vec.shrink_to_fit();
	}

	size_type size() const{
		return this->end() - this->begin();
	}

	void clear(){
		this->beginOffset = 0;
		this->vec.clear();
	}

private:
	size_type beginOffset;
	vecType vec;
};
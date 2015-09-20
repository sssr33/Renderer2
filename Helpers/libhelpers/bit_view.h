#pragma once
#include "H.h"

#include <cstdint>
#include <limits>
#include <assert.h>

class big_endian_block_order{
public:
	static bool get_bit(const uint8_t *data, size_t bitIdx){
		bool res = (data[0] & (static_cast<uint8_t>(0x80) >> bitIdx)) != 0;
		return res;
	}

	static void set_bit(uint8_t *data, size_t bitIdx, bool v){
		if (v){
			data[0] |= static_cast<uint8_t>(0x80) >> bitIdx;
		}
		else{
			data[0] &= ~(static_cast<uint8_t>(0x80) >> bitIdx);
		}
	}
};

class little_endian_block_order{
public:
	static bool get_bit(const uint8_t *data, size_t bitIdx){
		bool res = (data[0] & (static_cast<uint8_t>(0x01) << bitIdx)) != 0;
		return res;
	}

	static void set_bit(uint8_t *data, size_t bitIdx, bool v){
		if (v){
			data[0] |= static_cast<uint8_t>(0x01) << bitIdx;
		}
		else{
			data[0] &= ~(static_cast<uint8_t>(0x01) << bitIdx);
		}
	}
};

template<class block_order = little_endian_block_order>
class bit_view{
	static const size_t block_size = 8;

	class reference{
	public:
		reference(uint8_t *data, size_t bit_idx)
			: data(data), bit_idx(bit_idx){
			H::System::Assert(this->bit_idx < bit_view::block_size);
		}

		reference &operator=(bool v){
			H::System::Assert(this->bit_idx < bit_view::block_size);
			block_order::set_bit(this->data, this->bit_idx, v);
			return *this;
		}

		reference &operator=(const reference &other){
			H::System::Assert(this->bit_idx < bit_view::block_size);
			byte_order::set_bit(this->data, this->bit_idx, bool(other));
			return (*this);
		}

		operator bool(){
			H::System::Assert(this->bit_idx < bit_view::block_size);
			return block_order::get_bit(this->data, this->bit_idx);
		}
	private:
		uint8_t *data;
		size_t bit_idx;
	};

	class pointer{
		friend class bit_view;
	public:
		pointer(bit_view &parent, size_t block_idx, size_t bit_idx)
			: parent(parent), block_idx(block_idx), bit_idx(bit_idx){
		}

		pointer(const pointer &other)
			: parent(other.parent), block_idx(other.block_idx), bit_idx(other.bit_idx){
		}

		pointer &operator=(const pointer &other){
			if (this != &other){
				this->parent = other.parent;
				this->block_idx = other.block_idx;
				this->bit_idx = other.bit_idx;
			}

			return *this;
		}

		bool operator*(){
			H::System::Assert(this->bit_idx < bit_view::block_size);
			return byte_order::get_bit(&this->parent.data[this->block_idx], this->bit_idx);
		}

		pointer operator++(){
			pointer i = *this;

			this->bit_idx++;

			if (this->bit_idx >= bit_view::block_size){
				this->bit_idx = 0;
				this->block_idx++;
			}

			return i;
		}

		pointer operator++(int junk){
			this->bit_idx++;

			if (this->bit_idx >= bit_view::block_size){
				this->bit_idx = 0;
				this->block_idx++;
			}

			return *this;
		}
	private:
		bit_view &parent;
		size_t block_idx;
		size_t bit_idx;
	};
public:

	bit_view()
		: data(nullptr), bit_size(0){}

	bit_view(void *data, size_t bit_size)
		: bit_size(bit_size){
		this->data = static_cast<uint8_t *>(data);
	}

	~bit_view(){
	}

	void set_data(void *data, size_t bit_size){
		this->data = static_cast<uint8_t *>(data);
		this->bit_size = bit_size;
	}

	bool operator[](const size_t pos) const{
		H::System::Assert(pos < this->bit_size);
		size_t block_idx = pos / bit_view::block_size;
		size_t bit_idx = pos % bit_view::block_size;
		return block_order::get_bit(&this->data[block_idx], bit_idx);
	}

	reference operator[] (const size_t pos){
		H::System::Assert(pos < this->bit_size);
		size_t block_idx = bit_view::get_block_idx(pos);
		size_t bit_idx = pos % bit_view::block_size;
		return reference(&this->data[block_idx], bit_idx);
	}

	size_t size() const{
		return this->bit_size;
	}

	static size_t get_block_idx(const size_t bit_pos){
		return bit_pos / bit_view::block_size;
	}
private:
	size_t bit_size;
	uint8_t *data;
};
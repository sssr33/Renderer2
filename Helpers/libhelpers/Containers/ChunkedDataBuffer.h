#pragma once

#include <cstdint>
#include <vector>
#include <list>
#include <assert.h>
// hello from submodule!!!
// http://johnleach.co.uk/words/323/git-submodules-in-n-easy-steps
template<class T, size_t ChunkSize = 1024 * 4>
class ChunkedDataBuffer{
public:

	ChunkedDataBuffer()
		: frontChunkBeg(0){
	}

	void PushBack(const T &v){
		this->AddChunkIfNeeded();
		this->storage.back().push_back(v);
	}

	void PushBack(T &&v){
		this->AddChunkIfNeeded();
		this->storage.back().push_back(std::move(v));
	}

	template<class It>
	void PushBack(It first, It last){
		auto curIt = first;

		while (curIt != last){
			this->AddChunkIfNeeded();

			while (this->storage.back().size() != ChunkSize && curIt != last){
				this->storage.back().push_back(*curIt);
				curIt++;
			}
		}
	}

	void PopFront(size_t count = 1){
		if (!this->Empty()){
			size_t chunksToRemove = (count + this->frontChunkBeg) / ChunkSize;

			for (size_t i = 0; i < chunksToRemove && !this->storage.empty(); i++){
				this->storage.pop_front();
			}

			if (this->storage.empty()){
				this->frontChunkBeg = 0;
			}
			else{
				this->frontChunkBeg = (count + this->frontChunkBeg) % ChunkSize;
			}
		}
	}

	T &Front(){
		assert(!this->Empty());
		return this->storage.front()[this->frontChunkBeg];
	}

	template<class It>
	size_t Front(It first, It last){
		size_t copied = 0;
		auto curIt = first;
		auto storageIt = this->storage.begin();
		auto chunkIdx = this->frontChunkBeg;

		while (curIt != last && storageIt != this->storage.end()){
			while (chunkIdx < storageIt->size() && curIt != last){
				*curIt = (*storageIt)[chunkIdx];

				chunkIdx++;
				curIt++;
				copied++;
			}

			chunkIdx = 0;
			storageIt++;
		}

		return copied;
	}

	bool Empty() const{
		return this->storage.empty() || this->storage.back().size() == this->frontChunkBeg;
	}

	size_t Size() const{
		size_t sz = 0;

		if (this->storage.size() > 0){
			// have at least front() chunk
			sz += this->storage.front().size() - this->frontChunkBeg;

			if (this->storage.size() > 1){
				// have at least back() chunk
				sz += this->storage.back().size();

				if (this->storage.size() > 2){
					// have this->storage.size() - 2 full chunks
					sz += (this->storage.size() - 2) * ChunkSize;
				}
			}
		}

		return sz;
	}

private:
	typedef std::vector<T> chunk;
	typedef std::list<chunk> data_storage;

	data_storage storage;
	size_t frontChunkBeg;

	void AddChunkIfNeeded(){
		if (storage.empty() || storage.back().size() == ChunkSize){
			storage.push_back(chunk());
			storage.back().reserve(ChunkSize);
		}
	}
};
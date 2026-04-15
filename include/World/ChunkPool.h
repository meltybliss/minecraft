#pragma once
#include "Chunk.h"
#include <stdint.h>
#include <vector>

struct PoolBlock {
	Chunk* baseAddr = nullptr;
	unsigned int capacity = 0;
};

struct SlotRef {
	int blockIdx;
	int slotIdx;

};

class ChunkPool {

	std::vector<PoolBlock> blocks;//chunk256ŒÂ‚Å1
	std::vector<SlotRef> freelist;//chunk‚Ì”‚ ‚é

	static constexpr unsigned int BLOCK_WIDTH = 256;//Blockˆê‚Â‚Échunk‚ª‰½ŒÂ“ü‚é‚©

public:

	ChunkPool(int chunkCount) {
		int blockCount = (chunkCount + (BLOCK_WIDTH - 1)) / BLOCK_WIDTH;

		blocks.reserve(blockCount);
		freelist.reserve(chunkCount);

		for (int i = 0; i < blockCount; i++) {
			Chunk* mem = static_cast<Chunk*>(::operator new(sizeof(Chunk) * BLOCK_WIDTH));
			blocks.emplace_back(PoolBlock{ mem, BLOCK_WIDTH });
		}

		for (int bi = 0; bi < blockCount; bi++) {

			for (int si = 0; si < BLOCK_WIDTH; si++) {
				freelist.emplace_back(SlotRef{ bi, si });
			}
		}

	}

	~ChunkPool() {
		for (const auto& b : blocks) {
			::operator delete(b.baseAddr);
		}
	}

	Chunk* Allocate() {
		if (!freelist.empty()) {
			SlotRef slot = freelist.back();
			freelist.pop_back();

			Chunk* addr = blocks[slot.blockIdx].baseAddr + slot.slotIdx;

			new (addr) Chunk();//placement new
			
			return addr;
		}

		int bi = static_cast<int>(blocks.size());
		Chunk* mem = static_cast<Chunk*>(::operator new(sizeof(Chunk) * BLOCK_WIDTH));
		blocks.emplace_back(PoolBlock{ mem, BLOCK_WIDTH });

		for (int si = 1; si < BLOCK_WIDTH; si++) {

			freelist.emplace_back(SlotRef{ bi, si });
		}

		Chunk* addr = blocks[bi].baseAddr + 0;

		new (addr) Chunk();

		return addr;
	}

	std::vector<Chunk*> AllocateMulti(int chunkCount) {
		std::vector<Chunk*> result;
		result.reserve(chunkCount);

		for (int i = 0; i < chunkCount; i++) {
			result.push_back(Allocate());
		}

		return result;

	}

	void Release(Chunk* p) {
		if (!p) return;

		p->~Chunk();

		for (int bi = 0; bi < (int)blocks.size(); bi++) {
			Chunk* begin = blocks[bi].baseAddr;
			Chunk* end = blocks[bi].baseAddr + blocks[bi].capacity;

			if (p >= begin && p < end) {//‘ÎÛ‚Ìpoolblock‚ª“Á’è‚Å‚«‚½‚Ì‚Åslotindex‚à“Á’è‰Â
				int si = p - begin;
				freelist.emplace_back(SlotRef{ bi, si });
				return;
			}

		}

	}

};
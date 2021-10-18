module;

#include <cstdint>
#include <cstddef>

export module rbx.lua.hash;

import mem;

import rbx.lua.object;
import rbx.lua.constants;

export namespace rbx::hash {
	rbx::node* next(rbx::node* node) {
		return reinterpret_cast<rbx::node*>(*reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(node) + 28) >> 4);
	}

	//rbx::tvalue* get_string(rbx::table* table, rbx::string* key) {
	//	std::size_t size_node = 1 << table->log2_sizenode;
	//	std::size_t operation = (key->hash ^ reinterpret_cast<std::uintptr_t>(&key->hash)) & (size_node - 1);
	//	auto node = &(reinterpret_cast<rbx::node*>(reinterpret_cast<std::uintptr_t>(&table->node) + reinterpret_cast<std::uintptr_t>(table->node))[operation]);
	//	
	//	do {
	//		if (node->key.type == rbx::string_t && (&node->key.value.gc->string == key)) {
	//			return &node->value;
	//		}
	// 
	//		node = next(node);
	//	} while (node);

	//	return &rbx::nil_object;
	//}

	rbx::tvalue* get_string(rbx::table* table, rbx::string* key) {
		int v3;
		auto t = reinterpret_cast<std::uintptr_t>(table);
		auto tskey = reinterpret_cast<std::uintptr_t>(key);

		for (std::uintptr_t i10 = *(std::uintptr_t*)(t + 28)
			+ 32 * (((tskey + 12) ^ *(std::uintptr_t*)(tskey + 12)) & ((1 << *(std::uint8_t*)(t + 9)) - 1))
			- (t + 28); ; i10 += 32 * v3)
		{

			if ((*(std::uint8_t*)(i10 + 28) & 0xF) == 5 && *(std::uintptr_t*)(i10 + 16) == tskey)
				return (rbx::tvalue*)i10;

			v3 = *(std::uintptr_t*)(i10 + 28) >> 4;
		}
		return &rbx::nil_object;
	}
}

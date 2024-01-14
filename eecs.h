#ifndef EECS_H
#define EECS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef EECS_API
#	ifdef __cplusplus
#		define EECS_API extern "C"
#	else
#		define EECS_API
#	endif
#endif

#ifndef EECS_ID_TYPE
#	define EECS_ID_TYPE int32_t
#endif

#ifndef EECS_MASK_TYPE
#	define EECS_MASK_TYPE uint64_t
#endif

#ifndef EECS_ALIGN_TYPE
#	define EECS_ALIGN_TYPE max_align_t
#endif

#ifndef EECS_DEFAULT_TABLE_CHUNK_SIZE
#	define EECS_DEFAULT_TABLE_CHUNK_SIZE 16384
#endif

#ifndef EECS_MALLOC
#include <stdlib.h>
#define EECS_MALLOC(CTX, SIZE) malloc(SIZE)
#define EECS_REALLOC(CTX, PTR, NEW_SIZE) realloc(PTR, NEW_SIZE)
#define EECS_FREE(CTX, SIZE) free(SIZE)
#endif

#define EECS_UPDATE_ALL ((eecs_mask_t)-1)
#define EECS_UPDATE_NONE 0
#define EECS_HANDLE_INIT { 0 }
#define EECS_END_OF_LIST { 0 }

#ifndef EECS_ASSERT
#include <assert.h>
#define EECS_ASSERT(COND, MSG) assert((COND) && (MSG))
#endif

typedef EECS_ID_TYPE eecs_id_t;
typedef EECS_MASK_TYPE eecs_mask_t;

typedef struct eecs_s eecs_t;
typedef struct eecs_world_s eecs_world_t;
typedef struct { eecs_id_t from_1_index; eecs_id_t gen; } eecs_entity_t;
typedef struct { eecs_id_t from_1_index; } eecs_component_t;
typedef struct { eecs_id_t from_1_index; } eecs_system_t;
typedef struct { eecs_id_t from_1_index; } eecs_template_t;

typedef struct eecs_batch_s {
	eecs_world_t* world;
	eecs_id_t size;
	void* chunk;
	ptrdiff_t* offsets;
} eecs_batch_t;

typedef void (*eecs_component_fn_t)(
	eecs_world_t* world,
	eecs_entity_t entity,
	void* component_data,
	void* userdata
);

typedef void (*eecs_system_update_fn_t)(
	eecs_world_t* world,
	eecs_batch_t batch,
	void* userdata
);

typedef void (*eecs_system_world_fn_t)(
	eecs_world_t* world,
	void* userdata
);

typedef void (*eecs_system_entity_fn_t)(
	eecs_world_t* world,
	eecs_entity_t entity,
	void* userdata
);

typedef struct eecs_component_init_s {
	eecs_component_t component;
	const void* data;
} eecs_component_init_t;

typedef struct eecs_component_options_s {
	size_t size;
	size_t alignment;
	eecs_component_fn_t init_fn;
	eecs_component_fn_t cleanup_fn;
	void* userdata;
} eecs_component_options_t;

typedef struct eecs_system_options_s {
	void* userdata;
	eecs_mask_t update_mask;
	eecs_component_t* match_components;
	eecs_system_world_fn_t pre_update_fn;
	eecs_system_world_fn_t post_update_fn;
	eecs_system_update_fn_t update_fn;
	eecs_system_world_fn_t init_per_world_fn;
	eecs_system_world_fn_t cleanup_per_world_fn;
	eecs_system_entity_fn_t init_per_entity_fn;
	eecs_system_entity_fn_t cleanup_per_entity_fn;
} eecs_system_options_t;

typedef struct eecs_world_options_s {
	void* memctx;
	void* table_chunk_memctx;
	size_t table_chunk_size;
} eecs_world_options_t;

typedef struct eecs_options_s {
	void* memctx;
} eecs_options_t;

EECS_API eecs_t*
eecs_create(eecs_options_t options);

EECS_API void
eecs_destroy(eecs_t* ecs);

EECS_API void
eecs_register_component(
	eecs_t* ecs,
	eecs_component_t* handle,
	eecs_component_options_t options
);

EECS_API void
eecs_register_system(
	eecs_t* ecs,
	eecs_system_t* handle,
	eecs_system_options_t options
);

EECS_API eecs_world_t*
eecs_create_world(eecs_t* ecs, eecs_world_options_t options);

EECS_API void
eecs_destroy_world(eecs_world_t* world);

EECS_API void
eecs_set_per_world_userdata(
	eecs_world_t* world,
	eecs_system_t system,
	void* userdata
);

EECS_API void*
eecs_get_per_world_userdata(
	eecs_world_t* world,
	eecs_system_t system
);

EECS_API eecs_entity_t
eecs_create_entity(eecs_world_t* world, const eecs_component_init_t* init);

EECS_API void
eecs_morph_entity(
	eecs_world_t* world,
	eecs_entity_t entity,
	const eecs_component_init_t* new_components,
	const eecs_component_t* removed_components
);

EECS_API void
eecs_register_template(
	eecs_world_t* world,
	eecs_template_t* handle,
	const eecs_component_init_t* init
);

EECS_API eecs_entity_t
eecs_create_entity_from_template(
	eecs_world_t* world,
	eecs_template_t entity_template,
	const eecs_component_init_t* overrides
);

EECS_API void
eecs_destroy_entity(eecs_world_t* world, eecs_entity_t entity);

EECS_API void
eecs_activate_entity(eecs_world_t* world, eecs_entity_t entity);

EECS_API void
eecs_deactivate_entity(eecs_world_t* world, eecs_entity_t entity);

EECS_API bool
eecs_is_entity_active(eecs_world_t* world, eecs_entity_t entity);

EECS_API bool
eecs_is_valid_entity(eecs_world_t* world, eecs_entity_t entity);

EECS_API void*
eecs_get_component_in_entity(
	eecs_world_t* world,
	eecs_entity_t entity,
	eecs_component_t component_type
);

EECS_API void
eecs_run_systems(eecs_world_t* world, eecs_mask_t update_mask);

EECS_API void
eecs_run_system(eecs_world_t* world, eecs_mask_t update_mask, eecs_system_t system);

EECS_API eecs_mask_t
eecs_get_current_update_mask(eecs_world_t* world);

EECS_API eecs_id_t
eecs_get_batch_size(eecs_batch_t batch);

EECS_API void*
eecs_get_components_in_batch(eecs_batch_t batch, eecs_id_t match_index);

EECS_API eecs_entity_t
eecs_get_entity_in_batch(eecs_batch_t batch, eecs_id_t index);

#endif

#ifdef EECS_IMPLEMENTATION

#include <string.h>

#define eecs_max(a, b) ((a) > (b) ? (a) : (b))
#define eecs_min(a, b) ((a) < (b) ? (a) : (b))
#define eecs_index_of(handle) ((handle).from_1_index - 1)
#define EECS_PRIVATE static inline

// Memory

EECS_PRIVATE void*
eecs_malloc(void* memctx, size_t size) {
	void* ptr = EECS_MALLOC(memctx, size);
	EECS_ASSERT(ptr != NULL, "Out of memory");
	return ptr;
}

EECS_PRIVATE void
eecs_free(void* memctx, void* ptr) {
	EECS_FREE(memctx, ptr);
}

EECS_PRIVATE void*
eecs_realloc(void* memctx, void* ptr, size_t new_size) {
	void* new_ptr =  EECS_REALLOC(memctx, ptr, new_size);
	EECS_ASSERT(new_ptr != NULL, "Out of memory");
	return new_ptr;
}

// Dynamic array

#define eecs_array(type) type*

#define eecs_array_length(array) \
	eecs_dynamic_array_length(array)

#define eecs_array_capacity(array) \
	eecs_dynamic_array_capacity(array)

#define eecs_array_push(allocator, array, item) \
	do { \
		eecs_id_t length_ = eecs_array_length(array); \
		array = eecs_dynamic_array_prepare_push(allocator, array, sizeof(*array)); \
		array[length_] = (item); \
	} while (0)

#define eecs_array_back(array) array[eecs_array_length(array) - 1]

#define eecs_array_pop(array) array[eecs_dynamic_array_pop(array)]

#define eecs_array_resize(allocator, array, length) \
	eecs_dynamic_array_resize(allocator, array, length, sizeof(*array))

#define eecs_array_clear(array) \
	eecs_dynamic_array_clear(array)

#define eecs_array_free(allocator, array) \
	eecs_free_dynamic_array(allocator, array)

#define eecs_array_indexed_foreach(type, itr, array) \
	for ( \
		struct { eecs_id_t index; type* value; } itr = { 0, array }; \
		itr.index < eecs_array_length(array); \
		++itr.index, itr.value = &(array)[itr.index] \
	)

#define eecs_array_indexed_foreach_rev(type, itr, array) \
	for ( \
		struct { eecs_id_t index; type* value; } itr = { eecs_array_length(array) - 1, &eecs_array_back(array) }; \
		itr.index >= 0; \
		--itr.index, itr.value = &(array)[itr.index] \
	)

#define eecs_insertion_sort(length, array, element_type, cmp_lt) \
	do { \
		for (eecs_id_t sort_i = 1; sort_i < length; ++sort_i) { \
			element_type element_i = array[sort_i]; \
			eecs_id_t sort_j = sort_i; \
			while ((sort_j >= 0) && (cmp_lt(element_i, array[sort_j - 1]))) { \
				array[sort_j] = array[sort_j - 1]; \
				--sort_j; \
			} \
			array[sort_j] = element_i; \
		} \
	} while (0)

#define eecs_component_init_cmp_lt(lhs, rhs) \
	lhs.component.from_1_index < rhs.component.from_1_index

typedef struct eecs_dynamic_array_s {
	eecs_id_t length;
	eecs_id_t capacity;
	_Alignas(EECS_ALIGN_TYPE) char elements[];
} eecs_dynamic_array_t;

EECS_PRIVATE eecs_dynamic_array_t*
eecs_dynamic_array_header(void* array) {
	if (array == NULL) {
		return NULL;
	} else {
		return (void*)((char*)array - offsetof(eecs_dynamic_array_t, elements));
	}
}

EECS_PRIVATE eecs_id_t
eecs_dynamic_array_length(void* array) {
	return array == NULL ? 0 : eecs_dynamic_array_header(array)->length;
}

EECS_PRIVATE eecs_id_t
eecs_dynamic_array_capacity(void* array) {
	return array == NULL ? 0 : eecs_dynamic_array_header(array)->capacity;
}

EECS_PRIVATE void*
eecs_dynamic_array_clear(void* array) {
	if (array != NULL) {
		eecs_dynamic_array_header(array)->length = 0;
	}
	return array;
}

EECS_PRIVATE void*
eecs_dynamic_array_prepare_push(
	void* memctx,
	void* array,
	size_t element_size
) {
	eecs_id_t new_length = eecs_dynamic_array_length(array) + 1;
	eecs_id_t capacity = eecs_dynamic_array_capacity(array);

	eecs_dynamic_array_t* header;
	if (new_length > capacity) {
		eecs_id_t new_capacity = eecs_max(new_length, capacity * 2);

		header = eecs_realloc(
			memctx,
			eecs_dynamic_array_header(array),
			new_capacity * (eecs_id_t)element_size + sizeof(eecs_dynamic_array_t)
		);

		header->capacity = new_capacity;
		// Zero new elements, except for the one about to be written
		memset(
			header->elements + new_length * (eecs_id_t)element_size,
			0,
			(new_capacity - new_length) * (eecs_id_t)element_size
		);
	} else {
		header = eecs_dynamic_array_header(array);
	}

	header->length = new_length;
	return header->elements;
}

EECS_PRIVATE void
eecs_free_dynamic_array(void* memctx, void* array) {
	eecs_free(memctx, eecs_dynamic_array_header(array));
}

EECS_PRIVATE void*
eecs_dynamic_array_resize(
	void* memctx,
	void* array,
	eecs_id_t new_length,
	size_t element_size
) {
	EECS_ASSERT(new_length >= 0, "Invalid length");

	eecs_id_t existing_length = eecs_array_length(array);

	if (new_length == existing_length) {
		return array;
	} else if (new_length < existing_length) {
		eecs_dynamic_array_header(array)->length = new_length;
		return array;
	} else if (new_length <= eecs_array_capacity(array)) {
		// Zero all new elements
		memset(
			(char*)array + existing_length * (eecs_id_t)element_size,
			0,
			(new_length - existing_length) * (eecs_id_t)element_size
		);
		eecs_dynamic_array_header(array)->length = new_length;
		return array;
	} else {
		eecs_dynamic_array_t* header = eecs_realloc(
			memctx,
			eecs_dynamic_array_header(array),
			new_length * (eecs_id_t)element_size + sizeof(eecs_dynamic_array_t)
		);
		// Zero all new elements
		memset(
			header->elements + existing_length * (eecs_id_t)element_size,
			0,
			(new_length - existing_length) * (eecs_id_t)element_size
		);
		header->length = new_length;
		header->capacity = new_length;
		return header->elements;
	}
}

EECS_PRIVATE eecs_id_t
eecs_dynamic_array_pop(void* array) {
	eecs_dynamic_array_t* header = eecs_dynamic_array_header(array);
	return --header->length;
}

// bitset

#include <limits.h>

typedef struct eecs_bitset_s {
	eecs_id_t num_masks;
	eecs_mask_t masks[];
} eecs_bitset_t;

EECS_PRIVATE size_t
eecs_bitset_memory_size(eecs_id_t max_bits) {
	size_t num_bits_per_mask = sizeof(eecs_mask_t) * CHAR_BIT;
	size_t num_masks = ((size_t)max_bits + num_bits_per_mask - 1) / num_bits_per_mask;
	return sizeof(eecs_bitset_t) + sizeof(eecs_mask_t) * num_masks;
}

EECS_PRIVATE void
eecs_bitset_init(eecs_bitset_t* bitset, eecs_id_t max_bits) {
	size_t num_bits_per_mask = sizeof(eecs_mask_t) * CHAR_BIT;
	size_t num_masks = ((size_t)max_bits + num_bits_per_mask - 1) / num_bits_per_mask;
	memset(bitset->masks, 0, sizeof(eecs_mask_t) * num_masks);
	bitset->num_masks = (eecs_id_t)num_masks;
}

EECS_PRIVATE void
eecs_bitset_set(eecs_bitset_t* bitset, eecs_id_t bit_index) {
	eecs_id_t num_bits_per_mask = (eecs_id_t)(sizeof(eecs_mask_t) * CHAR_BIT);
	eecs_id_t mask_index = (eecs_id_t)((eecs_mask_t)bit_index / num_bits_per_mask);
	eecs_mask_t bit_mask = 1 << ((eecs_mask_t)bit_index % num_bits_per_mask);
	EECS_ASSERT(mask_index < bitset->num_masks, "Out of bound");

	bitset->masks[mask_index] |= bit_mask;
}

EECS_PRIVATE bool
eecs_bitset_is_set(const eecs_bitset_t* bitset, eecs_id_t bit_index) {
	eecs_id_t num_bits_per_mask = (eecs_id_t)(sizeof(eecs_mask_t) * CHAR_BIT);
	eecs_id_t mask_index = (eecs_id_t)((eecs_mask_t)bit_index / num_bits_per_mask);

	if (mask_index < bitset->num_masks) {
		eecs_mask_t bit_mask = 1 << ((eecs_mask_t)bit_index % num_bits_per_mask);
		return (bitset->masks[mask_index] & bit_mask) > 0;
	} else {
		return false;
	}
}

EECS_PRIVATE bool
eecs_bitset_is_all_set(const eecs_bitset_t* bitset, const eecs_bitset_t* required_bits) {
	bool result = true;
	for (eecs_id_t i = 0; i < required_bits->num_masks; ++i) {
		eecs_mask_t mask = i < bitset->num_masks ? bitset->masks[i] : 0;
		eecs_mask_t required_mask = required_bits->masks[i];
		result &= (mask & required_mask) == required_mask;
	}

	return result;
}

// eecs implementation
// Private

typedef struct eecs_signature_s {
	eecs_id_t length;
	const eecs_component_t* components;
} eecs_signature_t;

typedef struct eecs_component_slot_s {
	eecs_id_t index;
	eecs_component_t component;
} eecs_component_slot_t;

typedef struct eecs_system_entity_callback_s {
	eecs_id_t system_index;
	eecs_system_entity_fn_t fn;
	void* userdata;
} eecs_system_entity_callback_t;

typedef struct eecs_component_entity_callback_s {
	eecs_id_t component_index;
	eecs_id_t signature_index;
	eecs_component_fn_t fn;
	void* userdata;
} eecs_component_entity_callback_t;

typedef struct eecs_table_s {
	eecs_signature_t signature;
	eecs_bitset_t* bitset;

	eecs_id_t num_entities_per_chunk;
	ptrdiff_t* component_storage_offsets;
	size_t* component_sizes;

	eecs_array(eecs_system_entity_callback_t) system_init_callbacks;
	eecs_array(eecs_system_entity_callback_t) system_cleanup_callbacks;
	eecs_array(eecs_component_entity_callback_t) component_init_callbacks;
	eecs_array(eecs_component_entity_callback_t) component_cleanup_callbacks;

	eecs_id_t num_entities;
	eecs_array(char*) chunks;
} eecs_table_t;

typedef struct eecs_system_table_match_s {
	eecs_table_t* table;
	ptrdiff_t* component_storage_offsets;
} eecs_system_table_match_t;

typedef struct eecs_system_data_s {
	void* per_world_data;
	eecs_bitset_t* bitset;
	eecs_array(eecs_system_table_match_t) matched_tables;
} eecs_system_data_t;

typedef struct eecs_morph_entry_s {
	void* init_data;
	eecs_id_t gen;
} eecs_morph_entry_t;

typedef struct eecs_table_chunk_header_s {
	struct eecs_table_chunk_header_s* next;
} eecs_table_chunk_header_t;

typedef struct eecs_arena_chunk_s {
	uintptr_t current;
	uintptr_t end;
	struct eecs_arena_chunk_s* previous;
	char begin[];
} eecs_arena_chunk_t;

typedef struct eecs_arena_s {
	eecs_arena_chunk_t* current_chunk;
} eecs_arena_t;

typedef struct eecs_arena_checkpoint_s {
	eecs_arena_chunk_t* current_chunk;
	uintptr_t bump_ptr;
} eecs_arena_checkpoint_t;

typedef struct eecs_entity_data_s {
	eecs_table_t* table;
	eecs_id_t gen;
	eecs_id_t pos_in_table;
} eecs_entity_data_t;

typedef enum eecs_defferred_op_type_e {
	EECS_DESTROY_ENTITY,
	EECS_MORPH_ENTITY,
} eecs_deferred_op_type_t;

typedef struct eecs_deferred_op_s {
	eecs_deferred_op_type_t type;
	eecs_entity_t handle;

	// For EECS_MORPH_ENTITY
	eecs_component_init_t* new_components;
	eecs_component_t* removed_components;

	struct eecs_deferred_op_s* next;
} eecs_deferred_op_t;

typedef struct eecs_template_data_s {
	eecs_table_t* table;
	eecs_component_init_t* init_data;
} eecs_template_data_t;

struct eecs_s {
	eecs_options_t options;
	eecs_id_t version;
	eecs_array(eecs_component_options_t) components;
	eecs_array(eecs_system_options_t) systems;
};

struct eecs_world_s {
	eecs_t* ecs;
	eecs_world_options_t options;
	eecs_id_t version;

	eecs_mask_t update_mask;
	eecs_table_t* current_update_table;

	eecs_array(eecs_system_data_t) system_data;

	eecs_id_t next_free_entity_slot;
	eecs_array(eecs_entity_data_t) entities;

	eecs_array(eecs_template_data_t) templates;

	// Store pointer so that table's address is stable
	eecs_array(eecs_table_t*) tables;

	eecs_deferred_op_t* first_deferred_ops;
	eecs_deferred_op_t* last_deferred_ops;

	eecs_arena_t version_arena;
	eecs_arena_t deferred_arena;
	eecs_arena_t tmp_arena;

	eecs_table_chunk_header_t* next_free_table_chunks;
};

EECS_PRIVATE uintptr_t
eecs_align_ptr(uintptr_t ptr, size_t alignment) {
	return ((uintptr_t)ptr + (uintptr_t)(alignment - 1)) & -(uintptr_t)alignment;
}

EECS_PRIVATE void*
eecs_allocate_chunk(eecs_world_t* world) {
	if (world->next_free_table_chunks) {
		eecs_table_chunk_header_t* header = world->next_free_table_chunks;
		world->next_free_table_chunks = header->next;
		return header;
	}

	return eecs_malloc(
		world->options.table_chunk_memctx, world->options.table_chunk_size
	);
}

EECS_PRIVATE void
eecs_release_chunk(eecs_world_t* world, void* chunk) {
	eecs_table_chunk_header_t* header = chunk;
	header->next = world->next_free_table_chunks;
	world->next_free_table_chunks = header;
}

EECS_PRIVATE void*
eecs_arena_alloc_from_chunk(eecs_arena_chunk_t* chunk, size_t size, size_t alignment) {
	if (chunk == NULL) { return NULL; }

	uintptr_t ptr = (uintptr_t)chunk->current;
	ptr = eecs_align_ptr(ptr, alignment);
	uintptr_t new_ptr = ptr + size;

	if (new_ptr < chunk->end) {
		chunk->current = new_ptr;
		return (void*)ptr;
	} else {
		return NULL;
	}
}

EECS_PRIVATE void*
eecs_arena_alloc(eecs_world_t* world, eecs_arena_t* arena, size_t size, size_t alignment) {
	size_t chunk_size = world->options.table_chunk_size;
	EECS_ASSERT(size < chunk_size, "Requested memory larger than arena");

	void* mem = eecs_arena_alloc_from_chunk(arena->current_chunk, size, alignment);

	if (mem) { return mem; }

	eecs_arena_chunk_t* new_chunk = eecs_allocate_chunk(world);
	new_chunk->end = (uintptr_t)new_chunk + chunk_size;
	new_chunk->current = (uintptr_t)new_chunk->begin;
	new_chunk->previous = arena->current_chunk;
	arena->current_chunk = new_chunk;

	mem = eecs_arena_alloc_from_chunk(new_chunk, size, alignment);
	EECS_ASSERT(mem != NULL, "Out of memory");

	return mem;
}

EECS_PRIVATE eecs_arena_checkpoint_t
eecs_arena_checkpoint(eecs_world_t* world, eecs_arena_t* arena) {
	(void)world;
	return (eecs_arena_checkpoint_t){
		.current_chunk = arena->current_chunk,
		.bump_ptr = arena->current_chunk != NULL
			? arena->current_chunk->current
			: 0,
	};
}

EECS_PRIVATE void
eecs_arena_rollback(
	eecs_world_t* world,
	eecs_arena_t* arena,
	eecs_arena_checkpoint_t checkpoint
) {
	for (eecs_arena_chunk_t* itr = arena->current_chunk; itr != checkpoint.current_chunk;) {
		eecs_arena_chunk_t* next = itr->previous;
		eecs_release_chunk(world, itr);
		itr = next;
	}

	arena->current_chunk = checkpoint.current_chunk;
	if (checkpoint.current_chunk != NULL) {
		checkpoint.current_chunk->current = checkpoint.bump_ptr;
	}
}

EECS_PRIVATE void
eecs_arena_reset(eecs_world_t* world, eecs_arena_t* arena) {
	eecs_arena_rollback(world, arena, (eecs_arena_checkpoint_t){
		.current_chunk = NULL,
	});
}

EECS_PRIVATE eecs_id_t
eecs_component_list_length(const eecs_component_t* list) {
	eecs_id_t i;
	for (i = 0; list != NULL && list[i].from_1_index != 0; ++i) { }
	return i;
}

EECS_PRIVATE eecs_id_t
eecs_component_init_list_length(const eecs_component_init_t* list) {
	eecs_id_t i;
	for (i = 0; list != NULL && list[i].component.from_1_index != 0; ++i) { }
	return i;
}

EECS_PRIVATE void
eecs_try_match_system_with_table(
	eecs_world_t* world,
	eecs_id_t system_index,
	eecs_table_t* table
) {
	const eecs_system_options_t* system_options = &world->ecs->systems[system_index];
	eecs_system_data_t* system_data = &world->system_data[system_index];

	if (!eecs_bitset_is_all_set(table->bitset, system_data->bitset)) {
		return;
	}

	eecs_signature_t signature = table->signature;
	eecs_id_t num_requirements = eecs_component_list_length(system_options->match_components);

	void* memctx = world->options.memctx;
	if (system_options->init_per_entity_fn) {
		eecs_array_push(memctx, table->system_init_callbacks, ((eecs_system_entity_callback_t) {
			.system_index = system_index,
			.fn = system_options->init_per_entity_fn,
			.userdata = system_options->userdata,
		}));
	}

	if (system_options->cleanup_per_entity_fn) {
		eecs_array_push(memctx, table->system_cleanup_callbacks, ((eecs_system_entity_callback_t) {
			.system_index = system_index,
			.fn = system_options->cleanup_per_entity_fn,
			.userdata = system_options->userdata,
		}));
	}

	if (system_options->update_fn) {
		eecs_array_push(memctx, system_data->matched_tables, (eecs_system_table_match_t){ 0 });
		eecs_system_table_match_t* match = &eecs_array_back(system_data->matched_tables);
		match->table = table;
		if (num_requirements > 0) {
			match->component_storage_offsets = eecs_arena_alloc(
				world,
				&world->version_arena,
				sizeof(ptrdiff_t) * num_requirements,
				_Alignof(ptrdiff_t)
			);
		}

		for (eecs_id_t i = 0; i < num_requirements; ++i) {
			eecs_component_t requirement = system_options->match_components[i];

			for (eecs_id_t j = 0; j < signature.length; ++j) {
				if (signature.components[j].from_1_index == requirement.from_1_index) {
					match->component_storage_offsets[i] = table->component_storage_offsets[j];
					break;
				}
			}
		}
	}
}

EECS_PRIVATE void
eecs_record_component_callbacks(
	eecs_world_t* world,
	eecs_table_t* table
) {
	const eecs_t* ecs = world->ecs;
	void* memctx = world->options.memctx;

	for (eecs_id_t i = 0; i < table->signature.length; ++i) {
		eecs_id_t component_index = eecs_index_of(table->signature.components[i]);
		const eecs_component_options_t* component_options = &ecs->components[component_index];

		if (component_options->init_fn) {
			eecs_array_push(
				memctx,
				table->component_init_callbacks,
				((eecs_component_entity_callback_t){
					.component_index = component_index,
					.signature_index = i,
					.fn = component_options->init_fn,
					.userdata = component_options->userdata,
				})
			);
		}

		if (component_options->cleanup_fn) {
			eecs_array_push(
				memctx,
				table->component_init_callbacks,
				((eecs_component_entity_callback_t){
					.component_index = component_index,
					.signature_index = i,
					.fn = component_options->cleanup_fn,
					.userdata = component_options->userdata,
				})
			);
		}
	}
}

EECS_PRIVATE void
eecs_sync_world(eecs_world_t* world) {
	const eecs_t* ecs = world->ecs;

	if (world->version != ecs->version) {
		world->version = ecs->version;

		void* memctx = world->options.memctx;

		eecs_id_t old_num_systems = eecs_array_length(world->system_data);
		eecs_id_t new_num_systems = eecs_array_length(ecs->systems);
		eecs_array_resize(memctx, world->system_data, new_num_systems);

		eecs_arena_reset(world, &world->version_arena);

		eecs_array_indexed_foreach(eecs_table_t*, itr, world->tables) {
			eecs_table_t* table = *itr.value;
			eecs_array_clear(table->system_init_callbacks);
			eecs_array_clear(table->system_cleanup_callbacks);

			eecs_array_clear(table->component_init_callbacks);
			eecs_array_clear(table->component_cleanup_callbacks);
			eecs_record_component_callbacks(world, table);
		}

		eecs_id_t num_available_components = eecs_array_length(ecs->components);
		for (eecs_id_t i = 0; i < new_num_systems; ++i) {
			const eecs_system_options_t* system_options = &ecs->systems[i];
			eecs_system_data_t* system_data = &world->system_data[i];
			eecs_array_clear(system_data->matched_tables);

			system_data->bitset = eecs_arena_alloc(
				world, &world->version_arena,
				eecs_bitset_memory_size(num_available_components),
				_Alignof(eecs_bitset_t)
			);
			eecs_bitset_init(system_data->bitset, num_available_components);

			for (
				eecs_id_t j = 0;
				system_options->match_components != NULL
				&& system_options->match_components[j].from_1_index != 0;
				++j
			) {
				eecs_bitset_set(
					system_data->bitset,
					eecs_index_of(system_options->match_components[j])
				);
			}

			eecs_array_indexed_foreach(eecs_table_t*, itr, world->tables) {
				eecs_try_match_system_with_table(world, i, *itr.value);
			}
		}

		for (eecs_id_t i = old_num_systems; i < new_num_systems; ++i) {
			const eecs_system_options_t* system_options = &ecs->systems[i];
			if (system_options->init_per_world_fn) {
				system_options->init_per_world_fn(world, system_options->userdata);
			}
		}
	}
}

EECS_PRIVATE eecs_table_t*
eecs_get_table(eecs_world_t* world, eecs_signature_t signature) {
	// TODO: Consider a hash table
	size_t sig_size = sizeof(*signature.components) * signature.length;
	eecs_array_indexed_foreach(eecs_table_t*, itr, world->tables) {
		eecs_signature_t table_signature = (*itr.value)->signature;
		if (
			table_signature.length == signature.length
			&& memcmp(table_signature.components, signature.components, sig_size) == 0
		) {
			return *itr.value;
		}
	}

	eecs_arena_checkpoint_t tmp_checkpoint = eecs_arena_checkpoint(world, &world->tmp_arena);
	void* memctx = world->options.memctx;
	eecs_component_t* sig_content_copy = eecs_malloc(memctx, sig_size);
	memcpy(sig_content_copy, signature.components, sig_size);

	eecs_id_t num_available_components = eecs_array_length(world->ecs->components);
	eecs_table_t* table = eecs_malloc(memctx, sizeof(eecs_table_t));
	*table = (eecs_table_t){
		.signature = {
			.length = signature.length,
			.components = sig_content_copy,
		},
		.bitset = eecs_malloc(
			memctx, eecs_bitset_memory_size(num_available_components)
		),
		.component_storage_offsets = eecs_malloc(
			memctx, sizeof(ptrdiff_t) * signature.length
		),
		.component_sizes = eecs_malloc(
			memctx, sizeof(size_t) * signature.length
		),
	};
	eecs_bitset_init(table->bitset, num_available_components);
	for (eecs_id_t i = 0; i < signature.length; ++i) {
		eecs_bitset_set(table->bitset, eecs_index_of(signature.components[i]));
	}
	eecs_array_push(memctx, world->tables, table);  // NOLINT(bugprone-sizeof-expression)

	// Calculate how many entities can fit in a chunk and storage offset
	// Sort by alignment to avoid wastage
	eecs_component_slot_t* component_slots = eecs_arena_alloc(
		world,
		&world->tmp_arena,
		sizeof(eecs_component_slot_t) * signature.length,
		_Alignof(eecs_component_slot_t)
	);
	for (eecs_id_t i = 0; i < signature.length; ++i) {
		component_slots[i].index = i;
		component_slots[i].component = signature.components[i];
	}

	const eecs_t* ecs = world->ecs;
	const eecs_component_options_t* components = ecs->components;
#define eecs_alignment_cmp_lt(lhs, rhs) \
	(components[eecs_index_of(lhs.component)].alignment < components[eecs_index_of(rhs.component)].alignment)
	eecs_insertion_sort(
		signature.length, component_slots, eecs_component_slot_t, eecs_alignment_cmp_lt
	);

	// The entity id is in the first position
	uintptr_t data_size = (uintptr_t)sizeof(eecs_id_t);
	uintptr_t struct_size = (uintptr_t)sizeof(eecs_id_t);
	uintptr_t max_align = (uintptr_t)_Alignof(eecs_id_t);

	for (eecs_id_t i = 0; i < signature.length; ++i) {
		const eecs_component_slot_t* slot = &component_slots[i];
		const eecs_component_options_t* component_options = &components[eecs_index_of(slot->component)];
		struct_size = eecs_align_ptr(struct_size, component_options->alignment);
		max_align = eecs_max(max_align, component_options->alignment);
		struct_size += component_options->size;
		data_size += component_options->size;
	}
	struct_size = eecs_align_ptr(struct_size, max_align);
	uintptr_t alignment_overhead = struct_size - data_size;
	uintptr_t num_entities_per_chunk = (world->options.table_chunk_size - alignment_overhead) / data_size;
	table->num_entities_per_chunk = (eecs_id_t)num_entities_per_chunk;

	// Layout each components
	uintptr_t data_offset = (uintptr_t)(sizeof(eecs_id_t) * num_entities_per_chunk);
	for (eecs_id_t i = 0; i < signature.length; ++i) {
		const eecs_component_slot_t* slot = &component_slots[i];
		const eecs_component_options_t* component_options = &components[eecs_index_of(slot->component)];
		data_offset = eecs_align_ptr(data_offset, component_options->alignment);
		table->component_storage_offsets[slot->index] = data_offset;
		table->component_sizes[slot->index] = component_options->size;
		data_offset += component_options->size * num_entities_per_chunk;
	}
	EECS_ASSERT(data_offset <= world->options.table_chunk_size, "Layout failed");

	eecs_record_component_callbacks(world, table);

	// Find matching systems
	eecs_array_indexed_foreach(eecs_system_data_t, itr, world->system_data) {
		eecs_try_match_system_with_table(world, itr.index, table);
	}

	eecs_arena_rollback(world, &world->tmp_arena, tmp_checkpoint);
	return table;
}

EECS_PRIVATE eecs_entity_data_t*
eecs_get_entity_data(eecs_world_t* world, eecs_entity_t handle) {
	eecs_id_t from_1_index = handle.from_1_index;
	if (!(1 <= from_1_index && from_1_index <= eecs_array_length(world->entities))) {
		return NULL;
	}

	eecs_entity_data_t* entity_data = &world->entities[from_1_index - 1];
	return entity_data->gen == handle.gen ? entity_data : NULL;
}

EECS_PRIVATE void
eecs_delete_entity_from_table(
	eecs_world_t* world,
	eecs_table_t* table,
	eecs_id_t pos_in_table
) {
	// Move the last entity into the destroyed slot
	eecs_id_t chunk_index = pos_in_table / table->num_entities_per_chunk;
	eecs_id_t pos_in_chunk = pos_in_table % table->num_entities_per_chunk;
	char* chunk = table->chunks[chunk_index];

	eecs_id_t last_slot = --table->num_entities;
	eecs_id_t last_chunk_index = last_slot / table->num_entities_per_chunk;
	eecs_id_t last_pos_in_chunk = last_slot % table->num_entities_per_chunk;
	char* last_chunk = table->chunks[last_chunk_index];

	eecs_id_t last_entity_from_1_index;
	((eecs_id_t*)chunk)[pos_in_chunk] = last_entity_from_1_index = ((eecs_id_t*)last_chunk)[last_pos_in_chunk];

	const ptrdiff_t* component_storage_offsets = table->component_storage_offsets;
	const size_t* component_sizes = table->component_sizes;
	for (eecs_id_t i = 0; i < table->signature.length; ++i) {
		size_t component_size = component_sizes[i];
		ptrdiff_t component_storage_offset = component_storage_offsets[i];
		char* component_data = chunk
			+ component_storage_offset
			+ pos_in_chunk * component_size;
		char* last_component_data = last_chunk
			+ component_storage_offset
			+ last_pos_in_chunk * component_size;

		memcpy(component_data, last_component_data, component_size);
	}
	world->entities[last_entity_from_1_index - 1].pos_in_table = pos_in_table;

	// If last chunk is empty, release it
	if (last_pos_in_chunk == 0) {
		eecs_release_chunk(world, eecs_array_pop(table->chunks));
	}
}

EECS_PRIVATE void
eecs_destroy_entity_now(eecs_world_t* world, eecs_entity_data_t* entity_data) {
	eecs_id_t from_1_index = entity_data - world->entities + 1;
	eecs_table_t* table = entity_data->table;
	eecs_entity_t handle = {
		.from_1_index = from_1_index,
		.gen = entity_data->gen,
	};
	eecs_id_t pos_in_table = entity_data->pos_in_table;
	eecs_id_t chunk_index = pos_in_table / table->num_entities_per_chunk;
	eecs_id_t pos_in_chunk = pos_in_table % table->num_entities_per_chunk;

	// Cleanup entity by systems
	eecs_array_indexed_foreach_rev(
		eecs_system_entity_callback_t, itr, table->system_cleanup_callbacks
	) {
		itr.value->fn(world, handle, itr.value->userdata);
	}

	// Cleanup components
	const ptrdiff_t* component_storage_offsets = table->component_storage_offsets;
	const size_t* component_sizes = table->component_sizes;
	char* chunk = table->chunks[chunk_index];
	eecs_array_indexed_foreach_rev(
		eecs_component_entity_callback_t, itr, table->component_cleanup_callbacks
	) {
		char* component_data = chunk
			+ component_storage_offsets[itr.value->signature_index]
			+ pos_in_chunk * component_sizes[itr.value->signature_index];

		itr.value->fn(world, handle, component_data, itr.value->userdata);
	}

	eecs_delete_entity_from_table(world, table, pos_in_table);

	// Recycle data slot
	entity_data = &world->entities[from_1_index - 1];
	++entity_data->gen;
	entity_data->pos_in_table = world->next_free_entity_slot;
	world->next_free_entity_slot = from_1_index;
}

EECS_PRIVATE eecs_deferred_op_t*
eecs_alloc_deferred_op(eecs_world_t* world, eecs_deferred_op_type_t type, eecs_entity_t handle) {
	eecs_deferred_op_t* op = eecs_arena_alloc(
		world,
		&world->deferred_arena,
		sizeof(eecs_deferred_op_t),
		_Alignof(eecs_deferred_op_t)
	);
	*op = (eecs_deferred_op_t){
		.type = type,
		.handle = handle,
	};

	if (world->first_deferred_ops == NULL) {
		world->first_deferred_ops = op;
	}

	if (world->last_deferred_ops != NULL) {
		world->last_deferred_ops->next = op;
	}
	world->last_deferred_ops = op;

	return op;
}

EECS_PRIVATE void
eecs_parse_component_init(
	eecs_world_t* world,
	const eecs_component_init_t* init,
	eecs_component_init_t** init_copy_out,
	eecs_table_t** table_out
) {
	// This assumes that the caller took a checkpoint
	eecs_id_t num_available_components = eecs_array_length(world->ecs->components);
	eecs_bitset_t* bitset = eecs_arena_alloc(
		world,
		&world->tmp_arena,
		eecs_bitset_memory_size(num_available_components),
		_Alignof(eecs_bitset_t)
	);
	eecs_bitset_init(bitset, num_available_components);

	eecs_id_t num_inits = eecs_component_init_list_length(init);
	eecs_component_init_t* init_copy = eecs_arena_alloc(
		world,
		&world->tmp_arena,
		sizeof(eecs_component_init_t) * num_inits,
		_Alignof(eecs_component_init_t)
	);

	// Dedupe
	eecs_id_t num_new_components = 0;
	for (eecs_id_t i = 0; i < num_inits; ++i) {
		eecs_id_t bit_index = eecs_index_of(init[i].component);
		if (eecs_bitset_is_set(bitset, bit_index)) { continue; }

		eecs_bitset_set(bitset, bit_index);
		init_copy[num_new_components++] = init[i];
	}

	eecs_insertion_sort(
		num_new_components, init_copy,
		eecs_component_init_t, eecs_component_init_cmp_lt
	);

	eecs_component_t* components = eecs_arena_alloc(
		world,
		&world->tmp_arena,
		sizeof(eecs_component_t) * num_new_components,
		_Alignof(eecs_component_t)
	);
	for (eecs_id_t i = 0; i < num_new_components; ++i) {
		components[i] = init_copy[i].component;
	}

	eecs_signature_t signature = {
		.length = num_new_components,
		.components = components,
	};
	*table_out = eecs_get_table(world, signature);
	*init_copy_out = init_copy;
}

EECS_PRIVATE void
eecs_insert_entity_into_table(
	eecs_world_t* world,
	eecs_table_t* table,
	eecs_id_t entity_from_1_index,
	const eecs_component_init_t* init,
	eecs_id_t* pos_in_table_out,
	char** chunk_out,
	eecs_id_t* pos_in_chunk_out
) {
	eecs_id_t pos_in_table = table->num_entities++;
	eecs_id_t chunk_index = pos_in_table / table->num_entities_per_chunk;
	eecs_id_t pos_in_chunk = pos_in_table % table->num_entities_per_chunk;

	char* chunk;
	if (chunk_index >= eecs_array_length(table->chunks)) {
		chunk = eecs_allocate_chunk(world);
		eecs_array_push(world->options.memctx, table->chunks, chunk);
	} else {
		chunk = table->chunks[chunk_index];
	}

	// Write entity data into chunk
	eecs_id_t* entity_ids = (eecs_id_t*)chunk;
	entity_ids[pos_in_chunk] = entity_from_1_index;
	const ptrdiff_t* component_storage_offsets = table->component_storage_offsets;
	const size_t* component_sizes = table->component_sizes;
	for (eecs_id_t i = 0; i < table->signature.length; ++i) {
		size_t component_size = component_sizes[i];

		char* component_data = chunk
			+ component_storage_offsets[i]
			+ pos_in_chunk * component_size;

		const void* init_data = init[i].data;
		if (init_data == NULL) {
			memset(component_data, 0, component_size);
		} else {
			memcpy(component_data, init_data, component_size);
		}
	}

	*pos_in_table_out = pos_in_table;
	*chunk_out = chunk;
	*pos_in_chunk_out = pos_in_chunk;
}

EECS_PRIVATE eecs_entity_t
eecs_create_entity_for_table(
	eecs_world_t* world,
	eecs_table_t* table,
	const eecs_component_init_t* init
) {
	void* memctx = world->options.memctx;

	eecs_entity_t entity_handle;
	eecs_entity_data_t* entity_data;
	if (world->next_free_entity_slot == 0) {
		eecs_array_push(memctx, world->entities, (eecs_entity_data_t){ 0 });
		eecs_id_t from_1_index = eecs_array_length(world->entities);
		entity_data = &world->entities[from_1_index - 1];
		entity_handle.from_1_index = from_1_index;
		entity_handle.gen = 0;
	} else {
		eecs_id_t from_1_index = world->next_free_entity_slot;
		entity_data = &world->entities[from_1_index - 1];
		world->next_free_entity_slot = entity_data->pos_in_table;
		entity_handle.from_1_index = from_1_index;
		entity_handle.gen = entity_data->gen;
	}

	entity_data->table = table;
	char* chunk;
	eecs_id_t pos_in_chunk;
	eecs_insert_entity_into_table(
		world, table, entity_handle.from_1_index, init,
		&entity_data->pos_in_table,
		&chunk, &pos_in_chunk
	);

	// Init components
	eecs_array_indexed_foreach(eecs_component_entity_callback_t, itr, table->component_init_callbacks) {
		char* component_data = chunk
			+ table->component_storage_offsets[itr.value->signature_index]
			+ pos_in_chunk * table->component_sizes[itr.value->signature_index];

		itr.value->fn(world, entity_handle, component_data, itr.value->userdata);
	}

	// Init entity by systems
	eecs_array_indexed_foreach(eecs_system_entity_callback_t, itr, table->system_init_callbacks) {
		itr.value->fn(world, entity_handle, itr.value->userdata);
	}

	return entity_handle;
}

EECS_PRIVATE void
eecs_morph_entity_now(
	eecs_world_t* world,
	eecs_entity_data_t* entity_data,
	const eecs_component_init_t* new_components,
	const eecs_component_t* removed_components
) {
	eecs_id_t from_1_index = entity_data - world->entities + 1;
	eecs_arena_checkpoint_t tmp_checkpoint = eecs_arena_checkpoint(world, &world->tmp_arena);
	eecs_table_t* table = entity_data->table;
	eecs_entity_t handle = {
		.from_1_index = from_1_index,
		.gen = entity_data->gen,
	};

	eecs_id_t num_new_components = eecs_component_init_list_length(new_components);
	eecs_id_t num_removed_components = eecs_component_list_length(removed_components);
	eecs_id_t num_available_components = eecs_array_length(world->ecs->components);

	eecs_bitset_t* remove_bitset = eecs_arena_alloc(
		world, &world->tmp_arena,
		eecs_bitset_memory_size(num_available_components),
		_Alignof(eecs_bitset_t)
	);
	eecs_bitset_init(remove_bitset, num_available_components);
	for (eecs_id_t i = 0; i < num_removed_components; ++i) {
		eecs_bitset_set(remove_bitset, eecs_index_of(removed_components[i]));
	}

	eecs_bitset_t* add_bitset = eecs_arena_alloc(
		world, &world->tmp_arena,
		eecs_bitset_memory_size(num_available_components),
		_Alignof(eecs_bitset_t)
	);
	eecs_bitset_init(add_bitset, num_available_components);
	eecs_component_init_t* init_data = eecs_arena_alloc(
		world, &world->tmp_arena,
		sizeof(eecs_component_init_t) * (num_new_components + entity_data->table->signature.length),
		_Alignof(eecs_component_init_t)
	);
	eecs_id_t new_sig_length = 0;

	eecs_id_t pos_in_table = entity_data->pos_in_table;
	eecs_id_t chunk_index = pos_in_table / table->num_entities_per_chunk;
	eecs_id_t pos_in_chunk = pos_in_table % table->num_entities_per_chunk;
	char* chunk = table->chunks[chunk_index];
	for (eecs_id_t i = 0; i < table->signature.length; ++i) {
		eecs_id_t component_index = eecs_index_of(table->signature.components[i]);
		if (eecs_bitset_is_set(remove_bitset, component_index)) { continue; }

		char* component_data = chunk
			+ table->component_storage_offsets[i]
			+ pos_in_chunk * table->component_sizes[i];

		eecs_bitset_set(add_bitset, component_index);
		init_data[new_sig_length++] = (eecs_component_init_t){
			.component = table->signature.components[i],
			.data = component_data,
		};
	}

	for (eecs_id_t i = 0; i < num_new_components; ++i) {
		eecs_id_t component_index = eecs_index_of(new_components[i].component);
		if (eecs_bitset_is_set(remove_bitset, component_index)) { continue; }
		if (eecs_bitset_is_set(add_bitset, component_index)) { continue; }

		eecs_bitset_set(add_bitset, component_index);
		init_data[new_sig_length++] = new_components[i];
	}

	eecs_insertion_sort(
		new_sig_length, init_data,
		eecs_component_init_t, eecs_component_init_cmp_lt
	);
	eecs_component_t* components = eecs_arena_alloc(
		world, &world->tmp_arena,
		sizeof(eecs_component_t) * new_sig_length,
		_Alignof(eecs_component_t)
	);
	for (eecs_id_t i = 0; i < new_sig_length; ++i) {
		components[i] = init_data[i].component;
	}
	eecs_signature_t new_signature = {
		.components = components,
		.length = new_sig_length,
	};

	eecs_table_t* new_table = eecs_get_table(world, new_signature);

	// Call clean up for systems present in the old table but not the new table
	eecs_array_indexed_foreach_rev(
		eecs_system_entity_callback_t, itr,
		table->system_cleanup_callbacks
	) {
		eecs_id_t system_index = itr.value->system_index;
		const eecs_system_data_t* system_data = &world->system_data[system_index];
		if (!eecs_bitset_is_all_set(new_table->bitset, system_data->bitset)) {
			itr.value->fn(world, handle, itr.value->userdata);
		}
	}

	// Call clean up for components present in the old table but not the new table
	eecs_array_indexed_foreach_rev(
		eecs_component_entity_callback_t, itr,
		table->component_cleanup_callbacks
	) {
		if (!eecs_bitset_is_set(new_table->bitset, itr.value->component_index)) {
			char* component_data = chunk
				+ table->component_storage_offsets[itr.value->signature_index]
				+ pos_in_chunk * table->component_sizes[itr.value->signature_index];
			itr.value->fn(world, handle, component_data, itr.value->userdata);
		}
	}

	// Copy data to new table
	entity_data = &world->entities[from_1_index - 1];
	char* new_chunk;
	eecs_id_t new_pos_in_table, new_pos_in_chunk;
	eecs_insert_entity_into_table(
		world, new_table, from_1_index, init_data,
		&new_pos_in_table, &new_chunk, &new_pos_in_chunk
	);
	entity_data->table = new_table;
	entity_data->pos_in_table = new_pos_in_table;

	// Delete the old entity slot in the old chunk
	eecs_delete_entity_from_table(world, table, pos_in_table);

	// Call init for components present in the new table but not the old table
	eecs_array_indexed_foreach(
		eecs_component_entity_callback_t, itr,
		new_table->component_init_callbacks
	) {
		if (!eecs_bitset_is_set(table->bitset, itr.value->component_index)) {
			char* component_data = new_chunk
				+ new_table->component_storage_offsets[itr.value->signature_index]
				+ new_pos_in_chunk * new_table->component_sizes[itr.value->signature_index];
			itr.value->fn(world, handle, component_data, itr.value->userdata);
		}
	}

	// Call init for systems present in the new table but not the old table
	eecs_array_indexed_foreach(
		eecs_system_entity_callback_t, itr,
		new_table->system_init_callbacks
	) {
		eecs_id_t system_index = itr.value->system_index;
		const eecs_system_data_t* system_data = &world->system_data[system_index];
		if (!eecs_bitset_is_all_set(table->bitset, system_data->bitset)) {
			itr.value->fn(world, handle, itr.value->userdata);
		}
	}

	eecs_arena_rollback(world, &world->tmp_arena, tmp_checkpoint);
}

EECS_PRIVATE void
eecs_do_run_system(
	eecs_world_t* world,
	const eecs_system_options_t* system_options,
	eecs_system_data_t* system_data
) {
	if (system_options->pre_update_fn) {
		system_options->pre_update_fn(world, system_options->userdata);
	}

	eecs_array_indexed_foreach(
		eecs_system_table_match_t,
		match_itr,
		system_data->matched_tables
	) {
		eecs_table_t* table = match_itr.value->table;

		eecs_arena_reset(world, &world->deferred_arena);
		world->first_deferred_ops = world->last_deferred_ops = NULL;
		world->current_update_table = table;

		ptrdiff_t* component_storage_offsets = match_itr.value->component_storage_offsets;
		eecs_id_t num_entities_per_chunk = table->num_entities_per_chunk;

		eecs_array_indexed_foreach(char*, chunk_itr, table->chunks) {
			eecs_id_t last_chunk_index = eecs_array_length(table->chunks) - 1;
			eecs_id_t num_entities_in_last_chunk = table->num_entities % num_entities_per_chunk;
			eecs_batch_t batch = {
				.world = world,
				.chunk = *chunk_itr.value,
				.offsets = component_storage_offsets,
				.size = chunk_itr.index == last_chunk_index
					? num_entities_in_last_chunk
					: num_entities_per_chunk,
			};

			system_options->update_fn(world, batch, system_options->userdata);
		}

		for (
			eecs_deferred_op_t* op = world->first_deferred_ops;
			op != NULL;
			op = op->next
		) {
			eecs_entity_data_t* entity_data = eecs_get_entity_data(world, op->handle);
			if (entity_data == NULL) { continue; }

			switch (op->type) {
				case EECS_DESTROY_ENTITY:
					eecs_destroy_entity_now(world, entity_data);
					break;
				case EECS_MORPH_ENTITY:
					eecs_morph_entity_now(
						world, entity_data,
						op->new_components, op->removed_components
					);
					break;
			}
		}
	}
	world->current_update_table = NULL;

	if (system_options->post_update_fn) {
		system_options->post_update_fn(world, system_options->userdata);
	}
}

// Public

eecs_t*
eecs_create(eecs_options_t options) {
	eecs_t* ecs = eecs_malloc(options.memctx, sizeof(eecs_t));
	*ecs = (eecs_t){
		.options = options,
	};

	return ecs;
}

void
eecs_destroy(eecs_t* ecs) {
	void* memctx = ecs->options.memctx;

	eecs_array_free(memctx, ecs->systems);
	eecs_array_free(memctx, ecs->components);
	eecs_free(memctx, ecs);
}

void
eecs_register_component(
	eecs_t* ecs,
	eecs_component_t* handle,
	eecs_component_options_t options
) {
	void* memctx = ecs->options.memctx;
	EECS_ASSERT(options.alignment > 0, "Invalid alignment");
	if (handle->from_1_index == 0) {
		eecs_array_push(memctx, ecs->components, options);
		handle->from_1_index = eecs_array_length(ecs->components);
	} else {
		ecs->components[eecs_index_of(*handle)] = options;
	}

	++ecs->version;
}

void
eecs_register_system(
	eecs_t* ecs,
	eecs_system_t* handle,
	eecs_system_options_t options
) {
	void* memctx = ecs->options.memctx;
	if (handle->from_1_index == 0) {
		eecs_array_push(memctx, ecs->systems, options);
		handle->from_1_index = eecs_array_length(ecs->components);
	} else {
		ecs->systems[eecs_index_of(*handle)] = options;
	}

	++ecs->version;
}

eecs_world_t*
eecs_create_world(eecs_t* ecs, eecs_world_options_t options) {
	options.memctx = options.memctx != NULL ? options.memctx : ecs->options.memctx;
	options.table_chunk_size = options.table_chunk_size > 0
		? options.table_chunk_size
		: EECS_DEFAULT_TABLE_CHUNK_SIZE;

	eecs_world_t* world = eecs_malloc(options.memctx, sizeof(eecs_world_t));

	*world = (eecs_world_t){
		.ecs = ecs,
		.options = options,
	};

	eecs_sync_world(world);

	return world;
}

void
eecs_destroy_world(eecs_world_t* world) {
	void* memctx = world->options.memctx;
	const eecs_t* ecs = world->ecs;

	eecs_array_indexed_foreach_rev(eecs_system_data_t, itr, world->system_data) {
		eecs_system_options_t* system = &ecs->systems[itr.index];
		if (system->cleanup_per_world_fn) {
			system->cleanup_per_world_fn(world, system->userdata);
		}

		eecs_array_free(memctx, itr.value->matched_tables);
	}
	eecs_array_free(memctx, world->system_data);

	eecs_array_free(memctx, world->entities);

	eecs_array_indexed_foreach(eecs_template_data_t, itr, world->templates) {
		eecs_id_t signature_length = itr.value->table->signature.length;
		for (eecs_id_t i = 0; i < signature_length; ++i) {
			void* component_data = (void*)itr.value->init_data[i].data;
			eecs_free(memctx, component_data);
		}
		eecs_free(memctx, itr.value->init_data);
	}
	eecs_array_free(memctx, world->templates);

	eecs_array_indexed_foreach(eecs_table_t*, itr, world->tables) {
		eecs_table_t* table = *itr.value;

		eecs_free(memctx, (void*)table->signature.components);
		eecs_free(memctx, table->component_storage_offsets);
		eecs_free(memctx, table->component_sizes);
		eecs_array_free(memctx, table->system_init_callbacks);
		eecs_array_free(memctx, table->system_cleanup_callbacks);
		eecs_array_free(memctx, table->component_init_callbacks);
		eecs_array_free(memctx, table->component_cleanup_callbacks);

		eecs_array_indexed_foreach(char*, chunk_itr, table->chunks) {
			eecs_free(world->options.table_chunk_memctx, *chunk_itr.value);
		}
		eecs_array_free(memctx, table->chunks);
		eecs_free(memctx, table->bitset);
		eecs_free(memctx, table);
	}
	eecs_array_free(memctx, world->tables);

	eecs_arena_reset(world, &world->version_arena);
	eecs_arena_reset(world, &world->deferred_arena);
	eecs_arena_reset(world, &world->tmp_arena);

	for (
		eecs_table_chunk_header_t* itr = world->next_free_table_chunks;
		itr != NULL;
	) {
		eecs_table_chunk_header_t* next = itr->next;
		eecs_free(world->options.table_chunk_memctx, itr);
		itr = next;
	}

	eecs_free(memctx, world);
}

void
eecs_set_per_world_userdata(
	eecs_world_t* world,
	eecs_system_t system,
	void* userdata
) {
	// This can only be called by a system callback so it must already be synced
	EECS_ASSERT(world->version == world->ecs->version, "World not synced");

	world->system_data[eecs_index_of(system)].per_world_data = userdata;
}

void*
eecs_get_per_world_userdata(
	eecs_world_t* world,
	eecs_system_t system
) {
	// This can only be called by a system callback so it must already be synced
	EECS_ASSERT(world->version == world->ecs->version, "World not synced");

	return world->system_data[eecs_index_of(system)].per_world_data;
}

eecs_entity_t
eecs_create_entity(eecs_world_t* world, const eecs_component_init_t* init) {
	eecs_sync_world(world);

	eecs_arena_checkpoint_t tmp_checkpoint = eecs_arena_checkpoint(world, &world->tmp_arena);

	eecs_component_init_t* init_copy;
	eecs_table_t* table;
	eecs_parse_component_init(world, init, &init_copy, &table);

	eecs_entity_t entity = eecs_create_entity_for_table(world, table, init_copy);

	eecs_arena_rollback(world, &world->tmp_arena, tmp_checkpoint);
	return entity;
}

void
eecs_destroy_entity(eecs_world_t* world, eecs_entity_t handle) {
	eecs_sync_world(world);

	eecs_entity_data_t* entity_data = eecs_get_entity_data(world, handle);
	if (entity_data == NULL) { return; }

	if (entity_data->table == world->current_update_table) {
		eecs_alloc_deferred_op(world, EECS_DESTROY_ENTITY, handle);
	} else {
		eecs_destroy_entity_now(world, entity_data);
	}
}

void
eecs_register_template(
	eecs_world_t* world,
	eecs_template_t* handle,
	const eecs_component_init_t* init
) {
	eecs_sync_world(world);

	eecs_arena_checkpoint_t tmp_checkpoint = eecs_arena_checkpoint(world, &world->tmp_arena);
	void* memctx = world->options.memctx;

	eecs_template_data_t* entity_template;
	if (handle->from_1_index == 0) {
		eecs_array_push(memctx, world->templates, (eecs_template_data_t){ 0 });
		entity_template = &eecs_array_back(world->templates);
	} else {
		entity_template = &world->templates[eecs_index_of(*handle)];

		eecs_id_t old_signature_length = entity_template->table->signature.length;
		for (eecs_id_t i = 0; i < old_signature_length; ++i) {
			eecs_free(memctx, (void*)entity_template->init_data[i].data);
		}
	}

	eecs_component_init_t* init_copy;
	eecs_table_t* table;
	eecs_parse_component_init(world, init, &init_copy, &table);

	entity_template->table = table;
	entity_template->init_data = eecs_realloc(
		memctx,
		entity_template->init_data,
		sizeof(eecs_component_init_t) * table->signature.length
	);

	const eecs_t* ecs = world->ecs;
	for (eecs_id_t i = 0; i < table->signature.length; ++i) {
		const eecs_component_options_t* component_options = &ecs->components[eecs_index_of(init_copy[i].component)];
		eecs_component_init_t* init_data = &entity_template->init_data[i];

		init_data->component = init_copy[i].component;

		if (init_copy[i].data != NULL) {
			void* data_copy = eecs_malloc(memctx, component_options->size);
			memcpy(data_copy, init_copy->data, component_options->size);

			init_data->data = data_copy;
		} else {
			init_data->data = NULL;
		}
	}

	eecs_arena_rollback(world, &world->tmp_arena, tmp_checkpoint);
}

eecs_entity_t
eecs_create_entity_from_template(
	eecs_world_t* world,
	eecs_template_t entity_template,
	const eecs_component_init_t* overrides
) {
	eecs_sync_world(world);

	eecs_arena_checkpoint_t tmp_checkpoint = eecs_arena_checkpoint(world, &world->tmp_arena);

	EECS_ASSERT(entity_template.from_1_index > 0, "Invalid template");
	const eecs_template_data_t* template_data = &world->templates[eecs_index_of(entity_template)];
	eecs_table_t* table = template_data->table;

	eecs_component_init_t* init_data;

	if (overrides != NULL) {
		init_data = eecs_arena_alloc(
			world, &world->tmp_arena,
			sizeof(eecs_component_init_t) * table->signature.length,
			_Alignof(eecs_component_init_t)
		);
		memcpy(
			init_data,
			template_data->init_data,
			sizeof(eecs_component_init_t) * table->signature.length
		);

		for (eecs_id_t i = 0; overrides[i].component.from_1_index != 0; ++i) {
			for (eecs_id_t j = 0; j < table->signature.length; ++j) {
				if (table->signature.components[j].from_1_index == overrides[i].component.from_1_index) {
					init_data[j].data = overrides[i].data;
					break;
				}
			}
		}
	} else {
		init_data = template_data->init_data;
	}

	eecs_entity_t entity = eecs_create_entity_for_table(world, table, init_data);

	eecs_arena_rollback(world, &world->tmp_arena, tmp_checkpoint);
	return entity;
}

void
eecs_run_systems(eecs_world_t* world, eecs_mask_t update_mask) {
	EECS_ASSERT(world->current_update_table == NULL, "eecs_run_systems is not reentrant");

	eecs_sync_world(world);

	world->update_mask = update_mask;

	const eecs_t* ecs = world->ecs;
	eecs_array_indexed_foreach(
		eecs_system_data_t,
		sys_itr,
		world->system_data
	) {
		const eecs_system_options_t* system_options = &ecs->systems[sys_itr.index];
		if ((update_mask & system_options->update_mask) == system_options->update_mask) {
			eecs_do_run_system(world, system_options, sys_itr.value);
		}
	}

	world->update_mask = EECS_UPDATE_NONE;
}

void
eecs_run_system(eecs_world_t* world, eecs_mask_t update_mask, eecs_system_t system) {
	EECS_ASSERT(world->current_update_table == NULL, "eecs_run_system is not reentrant");

	eecs_sync_world(world);

	world->update_mask = update_mask;

	const eecs_system_options_t* system_options = &world->ecs->systems[eecs_index_of(system)];
	eecs_system_data_t* system_data = &world->system_data[eecs_index_of(system)];
	eecs_do_run_system(world, system_options, system_data);

	world->update_mask = EECS_UPDATE_NONE;
}

eecs_mask_t
eecs_get_current_update_mask(eecs_world_t* world) {
	return world->update_mask;
}

eecs_id_t
eecs_get_batch_size(eecs_batch_t batch) {
	return batch.size;
}

void*
eecs_get_components_in_batch(eecs_batch_t batch, eecs_id_t match_index) {
	return (char*)batch.chunk + batch.offsets[match_index];
}

eecs_entity_t
eecs_get_entity_in_batch(eecs_batch_t batch, eecs_id_t index) {
	EECS_ASSERT(index < batch.size, "Out of bound access");

	const eecs_id_t* entity_ids = batch.chunk;
	eecs_id_t from_1_index = entity_ids[index];
	const eecs_entity_data_t* entity_data = &batch.world->entities[from_1_index - 1];

	return (eecs_entity_t){
		.from_1_index = from_1_index,
		.gen = entity_data->gen,
	};
}

bool
eecs_is_valid_entity(eecs_world_t* world, eecs_entity_t handle) {
	return eecs_get_entity_data(world, handle) != NULL;
}

void*
eecs_get_component_in_entity(
	eecs_world_t* world,
	eecs_entity_t entity,
	eecs_component_t component_type
) {
	const eecs_entity_data_t* entity_data = eecs_get_entity_data(world, entity);
	if (entity_data == NULL) { return NULL; }

	const eecs_table_t* table = entity_data->table;
	eecs_id_t pos_in_table = entity_data->pos_in_table;
	eecs_id_t chunk_index = pos_in_table / table->num_entities_per_chunk;
	eecs_id_t pos_in_chunk = pos_in_table % table->num_entities_per_chunk;
	char* chunk = table->chunks[chunk_index];

	for (eecs_id_t i = 0; i < table->signature.length; ++i) {
		if (table->signature.components->from_1_index == component_type.from_1_index) {
			return chunk
				+ table->component_storage_offsets[i]
				+ pos_in_chunk * table->component_sizes[i];
		}
	}

	return NULL;
}

void
eecs_morph_entity(
	eecs_world_t* world,
	eecs_entity_t handle,
	const eecs_component_init_t* new_components,
	const eecs_component_t* removed_components
) {
	eecs_sync_world(world);

	eecs_entity_data_t* entity_data = eecs_get_entity_data(world, handle);
	if (entity_data == NULL) { return; }

	if (entity_data->table == world->current_update_table) {
		eecs_deferred_op_t* op = eecs_alloc_deferred_op(world, EECS_MORPH_ENTITY, handle);

		eecs_id_t num_new_components = eecs_component_init_list_length(new_components);
		if (new_components != NULL && num_new_components > 0) {
			eecs_component_init_t* new_components_copy = eecs_arena_alloc(
				world, &world->deferred_arena,
				sizeof(eecs_component_init_t) * (num_new_components + 1),
				_Alignof(eecs_component_init_t)
			);
			for (eecs_id_t i = 0; i < num_new_components; ++i) {
				new_components_copy[i].component = new_components[i].component;
				const void* init_data = new_components[i].data;
				if (init_data != NULL) {
					const eecs_component_options_t* component_options = &world->ecs->components[eecs_index_of(new_components[i].component)];
					void* data_copy = eecs_arena_alloc(
						world, &world->deferred_arena,
						component_options->size, component_options->alignment
					);
					memcpy(data_copy, init_data, component_options->size);

					new_components_copy[i].data = data_copy;
				} else {
					new_components_copy[i].data = NULL;
				}
			}
			new_components_copy[num_new_components] = (eecs_component_init_t){ 0 };
			op->new_components = new_components_copy;
		}

		eecs_id_t num_removed_components = eecs_component_list_length(removed_components);
		if (removed_components != NULL && num_removed_components > 0) {
			eecs_component_t* removed_components_copy = eecs_arena_alloc(
				world, &world->deferred_arena,
				sizeof(eecs_component_t) * (num_removed_components + 1),
				_Alignof(eecs_component_t)
			);
			memcpy(
				removed_components_copy,
				removed_components,
				sizeof(eecs_component_t) * (num_removed_components + 1)
			);
			op->removed_components = removed_components_copy;
		}
	} else {
		eecs_morph_entity_now(
			world, entity_data, new_components, removed_components
		);
	}
}

#endif

#include <munit/munit.h>
#include <eecs.h>
#include "components.h"

struct SystemData {
	int init_per_world_called;
	int cleanup_per_world_called;
	int init_per_entity;
	int cleanup_per_entity;
	eecs_system_t handle;

	eecs_entity_t seen_entities[10];
};

static void
system_init_per_world(
	eecs_world_t* world,
	void* userdata
) {
	struct SystemData* system_data = userdata;
	++system_data->init_per_world_called;

	eecs_set_per_world_userdata(world, system_data->handle, userdata);
}

static void
system_cleanup_per_world(
	eecs_world_t* world,
	void* userdata
) {
	struct SystemData* system_data = userdata;
	++system_data->cleanup_per_world_called;
}

static void
system_init_per_entity(
	eecs_world_t* world,
	eecs_entity_t entity,
	void* userdata
) {
	struct SystemData* system_data = userdata;

	munit_assert_true(eecs_is_valid_entity(world, entity));
	system_data->seen_entities[system_data->init_per_entity++] = entity;
}

static void
system_cleanup_per_entity(
	eecs_world_t* world,
	eecs_entity_t entity,
	void* userdata
) {
	struct SystemData* system_data = userdata;
	++system_data->cleanup_per_entity;

	munit_assert_true(eecs_is_valid_entity(world, entity));
}

static MunitResult
init_cleanup(const MunitParameter params[], void* fixture) {
	eecs_t* ecs = eecs_create((eecs_options_t) { 0 });

	eecs_component_t comp_A = EECS_HANDLE_INIT;
	eecs_component_t comp_B = EECS_HANDLE_INIT;
	eecs_component_t comp_C = EECS_HANDLE_INIT;
	eecs_register_component(ecs, &comp_A, (eecs_component_options_t){
		.size = sizeof(struct A),
		.alignment = _Alignof(struct A),
	});
	eecs_register_component(ecs, &comp_B, (eecs_component_options_t){
		.size = sizeof(struct B),
		.alignment = _Alignof(struct B),
	});
	eecs_register_component(ecs, &comp_C, (eecs_component_options_t){
		.size = sizeof(struct C),
		.alignment = _Alignof(struct C),
	});

	struct SystemData sys_with_init_data = { 0 };
	eecs_register_system(ecs, &sys_with_init_data.handle, (eecs_system_options_t){
		.init_per_world_fn = system_init_per_world,
		.userdata = &sys_with_init_data,
	});

	struct SystemData sys_with_cleanup_data = { 0 };
	eecs_register_system(ecs, &sys_with_cleanup_data.handle, (eecs_system_options_t){
		.require_components = (eecs_component_t[]){
			comp_A,
			EECS_END_OF_LIST,
		},
		.cleanup_per_world_fn = system_cleanup_per_world,
		.init_per_entity_fn = system_init_per_entity,
		.cleanup_per_entity_fn = system_cleanup_per_entity,
		.userdata = &sys_with_cleanup_data,
	});

	eecs_world_t* world = eecs_create_world(ecs, (eecs_world_options_t){ 0 });

	munit_assert_ptr(&sys_with_init_data, ==, eecs_get_per_world_userdata(world, sys_with_init_data.handle));

	struct SystemData late_reg = { 0 };
	eecs_register_system(ecs, &late_reg.handle, (eecs_system_options_t){
		.init_per_world_fn = system_init_per_world,
		.cleanup_per_world_fn = system_cleanup_per_world,
		.require_components = (eecs_component_t[]){
			comp_A,
			comp_C,
			EECS_END_OF_LIST,
		},
		.init_per_entity_fn = system_init_per_entity,
		.cleanup_per_entity_fn = system_cleanup_per_entity,
		.userdata = &late_reg,
	});

	eecs_entity_t entity = eecs_create_entity(world, (eecs_component_init_t[]){
		{
			.component = comp_A,
			.data = &(struct A){
				.a = 3.14f,
			},
		},
		EECS_END_OF_LIST,
	});

	eecs_entity_t entity2 = eecs_create_entity(world, (eecs_component_init_t[]){
		{
			.component = comp_A,
			.data = &(struct A){
				.a = 3.14f,
			},
		},
		{
			.component = comp_B,
			.data = &(struct B){
				.b = 420
			},
		},
		{
			.component = comp_C,
		},
		EECS_END_OF_LIST,
	});
	munit_assert_true(eecs_is_valid_entity(world, entity2));

	struct A* a = eecs_get_component_in_entity(world, entity, comp_A);
	munit_assert_not_null(a);
	munit_assert_float(a->a, ==, 3.14f);

	eecs_destroy_entity(world, entity);

	munit_assert_false(eecs_is_valid_entity(world, entity));
	a = eecs_get_component_in_entity(world, entity, comp_A);

	eecs_destroy_world(world);

	munit_assert_int(sys_with_init_data.init_per_world_called, ==, 1);
	munit_assert_int(sys_with_init_data.cleanup_per_world_called, ==, 0);

	munit_assert_int(sys_with_cleanup_data.init_per_world_called, ==, 0);
	munit_assert_int(sys_with_cleanup_data.cleanup_per_world_called, ==, 1);
	munit_assert_int(sys_with_cleanup_data.init_per_entity, ==, 2);
	munit_assert_int(sys_with_cleanup_data.cleanup_per_entity, ==, 2);
	munit_assert_int(entity.from_1_index, ==, sys_with_cleanup_data.seen_entities[0].from_1_index);
	munit_assert_int(entity.gen, ==, sys_with_cleanup_data.seen_entities[0].gen);
	munit_assert_int(entity2.from_1_index, ==, sys_with_cleanup_data.seen_entities[1].from_1_index);
	munit_assert_int(entity2.gen, ==, sys_with_cleanup_data.seen_entities[1].gen);

	munit_assert_int(late_reg.init_per_world_called, ==, 1);
	munit_assert_int(late_reg.cleanup_per_world_called, ==, 1);
	munit_assert_int(late_reg.init_per_entity, ==, 1);
	munit_assert_int(late_reg.cleanup_per_entity, ==, 1);
	munit_assert_int(entity2.from_1_index, ==, late_reg.seen_entities[0].from_1_index);
	munit_assert_int(entity2.gen, ==, late_reg.seen_entities[0].gen);

	eecs_destroy(ecs);
	return MUNIT_OK;
}

MunitSuite basic = {
	.prefix = "/basic",
	.tests = (MunitTest[]){
		{ .name = "/init_cleanup", .test = init_cleanup },
		{ 0 },
	},
};

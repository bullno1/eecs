#include <munit/munit.h>
#include <eecs.h>
#include "components.h"

struct SystemData {
	int init_per_world_called;
	int cleanup_per_world_called;
	eecs_system_t handle;
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

static MunitResult
init(const MunitParameter params[], void* fixture) {
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
		.cleanup_per_world_fn = system_cleanup_per_world,
		.userdata = &sys_with_cleanup_data,
	});

	eecs_world_t* world = eecs_create_world(ecs, (eecs_world_options_t){ 0 });
	eecs_destroy_world(world);

	munit_assert_int(sys_with_init_data.init_per_world_called, ==, 1);
	munit_assert_int(sys_with_init_data.cleanup_per_world_called, ==, 0);

	munit_assert_int(sys_with_cleanup_data.init_per_world_called, ==, 0);
	munit_assert_int(sys_with_cleanup_data.cleanup_per_world_called, ==, 1);

	eecs_destroy(ecs);
	return MUNIT_OK;
}

MunitSuite basic = {
	.prefix = "/basic",
	.tests = (MunitTest[]){
		{ .name = "/init", .test = init },
		{ 0 },
	},
};

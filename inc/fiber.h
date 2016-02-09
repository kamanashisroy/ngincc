#ifndef NGINEZ_FIBER_H
#define NGINEZ_FIBER_H

C_CAPSULE_START

enum fiber_status {
	FIBER_STATUS_EMPTY = 0, // fiber is empty
	FIBER_STATUS_CREATED, // fiber is just created
	FIBER_STATUS_ACTIVATED, // fiber is activated/stepping
	FIBER_STATUS_DEACTIVATED, // fiber is deactivated/(not stepping)
	FIBER_STATUS_DESTROYED
};

/*
 * It registers a fiber for execution. It is a stackless non-preemptive co-operative multitasking example.
 * @param The fiber() callback function which is called in rotation 
 */
int register_fiber(int (*fiber)(int status));
/*
 * It unregisters a fiber from execution line.
 * @param The fiber() callback function which is called in rotation 
 */
int unregister_fiber(int (*fiber)(int status));

/**
 * It will dispatch all the fibers once
 */
int fiber_module_step();

/**
 * It will dispatch all the fibers iteratively until cancelled
 */
int fiber_module_run();

// TODO fiber suspend function

int fiber_module_init();
int fiber_module_deinit();

C_CAPSULE_END

#endif // NGINEZ_FIBER_H

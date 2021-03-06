#include <stdlib.h>

#include <proctal.h>

struct proctal_global proctal_global = {
	.malloc = malloc,
	.free = free
};

void proctal_global_set_malloc(void *(*f)(size_t))
{
	if (f == NULL) {
		proctal_global.malloc = malloc;
	}

	proctal_global.malloc = f;
}

void proctal_global_set_free(void (*f)(void *))
{
	if (f == NULL) {
		proctal_global.free = free;
	}

	proctal_global.free = f;
}

void proctal_init(struct proctal *p)
{
	p->malloc = proctal_global.malloc;
	p->free = proctal_global.free;
	p->error = 0;

	p->address.region_mask = 0;
	p->address.size = 1;
	p->address.align = 1;
	p->address.read = 1;
	p->address.write = 0;
	p->address.execute = 0;

	p->region.mask = 0;
	p->region.read = 1;
	p->region.write = 0;
	p->region.execute = 0;

	p->watch.addr = NULL;
	p->watch.read = 0;
	p->watch.write = 0;
	p->watch.execute = 0;
}

void proctal_deinit(struct proctal *p)
{
}

proctal proctal_create(void)
{
	return proctal_impl_create();
}

void proctal_destroy(proctal p)
{
	return proctal_impl_destroy(p);
}

void proctal_set_pid(proctal p, int pid)
{
	proctal_impl_set_pid(p, pid);
}

void proctal_set_malloc(proctal p, void *(*malloc)(size_t))
{
	p->malloc = malloc;
}

void proctal_set_free(proctal p, void (*free)(void *))
{
	p->free = free;
}

int proctal_pid(proctal p)
{
	return proctal_impl_pid(p);
}

void *proctal_malloc(proctal p, size_t size)
{
	void *a = p->malloc(size);

	if (a == NULL) {
		proctal_set_error(p, PROCTAL_ERROR_OUT_OF_MEMORY);
	}

	return a;
}

void proctal_free(proctal p, void *addr)
{
	return p->free(addr);
}

void *proctal_align_addr(void *addr, size_t align);

#include <assert.h>
#include <signal.h>
#include <proctal.h>

#include "cmd.h"
#include "printer.h"

static int request_quit = 0;

static void quit(int signum)
{
	request_quit = 1;
}

static int register_signal_handler()
{
	struct sigaction sa = {
		.sa_handler = quit,
		.sa_flags = 0,
	};

	sigemptyset(&sa.sa_mask);

	return sigaction(SIGINT, &sa, NULL) != -1
		&& sigaction(SIGTERM, &sa, NULL) != -1;
}

static void unregister_signal_handler()
{
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

int cli_cmd_watch(struct cli_cmd_watch_arg *arg)
{
	if (!register_signal_handler()) {
		fprintf(stderr, "Failed to set up signal handler.\n");
		return 1;
	}

	proctal p = proctal_create();

	if (proctal_error(p)) {
		cli_print_proctal_error(p);
		proctal_destroy(p);
		return 1;
	}

	if (!arg->read && !arg->write && !arg->execute) {
		fprintf(stderr, "Did not specify what to watch for.\n");
		proctal_destroy(p);
		return 1;
	}

	if (!(arg->read && arg->write && !arg->execute)
		&& !(arg->write && !arg->read && !arg->execute)
		&& !(!arg->write && !arg->read && arg->execute)) {
		fprintf(stderr, "The given combination of read, write and execute options is not supported.\n");
		proctal_destroy(p);
		return 1;
	}

	proctal_set_pid(p, arg->pid);

	proctal_watch_set_address(p, arg->address);
	proctal_watch_set_read(p, arg->read);
	proctal_watch_set_write(p, arg->write);
	proctal_watch_set_execute(p, arg->execute);

	cli_val_attr addr_attr = cli_val_attr_create(CLI_VAL_TYPE_ADDRESS);
	cli_val vaddr = cli_val_create(addr_attr);
	cli_val_attr_destroy(addr_attr);

	// TODO: Should use a data structure that can grow dynamically and has
	// good lookup performance.
	void *matches[10000];
	size_t match_count = 0;

	proctal_freeze(p);

	while (!request_quit) {
		void *addr;

		if (!proctal_watch(p, &addr)) {
			break;
		}

		if (arg->unique) {
			int match = 0;

			for (size_t i = 0; i < match_count; ++i) {
				void *prev = matches[i];

				if (addr == prev) {
					match = 1;
					break;
				}
			}

			if (match) {
				continue;
			} else {
				assert(match_count < sizeof matches / sizeof matches[0]);
				matches[match_count++] = addr;
			}
		}

		cli_val_parse_bin(vaddr, (const char*) &addr, sizeof addr);
		cli_val_print(vaddr, stdout);
		printf("\n");
	}

	unregister_signal_handler();

	cli_val_destroy(vaddr);

	if (proctal_error(p)) {
		cli_print_proctal_error(p);
		proctal_destroy(p);
		return 1;
	}

	proctal_unfreeze(p);

	proctal_destroy(p);

	return 0;
}

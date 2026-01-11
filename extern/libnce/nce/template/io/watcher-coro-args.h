/**
 * \struct nce_io_coro_args
 * \brief Options for ::nce_io_coro_spawn.
 */
struct nce_io_coro_args {
	/**
	 * File descriptor to monitor.
	 */
	int fd;

	/**
	 * Events to monitor for the file descriptor.
	 *
	 * If 0, coroutine starts with the ::nce_io unset. User will have to call
	 * ::nce_io_set and ::nce_io_start manually.
	 */
	int events;
};

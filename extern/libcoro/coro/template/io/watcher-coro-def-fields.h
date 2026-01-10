
	/**
	 * File descriptor to monitor.
	 */
	int fd;

	/**
	 * Events to monitor when the watcher coroutine is started.
	 *
	 * This has no effect if ::cio_coro_def::flags has ::CORO_INACTIVE set.
	 */
	int events;

	/**
	 * Close the file descriptor when the coroutine is destroyed.
	 */
	int close;

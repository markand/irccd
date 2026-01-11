
	/*
	 * Avoid starting the watcher if events is zero because if the whole
	 * cio_coro_args is zero it could start on STDIN_FILENO which may be
	 * undesired.
	 */
	if (!args || args->events == 0)
		evco->coro.flags |= NCE_CORO_INACTIVE;
	else
		nce_io_set(&evco->io, args->fd, args->events);


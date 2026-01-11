	if (args)
		nce_stat_set(&evco->stat, args->path, args->interval);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;


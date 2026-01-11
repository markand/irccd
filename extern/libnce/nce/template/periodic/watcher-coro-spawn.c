
	if (args)
		nce_periodic_set(&evco->periodic,
		                  args->offset,
		                  args->interval,
		                  evco->periodic.rescheduler);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;


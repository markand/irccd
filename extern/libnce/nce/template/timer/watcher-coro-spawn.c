	if (args)
		nce_timer_set(&evco->timer, args->after, args->repeat);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;


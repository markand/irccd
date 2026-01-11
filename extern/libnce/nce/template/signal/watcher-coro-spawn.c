	if (args)
		nce_signal_set(&evco->signal, args->signo);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;


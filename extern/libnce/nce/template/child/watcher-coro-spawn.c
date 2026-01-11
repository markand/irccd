	if (args)
		nce_child_set(&evco->child, args->pid, args->trace);
	else
		evco->coro.flags |= NCE_CORO_INACTIVE;


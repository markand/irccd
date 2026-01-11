
void
nce_child_set(struct nce_child *ev, pid_t pid, int trace)
{
	assert(ev);

	ev_child_set(&ev->child, pid, trace);
}


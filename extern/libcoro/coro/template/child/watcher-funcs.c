
void
cchild_set(struct cchild *ev, pid_t pid, int trace)
{
	assert(ev);

	ev_child_set(&ev->child, pid, trace);
}



void
nce_signal_set(struct nce_signal *ev, int signo)
{
	assert(ev);
	assert(ev->signal.active == 0);

	ev_signal_set(&ev->signal, signo);
}


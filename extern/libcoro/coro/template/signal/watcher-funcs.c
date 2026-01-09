
void
csignal_set(struct csignal *ev, int signo)
{
	assert(ev);
	assert(ev->signal.active == 0);

	ev_signal_set(&ev->signal, signo);
}


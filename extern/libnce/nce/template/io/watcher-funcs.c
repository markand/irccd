
void
nce_io_set(struct nce_io *ev, int fd, int events)
{
	assert(ev);
	assert(ev->io.active == 0);

	ev_io_set(&ev->io, fd, events);
}

void
nce_io_reset(EV_P_ struct nce_io *ev, int fd, int events)
{
	assert(ev);

	ev_io_stop(EV_A_ &ev->io);
	ev_io_set(&ev->io, fd, events);
	ev_io_start(EV_A_ &ev->io);
}


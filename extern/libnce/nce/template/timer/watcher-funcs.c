
void
nce_timer_set(struct nce_timer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);
	assert(ev->timer.active == 0);

	ev_timer_set(&ev->timer, after, repeat);
}

void
nce_timer_restart(EV_P_ struct nce_timer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);

	ev_timer_stop(EV_A_ &ev->timer);
	ev_timer_set(&ev->timer, after, repeat);
	ev_timer_start(EV_A_ &ev->timer);
}

void
nce_timer_again(EV_P_ struct nce_timer *ev)
{
	assert(ev);

	ev_timer_again(EV_A_ &ev->timer);
}


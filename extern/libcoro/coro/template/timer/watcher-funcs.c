
void
ctimer_set(struct ctimer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);
	assert(ev->timer.active == 0);

	ev_timer_set(&ev->timer, after, repeat);
}

void
ctimer_restart(EV_P_ struct ctimer *ev, ev_tstamp after, ev_tstamp repeat)
{
	assert(ev);

	ev_timer_stop(EV_A_ &ev->timer);
	ev_timer_set(&ev->timer, after, repeat);
	ev_timer_start(EV_A_ &ev->timer);
}

void
ctimer_again(EV_P_ struct ctimer *ev)
{
	assert(ev);

	ev_timer_again(EV_A_ &ev->timer);
}



static ev_tstamp
rescheduler_cb(struct ev_periodic *self, ev_tstamp now)
{
	struct cperiodic *ev = CPERIODIC(self, periodic);

	return ev->rescheduler(ev, now);
}

void
cperiodic_set(struct cperiodic *ev,
              ev_tstamp offset,
              ev_tstamp interval,
              cperiodic_rescheduler_t rescheduler
)
{
	assert(ev);
	assert(ev->periodic.active == 0);

	ev->rescheduler = rescheduler;

	if (rescheduler)
		ev_periodic_set(&ev->periodic, offset, interval, rescheduler_cb);
	else
		ev_periodic_set(&ev->periodic, offset, interval, NULL);
}



static ev_tstamp
rescheduler_cb(struct ev_periodic *self, ev_tstamp now)
{
	struct nce_periodic *ev = NCE_PERIODIC(self, periodic);

	return ev->rescheduler(ev, now);
}

void
nce_periodic_set(struct nce_periodic *ev,
                 ev_tstamp offset,
                 ev_tstamp interval,
                 nce_periodic_rescheduler_t rescheduler
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



/**
 * Configure periodic interval, offset and optional rescheduler.
 *
 * It is equivalent to ev_periodic_set.
 */
void
nce_periodic_set(struct nce_periodic *ev,
                 ev_tstamp offset,
                 ev_tstamp interval,
                 nce_periodic_rescheduler_t rescheduler
);


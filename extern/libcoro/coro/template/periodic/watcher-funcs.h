
/**
 * Configure periodic interval, offset and optional rescheduler.
 *
 * It is equivalent to ev_periodic_set.
 */
void
cperiodic_set(struct cperiodic *ev,
              ev_tstamp offset,
              ev_tstamp interval,
              cperiodic_rescheduler_t rescheduler
);


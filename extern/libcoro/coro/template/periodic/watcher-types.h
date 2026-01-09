
/**
 * Function wrapping ev_periodic's rescheduler callback.
 */
typedef ev_tstamp (*cperiodic_rescheduler_t)(struct cperiodic *self, ev_tstamp now);


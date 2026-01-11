
/**
 * Function wrapping ev_periodic's rescheduler callback.
 */
typedef ev_tstamp (* nce_periodic_rescheduler_t)(struct nce_periodic *self, ev_tstamp now);


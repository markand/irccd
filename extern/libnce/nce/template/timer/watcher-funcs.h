
/**
 * Configure watcher.
 *
 * This function is equivalent to ev_timer_set.
 */
void
nce_timer_set(struct nce_timer *ev, ev_tstamp after, ev_tstamp repeat);

/**
 * This function stops the watcher, set its new values and start it again.
 *
 * There is no equivalent in libev.
 */
void
nce_timer_restart(EV_P_ struct nce_timer *ev, ev_tstamp after, ev_tstamp repeat);

/**
 * Rearm the timer.
 *
 * This function is equivalent to ev_timer_again.
 */
void
nce_timer_again(EV_P_ struct nce_timer *ev);



/**
 * Configure watcher.
 *
 * This function is equivalent to ev_timer_set.
 */
void
ctimer_set(struct ctimer *ev, ev_tstamp after, ev_tstamp repeat);

/**
 * This function stops the watcher, set its new values and start it again.
 *
 * There is no equivalent in libev.
 */
void
ctimer_restart(EV_P_ struct ctimer *ev, ev_tstamp after, ev_tstamp repeat);

/**
 * Rearm the timer.
 *
 * This function is equivalent to ev_timer_again.
 */
void
ctimer_again(EV_P_ struct ctimer *ev);


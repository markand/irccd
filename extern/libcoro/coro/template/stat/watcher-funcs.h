
/**
 * Configure watcher.
 *
 * This function is equivalent to ev_stat_set.
 */
void
cstat_set(EV_P_ struct cstat *ev, const char *path, ev_tstamp interval);

/**
 * Update internal stat values immediately.
 *
 * This function is equivalent to ev_stat_set.
 */
void
cstat_stat(EV_P_ struct cstat *ev);


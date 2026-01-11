
/**
 * Configure watcher.
 *
 * This function is equivalent to ev_stat_set.
 */
void
nce_stat_set(struct nce_stat *ev, const char *path, ev_tstamp interval);

/**
 * Update internal stat values immediately.
 *
 * This function is equivalent to ev_stat_set.
 */
void
nce_stat_stat(EV_P_ struct nce_stat *ev);


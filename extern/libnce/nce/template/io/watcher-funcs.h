
/**
 * Configure watcher.
 *
 * This function is equivalent to ev_io_set.
 */
void
nce_io_set(struct nce_io *ev, int fd, int events);

/**
 * This function stops the watcher, set its new values and start it again.
 *
 * There is no equivalent in libev.
 */
void
nce_io_reset(EV_P_ struct nce_io *ev, int fd, int events);


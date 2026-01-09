
/**
 * Configure watcher.
 *
 * This function is equivalent to ev_io_set.
 */
void
cio_set(struct cio *ev, int fd, int events);

/**
 * This function stops the watcher, set its new values and start it again.
 *
 * There is no equivalent in libev.
 */
void
cio_reset(EV_P_ struct cio *ev, int fd, int events);


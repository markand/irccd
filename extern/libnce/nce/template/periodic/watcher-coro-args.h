/**
 * \struct nce_periodic_coro_args
 * \brief Options for ::nce_periodic_coro_spawn.
 */
struct nce_periodic_coro_args {
	/**
	 * (optional)
	 *
	 * Refer to ::nce_periodic_set.
	 */
	ev_tstamp offset;

	/**
	 * (optional)
	 *
	 * Refer to ::nce_periodic_set.
	 */
	ev_tstamp interval;

	/**
	 * (optional)
	 *
	 * Refer to ::nce_periodic_set.
	 */
	nce_periodic_rescheduler_t rescheduler;
};

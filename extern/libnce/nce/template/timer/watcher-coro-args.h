/**
 * \struct nce_timer_coro_args
 * \brief Options for ::nce_timer_coro_spawn.
 */
struct nce_timer_coro_args {
	/**
	 * Refer to ::nce_timer_set.
	 */
	ev_tstamp after;

	/**
	 * Refer to ::nce_timer_set.
	 */
	ev_tstamp repeat;
};

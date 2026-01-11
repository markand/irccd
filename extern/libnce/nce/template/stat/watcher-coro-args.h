/**
 * \struct nce_stat_coro_args
 * \brief Options for ::nce_stat_coro_spawn.
 */
struct nce_stat_coro_args {
	/**
	 * Path to monitor.
	 */
	const char *path;

	/**
	 * Interval for monitoring the path.
	 */
	ev_tstamp interval;
};

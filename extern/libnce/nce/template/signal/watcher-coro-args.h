/**
 * \struct nce_signal_coro_args
 * \brief Options for ::nce_signal_coro_spawn.
 */
struct nce_signal_coro_args {
	/**
	 * Signal to wait on.
	 */
	int signo;
};

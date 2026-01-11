/**
 * \struct nce_child_coro_args
 * \brief Optchildns for ::nce_child_coro_spawn.
 */
struct nce_child_coro_args {
	/**
	 * Process PID to monitor.
	 */
	pid_t pid;

	/**
	 * Set non-zero to monitor stopped/continued events too.
	 */
	int trace;
};

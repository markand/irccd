
	/**
	 * \cond DOXYGEN_PRIVATE
	 */

	/**
	 * (read-only)
	 *
	 * When using a rescheduler, this function pointer wraps ev_periodic
	 * rescheduler_cb to provide cperiodic as argument.
	 *
	 * Do not edit this field directly, use ::nce_periodic_set instead.
	 */
	nce_periodic_rescheduler_t rescheduler;

	/**
	 * \endcond DOXYGEN_PRIVATE
	 */

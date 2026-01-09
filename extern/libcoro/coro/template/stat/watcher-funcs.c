
void
cstat_set(EV_P_ struct cstat *ev, const char *path, ev_tstamp interval)
{
	assert(ev);
	assert(path);

	ev_stat_set(&ev->stat, path, interval);
}

void
cstat_stat(EV_P_ struct cstat *ev)
{
	assert(ev);

	ev_stat_stat(&ev->stat);
}


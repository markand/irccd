
void
nce_stat_set(struct nce_stat *ev, const char *path, ev_tstamp interval)
{
	assert(ev);
	assert(path);

	ev_stat_set(&ev->stat, path, interval);
}

void
nce_stat_stat(EV_P_ struct nce_stat *ev)
{
	assert(ev);

	ev_stat_stat(EV_A_ &ev->stat);
}


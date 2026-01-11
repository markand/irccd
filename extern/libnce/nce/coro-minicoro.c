#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if EV_MULTIPLICITY
#define NCE_CORO_A_ (coro)->loop,
#else
#define NCE_CORO_A_
#endif

#define NCE_CORO_IS_ATTACHED(Co)  ((Co)->flags & NCE_CORO_ATTACHED)
#define NCE_CORO_IS_ESSENTIAL(Co) ((Co)->flags & NCE_CORO_ESSENTIAL)
#define NCE_CORO_IS_FOREVER(Co)   ((Co)->flags & NCE_CORO_FOREVER)

static const char * const statuses[] = {
	[MCO_DEAD]      = "!",
	[MCO_NORMAL]    = "#",
	[MCO_RUNNING]   = "@",
	[MCO_SUSPENDED] = "?"
};

#if defined(NCE_CORO_DEBUG)
__attribute__ ((format(printf, 2, 3)))
static inline void
nce_coro_debug_printf(const struct nce_coro *coro, const char *fmt, ...)
{
	va_list ap;

	enum mco_state state = mco_status(coro->mco_coro);

	va_start(ap, fmt);
	fprintf(stderr, "[coro] <%s> (%s) ", statuses[state], coro->name);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

#define NCE_CORO_DEBUG(Coro, ...) \
        nce_coro_debug_printf((Coro), __VA_ARGS__)

#else
#define NCE_CORO_DEBUG(...) do {} while (0)
#endif

/*
 * NCE_CORO_ABORT(coro, fmt, ...)
 *
 * Print a final message on error output and call abort().
 */
#if !defined(NDEBUG)
#define NCE_CORO_ABORT(Coro, ...)                                                   \
do {                                                                            \
        enum mco_state state = mco_status((Coro)->mco_coro);                    \
                                                                                \
        if (!(Coro)->name)                                                      \
                fprintf(stderr, "[coro] (%p %s) ",                              \
                    (const void *)(Coro), statuses[state]);                     \
        else                                                                    \
                fprintf(stderr, "[coro] (%s %s) ",                              \
                    (Coro)->name, statuses[state]);                             \
                                                                                \
        fprintf(stderr, __VA_ARGS__);                                           \
        fprintf(stderr, "\n");                                                  \
        abort();                                                                \
} while (0)
#endif

/*
 * NCE_CORO_RESUME(Coro)
 *
 * Resume the coroutine.
 *
 * Unoptimized asserts that the coroutine is resumable.
 */
#if !defined(NDEBUG)

#define NCE_CORO_RESUME(Co)                                                         \
do {                                                                            \
        if (mco_status((Co)->mco_coro) != MCO_SUSPENDED)                        \
                NCE_CORO_ABORT((Co), "non-resumable coroutine");                    \
                                                                                \
        NCE_CORO_DEBUG((Co), "resuming");                                           \
        mco_resume((Co)->mco_coro);                                             \
} while (0)

#else

#define NCE_CORO_RESUME(Co)                                                         \
do {                                                                            \
        NCE_CORO_DEBUG((Co), "resuming");                                           \
        mco_resume(coro->mco_coro);                                             \
} while (0)

#endif

/*
 * NCE_CORO_YIELD(Co)
 *
 * Yield the coroutine provided as argument.
 *
 * Unoptimized asserts that the coroutine is running.
 */
#if !defined(NDEBUG)

#define NCE_CORO_YIELD(Co)                                                          \
do {                                                                            \
        if (mco_status((Co)->mco_coro) != MCO_RUNNING)                          \
                NCE_CORO_ABORT((Co), "attempting to yield non-running coroutine");  \
                                                                                \
        mco_yield((Co)->mco_coro);                                              \
} while (0)

#else

#define NCE_CORO_YIELD(Co)                                                          \
do {                                                                            \
        mco_yield((Co)->mco_coro);                                              \
} while (0)

#endif

/*
 * NCE_CORO_RESUME_<ATTACHED|ESSENTIAL|FOREVER>(Co)
 *
 * Resume the coroutine.
 *
 * Attached coroutines will be disabled if they are no longer resumable.
 *
 * Essential coroutines will break the event loop if no longer resumable.
 *
 * Forever coroutines are resumed as-is meaning that the function must never
 * terminate. Unoptimized build asserts the condition.
 *
 * Note that for attached and essential coroutines we need to check their
 * resumable state twice. First when we are about to resume them and just after
 * resuming them as they may terminate just after the resume call.
 */

#define NCE_CORO_RESUME_ATTACHED(Co)                                            \
do {                                                                            \
        if (nce_coro_resumable((Co)))                                           \
                NCE_CORO_RESUME((Co));                                          \
        if (!nce_coro_resumable((Co))) {                                        \
                NCE_CORO_DEBUG((Co), "attached coroutine is dead, removing");   \
                nce_coro_destroy((Co));                                         \
        }                                                                       \
} while (0)

#define NCE_CORO_RESUME_ESSENTIAL(Co)                                           \
do {                                                                            \
        if (nce_coro_resumable((Co)))                                           \
                NCE_CORO_RESUME((Co));                                          \
        if (!nce_coro_resumable((Co))) {                                        \
                NCE_CORO_DEBUG((Co), "essential is dead, stopping loop");       \
                nce_coro_destroy((Co));                                         \
                ev_break(EV_A_ EVBREAK_ALL);                                    \
        }                                                                       \
} while (0)

#if !defined(NDEBUG)

#define NCE_CORO_RESUME_FOREVER(Co)                                             \
do {                                                                            \
        if (!nce_coro_resumable((Co)))                                          \
                NCE_CORO_ABORT((Co), "forever unexpectedly dead");              \
                                                                                \
        NCE_CORO_RESUME((Co));                                                  \
} while (0)

#else

#define NCE_CORO_RESUME_FOREVER(Co)                                             \
do {                                                                            \
        NCE_CORO_RESUME((Co));                                                  \
} while (0)

#endif

/*
 * NCE_CORO_PUSH(Into, Data, Size)
 *
 * Push data into the coroutine `Into`
 */
#if !defined(NDEBUG)

#define NCE_CORO_PUSH(Into, Data, Size)                                             \
do {                                                                            \
        enum mco_result rc;                                                     \
                                                                                \
        /*                                                                      \
         * This should not happen now unless user really broke the code, ooh oh \
         * ooooh.                                                               \
         */                                                                     \
        if ((rc = mco_push((Into)->mco_coro, (Data), (Size))) != MCO_SUCCESS)   \
                NCE_CORO_ABORT((Into), "push: storage error: %d", rc);              \
} while (0)

#else

#define NCE_CORO_PUSH(Into, Data, Size)                                             \
do {                                                                            \
        mco_push((Into)->mco_coro, (Data), (Size));                             \
} while (0)

#endif

/*
 * NCE_CORO_PULL(From, Data, Size)
 *
 * Pull data from coroutine `From`.
 */

#if !defined(NDEBUG)

#define NCE_CORO_PULL(From, Data, Size)                                             \
do {                                                                            \
        enum mco_result rc;                                                     \
                                                                                \
        /*                                                                      \
         * Should not happen now unless user really broke the trophy twice.     \
         */                                                                     \
        if ((rc = mco_pop((From)->mco_coro, (Data), (Size))) != MCO_SUCCESS)    \
                NCE_CORO_ABORT((From), "pull: storage error: %d", rc);              \
} while (0)

#else

#define NCE_CORO_PULL(From, Data, Size)                                             \
do {                                                                            \
        mco_pop((From)->mco_coro, (Data), (Size));                              \
} while (0)

#endif

/*
 * Wrap minicoro entrypoint to the one, passing the loop as argument.
 */
static void
nce_coro_entry_cb(struct mco_coro *self)
{
	struct nce_coro *coro = self->user_data;

	coro->entry(NCE_CORO_A_ coro);
}

/*
 * Define different callbacks depending on the coroutine flags to avoid doing
 * costly branching everytime they are resumed.
 */
static void
nce_coro_prepare_attached_cb(EV_P_ struct ev_prepare *self, int)
{
	struct nce_coro *coro = CORO(self, prepare);

	NCE_CORO_DEBUG(coro, "prepare attached");
	NCE_CORO_RESUME_ATTACHED(coro);
}

static void
nce_coro_check_attached_cb(EV_P_ struct ev_check *self, int)
{
	struct nce_coro *coro = CORO(self, check);

	NCE_CORO_DEBUG(coro, "check attached");
	NCE_CORO_RESUME_ATTACHED(coro);
}

static void
nce_coro_prepare_essential_cb(EV_P_ struct ev_prepare *self, int)
{
	struct nce_coro *coro = CORO(self, prepare);

	NCE_CORO_DEBUG(coro, "prepare essential");
	NCE_CORO_RESUME_ESSENTIAL(coro);
}

static void
nce_coro_check_essential_cb(EV_P_ struct ev_check *self, int)
{
	struct nce_coro *coro = CORO(self, check);

	NCE_CORO_DEBUG(coro, "check essential");
	NCE_CORO_RESUME_ESSENTIAL(coro);
}

static void
nce_coro_prepare_forever_cb(EV_P_ struct ev_prepare *self, int)
{
	struct nce_coro *coro = CORO(self, prepare);

	NCE_CORO_DEBUG(coro, "prepare forever");
	NCE_CORO_RESUME_FOREVER(coro);
}

static void
nce_coro_check_forever_cb(EV_P_ struct ev_check *self, int)
{
	struct nce_coro *coro = CORO(self, check);

	NCE_CORO_DEBUG(coro, "check forever");
	NCE_CORO_RESUME_FOREVER(coro);
}

static inline void
nce_coro_reset(struct nce_coro *coro)
{
	coro->mco_desc = (const struct mco_desc) {};
	coro->mco_coro = NULL;
	coro->prepare = (const struct ev_prepare) {};
	coro->check = (const struct ev_check) {};
	coro->off = 0;
#if EV_MULTIPLICITY
	coro->loop = NULL;
#endif
}

int
nce_coro_create(EV_P_ struct nce_coro *coro)
{
	assert(coro);
	assert(coro->entry);

	enum mco_result rc;
	void (*prepare_cb)(EV_P_ struct ev_prepare *, int);
	void (*check_cb)(EV_P_ struct ev_check *, int);

	nce_coro_reset(coro);

#if !defined(NDEBUG)
	/* Test some flags combinations to avoid mutual exclusion. */
	if (NCE_CORO_IS_ESSENTIAL(coro) && NCE_CORO_IS_FOREVER(coro))
		NCE_CORO_ABORT(coro, "essential and forever coroutines are mutually exclusive");
#endif

	coro->mco_desc = mco_desc_init(nce_coro_entry_cb, coro->stack_size);
	coro->mco_desc.user_data = coro;

#if EV_MULTIPLICITY
	coro->loop = EV_A;
#endif

	if ((rc = mco_create(&coro->mco_coro, &coro->mco_desc)) != MCO_SUCCESS)
		rc = -ENOMEM;
	else if (coro->flags & (NCE_CORO_ATTACHED | NCE_CORO_ESSENTIAL | NCE_CORO_FOREVER)) {
		/* Select prepare/check callbacks depending on the flags. */
		if (NCE_CORO_IS_ATTACHED(coro)) {
			NCE_CORO_DEBUG(coro, "of type attached");
			prepare_cb = nce_coro_prepare_attached_cb;
			check_cb   = nce_coro_check_attached_cb;
		} else if (NCE_CORO_IS_FOREVER(coro)) {
			NCE_CORO_DEBUG(coro, "of type forever");
			prepare_cb = nce_coro_prepare_forever_cb;
			check_cb   = nce_coro_check_forever_cb;
		} else {
			NCE_CORO_DEBUG(coro, "of type essential");
			prepare_cb = nce_coro_prepare_essential_cb;
			check_cb   = nce_coro_check_essential_cb;
		}

		/* Add pre/post loop resumer. */
		ev_prepare_init(&coro->prepare, prepare_cb);
		ev_set_priority(&coro->prepare, coro->priority);
		ev_prepare_start(EV_A_ &coro->prepare);

		ev_check_init(&coro->check, check_cb);
		ev_set_priority(&coro->check, coro->priority);
		ev_check_start(EV_A_ &coro->check);
	}

	return rc;
}

int
nce_coro_spawn(EV_P_ struct nce_coro *coro)
{
	assert(coro);

	int rc;

	if ((rc = nce_coro_create(EV_A_ coro)) == 0)
		NCE_CORO_RESUME(coro);

	return rc;
}

int
nce_coro_resumable(const struct nce_coro *coro)
{
	assert(coro);

	return coro->mco_coro && mco_status(coro->mco_coro) == MCO_SUSPENDED;
}

void
nce_coro_resume(struct nce_coro *coro)
{
	assert(coro);

	NCE_CORO_RESUME(coro);
}

void
nce_coro_yield(void)
{
	struct nce_coro *coro = nce_coro_self();

	NCE_CORO_YIELD(coro);
}

void
nce_coro_idle(void)
{
	struct nce_coro *coro = nce_coro_self();

	for (;;)
		NCE_CORO_YIELD(coro);
}

void
nce_coro_off(void)
{
	struct nce_coro *coro = nce_coro_self();

	for (coro->off = 1; coro->off; )
		NCE_CORO_YIELD(coro);
}

void
nce_coro_on(struct nce_coro *coro)
{
	assert(coro);

	if (coro->off) {
		coro->off = 0;

#if !defined(NDEBUG)
		if (!nce_coro_resumable(coro))
			NCE_CORO_ABORT(coro, "off coroutine is not resumable");
#endif

		NCE_CORO_RESUME(coro);
	}
}

void
nce_coro_return(const void *data, size_t size)
{
	assert(data);

	nce_coro_push(nce_coro_self(), data, size);
}

void
nce_coro_wait(void *data, size_t size)
{
	assert(data);

	nce_coro_pull(nce_coro_self(), data, size);
}

void
nce_coro_push(struct nce_coro *into, const void *data, size_t size)
{
	assert(into);
	assert(into->mco_coro);
	assert(data);
	assert(size);

	struct nce_coro *self = nce_coro_self();

	while (mco_get_bytes_stored(into->mco_coro) != 0) {
		NCE_CORO_DEBUG(self, "push: storage busy, yielding");
		NCE_CORO_YIELD(self);
	}

	NCE_CORO_DEBUG(self, "push: pushing %zu bytes into %s", size, into->name);
	NCE_CORO_DEBUG(self, "push: yield until consumed by %s", into->name);

	NCE_CORO_PUSH(into, data, size);

	while (mco_get_bytes_stored(into->mco_coro) != 0)
		NCE_CORO_YIELD(self);

	/*
	 * Note: if attached, `coro` may point to an already finalized
	 * coroutine, do not use it there.
	 */
	NCE_CORO_DEBUG(self, "push: consumed by returning");
}

void
nce_coro_pull(struct nce_coro *from, void *data, size_t size)
{
	assert(data);

	struct nce_coro *self = nce_coro_self();

	NCE_CORO_DEBUG(self, "pull: requiring %zu bytes", size);

	while (mco_get_bytes_stored(from->mco_coro) != size)
		NCE_CORO_YIELD(self);

	NCE_CORO_PULL(from, data, size);
	NCE_CORO_DEBUG(self, "pull: consumed %zu bytes", size);
}

struct nce_coro *
nce_coro_self(void)
{
	struct mco_coro *current;

	if (!(current = mco_running()))
		return NULL;

	/*
	 * If user is playing with minicoro on its own we may use an invalid
	 * pointer but for the time being we will assume it accepts this
	 * destiny.
	 */
	assert(current->user_data);

	return current->user_data;
}

void
nce_coro_join(struct nce_coro *coro)
{
	assert(coro);

	if (!coro->mco_coro)
		return;

	while (nce_coro_resumable(coro))
		NCE_CORO_RESUME(coro);

	nce_coro_destroy(coro);
}

void
nce_coro_destroy(struct nce_coro *coro)
{
	assert(coro);

	/* Stop event loop hooks if any. */
	ev_prepare_stop(NCE_CORO_A_ &coro->prepare);
	ev_check_stop(NCE_CORO_A_ &coro->check);

	if (coro->mco_coro) {
#if !defined(NDEBUG)
		if (mco_status(coro->mco_coro) != MCO_SUSPENDED &&
		    mco_status(coro->mco_coro) != MCO_DEAD)
			NCE_CORO_ABORT(coro, "attempting to destroy active coroutine");
#endif
		mco_destroy(coro->mco_coro);
		coro->mco_coro = NULL;
	}

	if (coro->finalizer)
		coro->finalizer(NCE_CORO_A_ coro);
}

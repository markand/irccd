#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if EV_MULTIPLICITY
#define CORO_A_ (coro)->loop,
#else
#define CORO_A_
#endif

#define CORO_IS_ATTACHED(Co)  ((Co)->def.flags & CORO_ATTACHED)
#define CORO_IS_ESSENTIAL(Co) ((Co)->def.flags & CORO_ESSENTIAL)
#define CORO_IS_FOREVER(Co)   ((Co)->def.flags & CORO_FOREVER)

static const char * const statuses[] = {
	[MCO_DEAD]      = "!",
	[MCO_NORMAL]    = "#",
	[MCO_RUNNING]   = "@",
	[MCO_SUSPENDED] = "?"
};

#if defined(CORO_DEBUG)
__attribute__ ((format(printf, 2, 3)))
static inline void
coro_debug_printf(const struct coro *coro, const char *fmt, ...)
{
	va_list ap;

	enum mco_state state = mco_status(coro->mco_coro);

	va_start(ap, fmt);
	fprintf(stderr, "[coro] <%s> (%s) ", statuses[state], coro->def.name);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

#define CORO_DEBUG(Coro, ...) \
        coro_debug_printf((Coro), __VA_ARGS__)

#else
#define CORO_DEBUG(...) do {} while (0)
#endif

/*
 * CORO_ABORT(coro, fmt, ...)
 *
 * Print a final message on error output and call abort().
 */
#if !defined(NDEBUG)
#define CORO_ABORT(Coro, ...)                                                   \
do {                                                                            \
        enum mco_state state = mco_status((Coro)->mco_coro);                    \
                                                                                \
        fprintf(stderr, "[coro] (%s %s) ", (Coro)->def.name, statuses[state]);  \
        fprintf(stderr, __VA_ARGS__);                                           \
        fprintf(stderr, "\n");                                                  \
        abort();                                                                \
} while (0)
#endif

/*
 * CORO_RESUME(Coro)
 *
 * Resume the coroutine.
 *
 * Unoptimized asserts that the coroutine is resumable.
 */
#if !defined(NDEBUG)

#define CORO_RESUME(Co)                                                         \
do {                                                                            \
        if (mco_status((Co)->mco_coro) != MCO_SUSPENDED)                        \
                CORO_ABORT((Co), "non-resumable coroutine");                    \
                                                                                \
        CORO_DEBUG((Co), "resuming");                                           \
        mco_resume((Co)->mco_coro);                                             \
} while (0)

#else

#define CORO_RESUME(Co)                                                         \
do {                                                                            \
        CORO_DEBUG((Co), "resuming");                                           \
        mco_resume(coro->mco_coro);                                             \
} while (0)

#endif

/*
 * CORO_YIELD(Co)
 *
 * Yield the coroutine provided as argument.
 *
 * Unoptimized asserts that the coroutine is running.
 */
#if !defined(NDEBUG)

#define CORO_YIELD(Co)                                                          \
do {                                                                            \
        if (mco_status((Co)->mco_coro) != MCO_RUNNING)                          \
                CORO_ABORT((Co), "attempting to yield non-running coroutine");  \
                                                                                \
        mco_yield((Co)->mco_coro);                                              \
} while (0)

#else

#define CORO_YIELD(Co)                                                          \
do {                                                                            \
        mco_yield((Co)->mco_coro);                                              \
} while (0)

#endif

/*
 * CORO_RESUME_<ATTACHED|ESSENTIAL|FOREVER>(Co)
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

#define CORO_RESUME_ATTACHED(Co)                                                \
do {                                                                            \
        if (coro_resumable((Co)))                                               \
                CORO_RESUME((Co));                                              \
        if (!coro_resumable((Co))) {                                            \
                CORO_DEBUG((Co), "attached coroutine is dead, removing");       \
                coro_finish((Co));                                              \
        }                                                                       \
} while (0)

#define CORO_RESUME_ESSENTIAL(Co)                                               \
do {                                                                            \
        if (coro_resumable((Co)))                                               \
                CORO_RESUME((Co));                                              \
        if (!coro_resumable((Co))) {                                            \
                CORO_DEBUG((Co), "essential is dead, stopping loop");           \
                coro_finish((Co));                                              \
                ev_break(EV_A_ EVBREAK_ALL);                                    \
        }                                                                       \
} while (0)

#if !defined(NDEBUG)

#define CORO_RESUME_FOREVER(Co)                                                 \
do {                                                                            \
        if (!coro_resumable((Co)))                                              \
                CORO_ABORT((Co), "forever unexpectedly dead");                  \
                                                                                \
        CORO_RESUME((Co));                                                      \
} while (0)

#else

#define CORO_RESUME_FOREVER(Co)                                                 \
do {                                                                            \
        CORO_RESUME((Co));                                                      \
} while (0)

#endif

/*
 * CORO_PUSH(Self, Into, Data, Size)
 *
 * Push data into the coroutine `Into`
 */
#if !defined(NDEBUG)

#define CORO_PUSH(Self, Into, Data, Size)                                       \
do {                                                                            \
        enum mco_result rc;                                                     \
                                                                                \
        /*                                                                      \
         * This should not happen now unless user really broke the code, ooh oh \
         * ooooh.                                                               \
         */                                                                     \
        if ((rc = mco_push((Into)->mco_coro, (Data), (Size))) != MCO_SUCCESS)   \
                CORO_ABORT((Self), "push: storage error: %d", rc);              \
} while (0)

#else

#define CORO_PUSH(Self, Into, Data, Size)                                       \
do {                                                                            \
        mco_push((Into)->mco_coro, (Data), (Size));                             \
} while (0)

#endif

/*
 * CORO_PULL(Self, From, Data, Size)
 *
 * Pull data from coroutine `From`.
 */

#if !defined(NDEBUG)

#define CORO_PULL(Self, From, Data, Size)                                       \
do {                                                                            \
        enum mco_result rc;                                                     \
                                                                                \
        /*                                                                      \
         * Should not happen now unless user really broke the trophy twice.     \
         */                                                                     \
        if ((rc = mco_pop((From)->mco_coro, (Data), (Size))) != MCO_SUCCESS)    \
                CORO_ABORT((Self), "pull: storage error: %d", rc);              \
} while (0)

#else

#define CORO_PULL(Self, From, Data, Size)                                       \
do {                                                                            \
        mco_pop((From)->mco_coro, (Data), (Size));                              \
} while (0)

#endif

/*
 * Wrap minicoro entrypoint to the one, passing the loop as argument.
 */
static void
coro_entry_cb(struct mco_coro *self)
{
	struct coro *coro = self->user_data;

	coro->def.entry(CORO_A_ coro);
}

/*
 * Define different callbacks depending on the coroutine flags to avoid doing
 * costly branching everytime they are resumed.
 */
static void
coro_prepare_attached_cb(EV_P_ struct ev_prepare *self, int)
{
	struct coro *coro = CORO(self, prepare);

	CORO_DEBUG(coro, "prepare attached");
	CORO_RESUME_ATTACHED(coro);
}

static void
coro_check_attached_cb(EV_P_ struct ev_check *self, int)
{
	struct coro *coro = CORO(self, check);

	CORO_DEBUG(coro, "check attached");
	CORO_RESUME_ATTACHED(coro);
}

static void
coro_prepare_essential_cb(EV_P_ struct ev_prepare *self, int)
{
	struct coro *coro = CORO(self, prepare);

	CORO_DEBUG(coro, "prepare essential");
	CORO_RESUME_ESSENTIAL(coro);
}

static void
coro_check_essential_cb(EV_P_ struct ev_check *self, int)
{
	struct coro *coro = CORO(self, check);

	CORO_DEBUG(coro, "check essential");
	CORO_RESUME_ESSENTIAL(coro);
}

static void
coro_prepare_forever_cb(EV_P_ struct ev_prepare *self, int)
{
	struct coro *coro = CORO(self, prepare);

	CORO_DEBUG(coro, "prepare forever");
	CORO_RESUME_FOREVER(coro);
}

static void
coro_check_forever_cb(EV_P_ struct ev_check *self, int)
{
	struct coro *coro = CORO(self, check);

	CORO_DEBUG(coro, "check forever");
	CORO_RESUME_FOREVER(coro);
}

static inline void
coro_reset(struct coro *coro)
{
	coro->def = (const struct coro_def) {};
	coro->mco_desc = (const struct mco_desc) {};
	coro->mco_coro = NULL;
	coro->off = 0;
#if EV_MULTIPLICITY
	coro->loop = NULL;
#endif
}

void
coro_init(struct coro *coro)
{
	assert(coro);

	coro_reset(coro);

	coro->prepare = (const struct ev_prepare) {};
	coro->check = (const struct ev_check) {};
}

void
coro_set_name(struct coro *coro, const char *name)
{
	assert(coro);

	if (name)
		coro->def.name = name;
	else
		coro->def.name = CORO_DEFAULT_NAME;
}

void
coro_set_priority(struct coro *coro, int priority)
{
	assert(coro);

	coro->def.priority = priority;
}

void
coro_set_flags(struct coro *coro, unsigned int flags)
{
	assert(coro);

	coro->def.flags = flags;
}

void
coro_set_stack_size(struct coro *coro, size_t stack_size)
{
	assert(coro);

	coro->def.stack_size = stack_size;
}

void
coro_set_entry(struct coro *coro, coro_entry_t entry)
{
	assert(coro);
	assert(entry);

	coro->def.entry = entry;
}

void
coro_set_finalizer(struct coro *coro, coro_finalizer_t finalizer)
{
	assert(coro);
	assert(finalizer);

	coro->def.finalizer = finalizer;
}

int
coro_create(EV_P_ struct coro *coro)
{
	assert(coro);
	assert(coro->def.entry);

	enum mco_result rc;
	void (*prepare_cb)(EV_P_ struct ev_prepare *, int);
	void (*check_cb)(EV_P_ struct ev_check *, int);

#if !defined(NDEBUG)
	/* Test some flags combinations to avoid mutual exclusion. */
	if (CORO_IS_ESSENTIAL(coro) && CORO_IS_FOREVER(coro))
		CORO_ABORT(coro, "essential and forever coroutines are mutually exclusive");
#endif

	coro->mco_desc = mco_desc_init(coro_entry_cb, coro->def.stack_size);
	coro->mco_desc.user_data = coro;

#if EV_MULTIPLICITY
	coro->loop = EV_A;
#endif

	if ((rc = mco_create(&coro->mco_coro, &coro->mco_desc)) != MCO_SUCCESS)
		rc = -ENOMEM;
	else if (coro->def.flags & (CORO_ATTACHED | CORO_ESSENTIAL | CORO_FOREVER)) {
		/* Select prepare/check callbacks depending on the flags. */
		if (CORO_IS_ATTACHED(coro)) {
			CORO_DEBUG(coro, "of type attached");
			prepare_cb = coro_prepare_attached_cb;
			check_cb   = coro_check_attached_cb;
		} else if (CORO_IS_FOREVER(coro)) {
			CORO_DEBUG(coro, "of type forever");
			prepare_cb = coro_prepare_forever_cb;
			check_cb   = coro_check_forever_cb;
		} else {
			CORO_DEBUG(coro, "of type essential");
			prepare_cb = coro_prepare_essential_cb;
			check_cb   = coro_check_essential_cb;
		}

		/* Add pre/post loop resumer. */
		ev_prepare_init(&coro->prepare, prepare_cb);
		ev_set_priority(&coro->prepare, coro->def.priority);
		ev_prepare_start(EV_A_ &coro->prepare);

		ev_check_init(&coro->check, check_cb);
		ev_set_priority(&coro->check, coro->def.priority);
		ev_check_start(EV_A_ &coro->check);
	}

	return rc;
}

int
coro_spawn(EV_P_ struct coro *coro, const struct coro_def *def)
{
	assert(coro);
	assert(def);

	int rc;

	coro_init(coro);
	coro_set_name(coro, def->name);
	coro_set_priority(coro, def->priority);
	coro_set_stack_size(coro, def->stack_size);
	coro_set_flags(coro, def->flags);
	coro_set_entry(coro, def->entry);

	if (def->finalizer)
		coro_set_finalizer(coro, def->finalizer);

	if ((rc = coro_create(EV_A_ coro)) < 0)
		coro_finish(coro);
	else
		CORO_RESUME(coro);

	return rc;
}

int
coro_resumable(const struct coro *coro)
{
	assert(coro);

	return coro->mco_coro && mco_status(coro->mco_coro) == MCO_SUSPENDED;
}

void
coro_resume(struct coro *coro)
{
	assert(coro);

	CORO_RESUME(coro);
}

void
coro_yield(void)
{
	struct coro *coro = coro_self();

	CORO_YIELD(coro);
}

void
coro_idle(void)
{
	struct coro *coro = coro_self();

	for (;;)
		CORO_YIELD(coro);
}

void
coro_off(void)
{
	struct coro *coro = coro_self();

	for (coro->off = 1; coro->off; )
		CORO_YIELD(coro);
}

void
coro_on(struct coro *coro)
{
	assert(coro);

	if (coro->off) {
		coro->off = 0;

#if !defined(NDEBUG)
		if (!coro_resumable(coro))
			CORO_ABORT(coro, "off coroutine is not resumable");
#endif

		CORO_RESUME(coro);
	}
}

void
coro_return(const void *data, size_t size)
{
	assert(data);

	coro_push(coro_self(), data, size);
}

void
coro_wait(void *data, size_t size)
{
	assert(data);

	coro_pull(coro_self(), data, size);
}

void
coro_push(struct coro *into, const void *data, size_t size)
{
	assert(into);
	assert(into->mco_coro);
	assert(data);
	assert(size);

	struct coro *self = coro_self();

	while (mco_get_bytes_stored(into->mco_coro) != 0) {
		CORO_DEBUG(self, "push: storage busy, yielding");
		CORO_YIELD(self);
	}

	CORO_DEBUG(self, "push: pushing %zu bytes into %s", size, into->def.name);
	CORO_DEBUG(self, "push: yield until consumed by %s", into->def.name);

	CORO_PUSH(self, into, data, size);

	while (mco_get_bytes_stored(into->mco_coro) != 0)
		CORO_YIELD(self);

	/*
	 * Note: if attached, `coro` may point to an already finalized
	 * coroutine, do not use it there.
	 */
	CORO_DEBUG(self, "push: consumed by returning");
}

void
coro_pull(struct coro *from, void *data, size_t size)
{
	assert(data);

	struct coro *self = coro_self();

	CORO_DEBUG(self, "pull: requiring %zu bytes", size);

	while (mco_get_bytes_stored(from->mco_coro) != size)
		CORO_YIELD(self);

	CORO_PULL(self, from, data, size);
	CORO_DEBUG(self, "pull: consumed %zu bytes", size);
}

struct coro *
coro_self(void)
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
coro_join(struct coro *coro)
{
	assert(coro);

	if (!coro->mco_coro)
		return;

	while (coro_resumable(coro))
		CORO_RESUME(coro);

	coro_finish(coro);
}

void
coro_finish(struct coro *coro)
{
	assert(coro);

	/* Stop event loop hooks if any. */
	ev_prepare_stop(CORO_A_ &coro->prepare);
	ev_check_stop(CORO_A_ &coro->check);

	if (coro->mco_coro) {
#if !defined(NDEBUG)
		if (mco_status(coro->mco_coro) != MCO_SUSPENDED &&
		    mco_status(coro->mco_coro) != MCO_DEAD)
			CORO_ABORT(coro, "attempting to destroy active coroutine");
#endif
		mco_destroy(coro->mco_coro);
		coro->mco_coro = NULL;
	}

	if (coro->def.finalizer)
		coro->def.finalizer(CORO_A_ coro);
}

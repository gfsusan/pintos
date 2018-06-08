#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

/* Random value for struct thread's `magic' member.
Used to detect stack overflow.  See the big comment at the top
of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
that are ready to run but not actually running. */
static struct list ready_list;
static struct list ready_list_fq0;
static struct list ready_list_fq1;
static struct list ready_list_fq2;
static struct list ready_list_fq3;
static struct list ready_list_fq4;


/* List of all processes.  Processes are added to this list
when they are first scheduled and removed when they exit. */
static struct list all_list;

/* List of process in sleep */
static struct list sleep_list;
static int64_t next_tick_to_wakeup = INT64_MAX;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame
{
	void *eip;                  /* Return address. */
	thread_func *function;      /* Function to call. */
	void *aux;                  /* Auxiliary data for function. */
};

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

								/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
#define MAX_TIME_SLICE 6		/* Max # of timer ticks to give thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

static void kernel_thread(thread_func *, void *aux);

static void idle(void *aux UNUSED);
static struct thread *running_thread(void);
static struct thread *next_thread_to_run(void);
static void init_thread(struct thread *, const char *name, int priority);
static bool is_thread(struct thread *) UNUSED;
static void *alloc_frame(struct thread *, size_t size);
static void schedule(void);
void thread_schedule_tail(struct thread *prev);
static tid_t allocate_tid(void);
struct list_elem *addAge(struct thread *);   // added


/* added by jy for MFQ
thread 의 priority 를 입력받아서
해당 우선순위의 쓰레드 리스트를 반환
*/
struct list * thread_get_fq(int p)
{
	switch (p) {
	case 0: return &ready_list_fq0;
	case 1: return &ready_list_fq1;
	case 2: return &ready_list_fq2;
	case 3: return &ready_list_fq3;
	case 4: return &ready_list_fq4;
	}
}
////////////////////////////////

/* Initializes the threading system by transforming the code
that's currently running into a thread.  This can't work in
general and it is possible in this case only because loader.S
was careful to put the bottom of the stack at a page boundary.

Also initializes the run queue and the tid lock.

After calling this function, be sure to initialize the page
allocator before trying to create any threads with
thread_create().

It is not safe to call thread_current() until this function
finishes. */
void
thread_init(void)
{
	ASSERT(intr_get_level() == INTR_OFF);

	lock_init(&tid_lock);
	list_init(&ready_list);
	/* added by jy for MFQ */
	list_init(&ready_list_fq0);
	list_init(&ready_list_fq1);
	list_init(&ready_list_fq2);
	list_init(&ready_list_fq3);
	list_init(&ready_list_fq4);
	/////////////////////////

	list_init(&all_list);
	list_init(&sleep_list);

	/* Set up a thread structure for the running thread. */
	initial_thread = running_thread();
	init_thread(initial_thread, "main", PRI_DEFAULT);
	initial_thread->status = THREAD_RUNNING;
	initial_thread->tid = allocate_tid();
}

/* Starts preemptive thread scheduling by enabling interrupts.
Also creates the idle thread. */
void
thread_start(void)
{
	/* Create the idle thread. */
	struct semaphore idle_started;
	sema_init(&idle_started, 0);
	thread_create("idle", PRI_MIN, idle, &idle_started);

	/* Start preemptive thread scheduling. */
	intr_enable();

	/* Wait for the idle thread to initialize idle_thread. */
	sema_down(&idle_started);
}


/* Called by the timer interrupt handler at each timer tick.
Thus, this function runs in an external interrupt context. */
void
thread_tick(void)
{
	struct thread *t = thread_current();
	// 추가됨
	int current_priority = thread_get_priority();
	struct list *lower_list;
	struct thread *lower_cur_t;
	struct list_elem *next_elem;
	/* Update statistics. */
	if (t == idle_thread)
		idle_ticks++;
#ifdef USERPROG
	else if (t->pagedir != NULL)
		user_ticks++;
#endif
	else
		kernel_ticks++;

	// 현재 thread보다 작은 priority를 가진 Ready State의 thread age++
	// 만약에 age가 20보다 크면 priority 증가 시키고 age = 0으로 초기화
	for (int i = current_priority - 1; i >= 0; i--) {
		lower_list = thread_get_fq(i);

		if (!list_empty(lower_list)) {

			next_elem = list_begin(lower_list);								// 현재 fq의 첫 list_elem * 반환

			do {
				lower_cur_t = list_entry(next_elem, struct thread, elem);   // next_elem을 갖는 struct thread 반환
				next_elem = addAge(lower_cur_t);							// 현재 thread의 Age에 대한 기능 다 하고, next_elem 반환
			} while (next_elem != list_end(lower_list));					// 현재 thread의 elem이 fq의 마지막이 아니면, next가 존재
			
		}
	}

	/* Enforce preemption. */
	// thread_ticks를 1 증가하고 정해진 TimeSlice를 계산해 크면 intr_yield_on_return()
	if (++thread_ticks >= MAX_TIME_SLICE - current_priority) {	/* 현재 Thread의 Priority에 따른 Time Slice를 계산한다. */
		//printf("thread_ticks = %d, time slice = %d\n", thread_tick, getTimeSlice(current_priority));
		intr_yield_on_return();
	}
}


/* 받아온 thread의 age를 1 증가시키고
age가 20보다 크면, age를 0으로 바꾸고 priority를 1 증가키고
현재 fq에서 삭제 후 priority하나 높은 fq에 push back 한다. */
struct list_elem * addAge(struct thread* cur_t) {
	struct list_elem *next_elem;
	struct list *nextfq;

	cur_t->age++;
	
	if (cur_t->age >= 20) {
		cur_t->priority++;
		cur_t->age = 0;
		printf("Thread %s's age has reached 20.\n", cur_t->name);
		printf("Thread %s's priority has been changed  from %d to %d.\n", cur_t->name, cur_t->priority - 1, cur_t->priority);

		// 기존 readyqueue에서 지우고 readyqueue 하나 올리기
		nextfq = thread_get_fq(cur_t->priority);		// 증가된 priority의 fq 반환
		next_elem = list_remove(&cur_t->elem);			// 현재 thread 기존 fq에서 제거 후 next_elem 반환
		list_push_back(nextfq, &cur_t->elem);			// 현재 thread 새로운 fq에 push back
	}
	else
		next_elem = list_next(&cur_t->elem);
	return next_elem;

}


/* Prints thread statistics. */
void
thread_print_stats(void)
{
	printf("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
		idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
PRIORITY, which executes FUNCTION passing AUX as the argument,
and adds it to the ready queue.  Returns the thread identifier
for the new thread, or TID_ERROR if creation fails.

If thread_start() has been called, then the new thread may be
scheduled before thread_create() returns.  It could even exit
before thread_create() returns.  Contrariwise, the original
thread may run for any amount of time before the new thread is
scheduled.  Use a semaphore or some other form of
synchronization if you need to ensure ordering.

The code provided sets the new thread's `priority' member to
PRIORITY, but no actual priority scheduling is implemented.
Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create(const char *name, int priority,
	thread_func *function, void *aux)
{
	struct thread *t;
	struct kernel_thread_frame *kf;
	struct switch_entry_frame *ef;
	struct switch_threads_frame *sf;
	tid_t tid;

	ASSERT(function != NULL);

	/* Allocate thread. */
	t = palloc_get_page(PAL_ZERO);
	if (t == NULL)
		return TID_ERROR;

	/* Initialize thread. */
	init_thread(t, name, priority);
	tid = t->tid = allocate_tid();

	/* Stack frame for kernel_thread(). */
	kf = alloc_frame(t, sizeof *kf);
	kf->eip = NULL;
	kf->function = function;
	kf->aux = aux;

	/* Stack frame for switch_entry(). */
	ef = alloc_frame(t, sizeof *ef);
	ef->eip = (void(*) (void)) kernel_thread;

	/* Stack frame for switch_threads(). */
	sf = alloc_frame(t, sizeof *sf);
	sf->eip = switch_entry;
	sf->ebp = 0;

	/* Add to run queue. */
	thread_unblock(t);

	return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
again until awoken by thread_unblock().

This function must be called with interrupts turned off.  It
is usually a better idea to use one of the synchronization
primitives in synch.h. */
void
thread_block(void)
{
	ASSERT(!intr_context());
	ASSERT(intr_get_level() == INTR_OFF);

	thread_current()->status = THREAD_BLOCKED;
	schedule();
}

/* Transitions a blocked thread T to the ready-to-run state.
This is an error if T is not blocked.  (Use thread_yield() to
make the running thread ready.)

This function does not preempt the running thread.  This can
be important: if the caller had disabled interrupts itself,
it may expect that it can atomically unblock a thread and
update other data. */
void
thread_unblock(struct thread *t)
{
	struct list * tmp_list;
	enum intr_level old_level;

	ASSERT(is_thread(t));

	old_level = intr_disable();
	ASSERT(t->status == THREAD_BLOCKED);

	/* added by jy for MFQ
	ready_list 변수에 우선순위별로 ready_list를 할당 */
	tmp_list = thread_get_fq(t->priority);
	////////////////////////

	list_push_back(tmp_list, &t->elem);
	t->status = THREAD_READY;
	intr_set_level(old_level);
}

static void
update_next_tick_to_wakeup(int64_t tick)
{
	next_tick_to_wakeup =
		(next_tick_to_wakeup > tick) ? tick : next_tick_to_wakeup;
}

int64_t
get_next_tick_to_wakeup(void)
{
	return next_tick_to_wakeup;
}

/* Wakes up this thread after ticks */
void
thread_sleep(int64_t tick)
{
	struct thread *cur;
	enum intr_level old_level;

	old_level = intr_disable();
	cur = thread_current();

	ASSERT(cur != idle_thread);

	update_next_tick_to_wakeup(cur->wakeup_tick = tick);
	list_push_back(&sleep_list, &cur->elem);

	thread_block();

	intr_set_level(old_level);
}

void
thread_wakeup(int64_t current_tick)
{
	struct list_elem *e;

	next_tick_to_wakeup = INT64_MAX;

	e = list_begin(&sleep_list);
	while (e != list_end(&sleep_list))
	{
		struct thread *t = list_entry(e, struct thread, elem);
		if (current_tick >= t->wakeup_tick)
		{
			e = list_remove(&t->elem);
			thread_unblock(t);
		}
		else
		{
			e = list_next(e);
			update_next_tick_to_wakeup(t->wakeup_tick);
		}
	}
}

/* Returns the name of the running thread. */
const char *
thread_name(void)
{
	return thread_current()->name;
}

/* Returns the running thread.
This is running_thread() plus a couple of sanity checks.
See the big comment at the top of thread.h for details. */
struct thread *
	thread_current(void)
{
	struct thread *t = running_thread();

	/* Make sure T is really a thread.
	If either of these assertions fire, then your thread may
	have overflowed its stack.  Each thread has less than 4 kB
	of stack, so a few big automatic arrays or moderate
	recursion can cause stack overflow. */
	ASSERT(is_thread(t));
	ASSERT(t->status == THREAD_RUNNING);

	return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid(void)
{
	return thread_current()->tid;
}

/* Deschedules the current thread and destroys it.  Never
returns to the caller. */
void
thread_exit(void)
{
	ASSERT(!intr_context());

#ifdef USERPROG
	process_exit();
#endif

	/* Remove thread from all threads list, set our status to dying,
	and schedule another process.  That process will destroy us
	when it calls thread_schedule_tail(). */
	intr_disable();
	list_remove(&thread_current()->allelem);
	thread_current()->status = THREAD_DYING;
	schedule();
	NOT_REACHED();
}

/* Yields the CPU.  The current thread is not put to sleep and
may be scheduled again immediately at the scheduler's whim. */
void
thread_yield(void)
{
	struct thread *cur = thread_current();
	struct list * tmp_list;
	enum intr_level old_level;

	ASSERT(!intr_context());

	old_level = intr_disable();
	if (cur != idle_thread)
	{
		/* added by jy for MFQ
		ready_list 변수에 우선순위별로 ready_list를 할당 */
		tmp_list = thread_get_fq(cur->priority);
		////////////////////////

		list_push_back(tmp_list, &cur->elem);
	}
	cur->status = THREAD_READY;
	schedule();
	intr_set_level(old_level);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
This function must be called with interrupts off. */
void
thread_foreach(thread_action_func *func, void *aux)
{
	struct list_elem *e;

	ASSERT(intr_get_level() == INTR_OFF);

	for (e = list_begin(&all_list); e != list_end(&all_list);
		e = list_next(e))
	{
		struct thread *t = list_entry(e, struct thread, allelem);
		func(t, aux);
	}
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority(int new_priority)
{
	thread_current()->priority = new_priority;
}

/* Returns the current thread's priority. */
int
thread_get_priority(void)
{
	return thread_current()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice(int nice UNUSED)
{
	/* Not yet implemented. */
}

/* Returns the current thread's nice value. */
int
thread_get_nice(void)
{
	/* Not yet implemented. */
	return 0;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg(void)
{
	/* Not yet implemented. */
	return 0;
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu(void)
{
	/* Not yet implemented. */
	return 0;
}

/* Idle thread.  Executes when no other thread is ready to run.

The idle thread is initially put on the ready list by
thread_start().  It will be scheduled once initially, at which
point it initializes idle_thread, "up"s the semaphore passed
to it to enable thread_start() to continue, and immediately
blocks.  After that, the idle thread never appears in the
ready list.  It is returned by next_thread_to_run() as a
special case when the ready list is empty. */
static void
idle(void *idle_started_ UNUSED)
{
	struct semaphore *idle_started = idle_started_;
	idle_thread = thread_current();
	sema_up(idle_started);

	for (;;)
	{
		/* Let someone else run. */
		intr_disable();
		thread_block();

		/* Re-enable interrupts and wait for the next one.

		The `sti' instruction disables interrupts until the
		completion of the next instruction, so these two
		instructions are executed atomically.  This atomicity is
		important; otherwise, an interrupt could be handled
		between re-enabling interrupts and waiting for the next
		one to occur, wasting as much as one clock tick worth of
		time.

		See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
		7.11.1 "HLT Instruction". */
		asm volatile ("sti; hlt" : : : "memory");
	}
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread(thread_func *function, void *aux)
{
	ASSERT(function != NULL);

	intr_enable();       /* The scheduler runs with interrupts off. */
	function(aux);       /* Execute the thread function. */
	thread_exit();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
	running_thread(void)
{
	uint32_t *esp;

	/* Copy the CPU's stack pointer into `esp', and then round that
	down to the start of a page.  Because `struct thread' is
	always at the beginning of a page and the stack pointer is
	somewhere in the middle, this locates the curent thread. */
	asm("mov %%esp, %0" : "=g" (esp));
	return pg_round_down(esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread(struct thread *t)
{
	return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
NAME. */
static void
init_thread(struct thread *t, const char *name, int priority)
{
	enum intr_level old_level;

	ASSERT(t != NULL);
	ASSERT(PRI_MIN <= priority && priority <= PRI_MAX);
	ASSERT(name != NULL);

	memset(t, 0, sizeof *t);
	t->status = THREAD_BLOCKED;
	strlcpy(t->name, name, sizeof t->name);
	t->stack = (uint8_t *)t + PGSIZE;
	t->priority = priority;
	t->magic = THREAD_MAGIC;

	old_level = intr_disable();
	list_push_back(&all_list, &t->allelem);
	intr_set_level(old_level);
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
returns a pointer to the frame's base. */
static void *
alloc_frame(struct thread *t, size_t size)
{
	/* Stack data is always allocated in word-size units. */
	ASSERT(is_thread(t));
	ASSERT(size % sizeof(uint32_t) == 0);

	t->stack -= size;
	return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
return a thread from the run queue, unless the run queue is
empty.  (If the running thread can continue running, then it
will be in the run queue.)  If the run queue is empty, return
idle_thread. */
static struct thread *
next_thread_to_run(void)
{
	if (list_empty(&ready_list_fq4))
	{
		if (list_empty(&ready_list_fq3))
		{
			if (list_empty(&ready_list_fq2))
			{
				if (list_empty(&ready_list_fq1))
				{
					if (list_empty(&ready_list_fq0))
					{
						return idle_thread;
					}
					else
					{
						ready_list = ready_list_fq0;
						return list_entry(list_pop_front(&ready_list), struct thread, elem);
					}
				}
				else
				{
					ready_list = ready_list_fq1;
					return list_entry(list_pop_front(&ready_list), struct thread, elem);
				}
			}
			else
			{
				ready_list = ready_list_fq2;
				return list_entry(list_pop_front(&ready_list), struct thread, elem);
			}
		}
		else
		{
			ready_list = ready_list_fq3;
			return list_entry(list_pop_front(&ready_list), struct thread, elem);
		}
	}
	else
	{
		ready_list = ready_list_fq4;
		return list_entry(list_pop_front(&ready_list), struct thread, elem);
	}
}

/* Completes a thread switch by activating the new thread's page
tables, and, if the previous thread is dying, destroying it.

At this function's invocation, we just switched from thread
PREV, the new thread is already running, and interrupts are
still disabled.  This function is normally invoked by
thread_schedule() as its final action before returning, but
the first time a thread is scheduled it is called by
switch_entry() (see switch.S).

It's not safe to call printf() until the thread switch is
complete.  In practice that means that printf()s should be
added at the end of the function.

After this function and its caller returns, the thread switch
is complete. */
void
thread_schedule_tail(struct thread *prev)
{
	struct thread *cur = running_thread();

	ASSERT(intr_get_level() == INTR_OFF);

	/* Mark us as running. */
	cur->status = THREAD_RUNNING;

	/* Start new time slice. */
	thread_ticks = 0;

#ifdef USERPROG
	/* Activate the new address space. */
	process_activate();
#endif

	/* If the thread we switched from is dying, destroy its struct
	thread.  This must happen late so that thread_exit() doesn't
	pull out the rug under itself.  (We don't free
	initial_thread because its memory was not obtained via
	palloc().) */
	if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread)
	{
		ASSERT(prev != cur);
		palloc_free_page(prev);
	}
}

/* Schedules a new process.  At entry, interrupts must be off and
the running process's state must have been changed from
running to some other state.  This function finds another
thread to run and switches to it.

It's not safe to call printf() until thread_schedule_tail()
has completed. */
static void
schedule(void)
{
	struct thread *cur = running_thread();

	/* added by jy for MFQ
	next_thread_to_run 이 결국 다음에 실행될 쓰레드를 가져오는 함수이다.
	이 함수에서 기존의 Round Robin 기법으로 다음 실행될 쓰레드를 가져오고 있다.
	MFQ 를 위해서 5개의 queue list 가 사용되기 때문에 이부분에서도 priority 에 따라
	쓰레드를 어떻게 가져올지에 관한 로직이 필요하다.
	*/
	struct thread *next = next_thread_to_run();
	struct thread *prev = NULL;

	ASSERT(intr_get_level() == INTR_OFF);
	ASSERT(cur->status != THREAD_RUNNING);
	ASSERT(is_thread(next));

	if (cur != next)
		prev = switch_threads(cur, next);
	thread_schedule_tail(prev);
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid(void)
{
	static tid_t next_tid = 1;
	tid_t tid;

	lock_acquire(&tid_lock);
	tid = next_tid++;
	lock_release(&tid_lock);

	return tid;
}

/* Offset of `stack' member within `struct thread'.
Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof(struct thread, stack);

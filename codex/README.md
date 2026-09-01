*This project has been created as part of the 42 curriculum by chabourk.*

# Codexion

## Description

Codexion is a multithreaded C simulation inspired by the Dining Philosophers
problem, reframed around a coding theme. A configurable number of **coders** sit
in a circular co-working hub, sharing a limited pool of **USB dongles** placed on
a central table (one dongle between each pair of neighboring coders). Each coder
repeatedly cycles through three phases:

- **compiling** -- requires holding two dongles simultaneously (left and right)
- **debugging** -- a fixed-duration phase requiring no resources
- **refactoring** -- a fixed-duration phase requiring no resources, after which
  the coder immediately attempts to compile again

If a coder does not start compiling within `time_to_burnout` milliseconds of the
start of its last compile (or the start of the simulation), it **burns out** and
the simulation stops immediately. Otherwise, the simulation stops once every
coder has compiled at least `number_of_compiles_required` times.

The goal is to correctly implement resource sharing, fair scheduling, and precise
timing across concurrent threads using POSIX threads, mutexes, and condition
variables -- while avoiding deadlocks, data races, and starvation.

## Instructions

### Compilation

```bash
make
```

Compiles with `-Wall -Wextra -Werror -pthread` and produces a binary named
`codexion`.

```bash
make clean    # remove object files
make fclean   # remove object files and binary
make re       # fclean + rebuild
```

### Execution

```bash
./codexion <coders> <burnout> <compile> <debug> <refactor> <required_compiles> <cooldown> <fifo|edf>
```

| Argument            | Unit     | Description                                     |
| ------------------- | -------- | ----------------------------------------------- |
| coders              | count    | Number of coder threads                         |
| burnout             | ms       | Max time without compiling before burnout       |
| compile             | ms       | Duration of the compiling phase                 |
| debug               | ms       | Duration of the debugging phase                 |
| refactor            | ms       | Duration of the refactoring phase               |
| required_compiles   | count    | Number of compiles needed to stop normally      |
| cooldown            | ms       | Minimum time before a released dongle can be reused |
| fifo \| edf         | mode     | Scheduler: FIFO or Earliest Deadline First     |

**Examples:**

```bash
./codexion 5 5000 200 200 200 3 200 fifo
./codexion 3 10000 300 300 300 5 500 edf
```

## Blocking Cases Handled

### Deadlock Prevention (Coffman's Conditions)

The Dining Philosophers problem is prone to deadlock when all coders pick up
their left dongle simultaneously, then wait forever for the right. We break the
**circular wait** condition using a **total ordering** over dongles via a
per-dongle priority queue. Each waiting coder registers in both queues (left and
right) before blocking. The queue grants the dongle to the highest-priority
waiting coder, ensuring a strict acquisition order. The last coder in the ring
acquires in reverse order, breaking the circular dependency entirely.

### Starvation Prevention

Each dongle maintains a FIFO (or EDF) wait queue. Under FIFO scheduling, the
longest-waiting coder is always served first, guaranteeing no coder is
permanently bypassed. Under EDF, the coder with the earliest burnout deadline
is prioritized, preventing burnout-induced starvation.

### Cooldown Handling

When a dongle is released, its `available_at` timestamp is set to
`now + cooldown`. Any coder attempting to acquire it before that time blocks via
`pthread_cond_timedwait`, which automatically re-acquires the mutex and retries
when the deadline expires. This prevents immediate re-use and models realistic
hardware access patterns.

### Precise Burnout Detection

A dedicated monitor thread polls every 500 microseconds, checking each coder's
`last_compile_start` against the burnout threshold. When burnout is detected, the
shared `running` flag is set to `0` **before** the burnout message is logged,
ensuring the "burned out" line is always the last output.

### Log Serialization

All output (both coder state changes and the burnout message) is serialized
through a single `log_mutex`. This guarantees that log lines are never
interleaved, regardless of how many threads print concurrently.

## Thread Synchronization Mechanisms

### Mutexes

| Mutex            | Protects                                          |
| ---------------- | ------------------------------------------------- |
| `state_mutex`    | `running` flag, `last_compile_start`, `compiles_done` per coder |
| `dongle->mutex`  | `available`, `available_at`, `waiters` queue per dongle |
| `log_mutex`      | All `printf` calls for serialized output          |

### Condition Variables

| Condvar          | Purpose                                           |
| ---------------- | ------------------------------------------------- |
| `dongle->cond`   | Wakes waiters when a dongle is released or cooldown expires |

### How They Coordinate

1. **Coders vs dongles:** A coder locks the dongle mutex, pushes itself into the
   wait queue, then blocks on `pthread_cond_wait` or `pthread_cond_timedwait`.
   When another coder releases the dongle, it broadcasts the condvar. The woken
   coder rechecks availability and queue position before proceeding.

2. **Coders vs monitor:** The monitor reads `last_compile_start` and
   `compiles_done` under `state_mutex`. Coder threads update these fields under
   the same mutex. The shared `running` flag is the only communication channel
   between the monitor and coders -- no direct signaling is needed.

3. **Race condition prevention:** All shared mutable state is protected by one of
   the three mutexes above. Helgrind verification confirms zero data races.

## Resources

- [Dining Philosophers Problem -- Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Coffman Conditions -- Wikipedia](https://en.wikipedia.org/wiki/Coffman_conditions)
- [POSIX Threads Programming -- LLVM](https://computing.llnl.gov/tutorials/pthreads/)
- [pthread(7) -- Linux man page](https://man7.org/linux/man-pages/man7/pthreads.7.html)

### AI Usage

AI (opencode / Claude) was used for:
- Architecture design and walkthrough of concurrency concepts
- Step-by-step explanation of each function before implementation
- Debugging coordination logic and verifying subject compliance
- Writing this README

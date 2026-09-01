#ifndef CODEX_H
# define CODEX_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>

# define FIFO 1
# define EDF 2

typedef struct s_systeme t_systeme;
typedef struct s_coder t_coder;
typedef struct s_dongle t_dongle;

typedef struct s_request
{
    t_coder *coder;
    long priority;
}   t_request;

typedef struct s_queue
{
    t_request requests[2];
    int size;
}   t_queue;

typedef struct s_params
{
    int nb_coders;
    long time_to_burnout;
    long time_to_compile;
    long time_to_debug;
    long time_to_refactor;
    int nb_required_compiles;
    long dongle_cooldown;
    int scheduler;
} t_params;

struct s_systeme
{
    pthread_t monitor;
    t_params params;
    long start_time;
    pthread_mutex_t state_mutex;
    int running;
    pthread_mutex_t log_mutex;
    t_coder *coders;
    t_dongle *dongles;
};

typedef struct s_dongle
{
    int available;
    long available_at;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    t_queue waiters;
}   t_dongle;

typedef struct s_coder
{
    int id;
    t_dongle *left;
    t_dongle *right;
    t_systeme *sim;
    pthread_t thread;
    long last_compile_start;
    long compiles_done;
}   t_coder;

int     parse_arguments(int argc, char **argv, t_params *params);

/*init.c*/
int     init_system(t_systeme *sim, t_params *params);

/*cleanup.c */
void    cleanup_system(t_systeme *sim, int nb_dongles);
int     error(char *msg);

/*routine.c*/
void    *coder_routine(void *arg);

/*threads.c */
int     create_threads(t_systeme *sim);
void    join_threads(t_systeme *sim);

/*monitor.c*/
void    *monitor(void *arg);

/*dongle.c*/
void    release_dongle(t_dongle *dongle, t_systeme *sim);

/* scheduler.c */
long    get_priority(t_coder *coder);
void    queue_push(t_queue *q, t_coder *coder, long priority);
void    queue_pop(t_queue *q);
t_coder *queue_front(t_queue *q);

/*logger.c*/
int     log_action(t_coder *coder, char *msg);

/*utils.c*/
long    current_time_ms(void);
void    msleep(t_systeme *sim, long time);
int     simulation_running(t_systeme *sim);
int     coder_done(t_coder *coder);
void    sleep_odd_coders(t_coder *coder);

#endif

#include "codex.h"


static void	get_condtime(struct timespec *t_s, long long time)
{
	t_s->tv_sec = time / 1000;
	t_s->tv_nsec = (time % 1000) * 1000000;
}

static int wait_to_take_dongle(t_dongle* dongle, t_coder* coder)
{
    struct timespec c_wait;
    
    while (!dongle->available || !(queue_front(&dongle->waiters) == coder->id) || current_time_ms() < dongle->available_at)
    {
        if (!dongle->available || !(queue_front(&dongle->waiters) == coder->id))
            pthread_cond_wait(&dongle->cond, &dongle->mutex);
        if (!simulation_running(coder->sim))
            return (1);
        if (current_time_ms() < dongle->available_at)
        {
            get_condtime(&c_wait, dongle->available_at);
            pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &c_wait);
        }
        if (!simulation_running(coder->sim))
            return (1);
    }
    return (0);
}

static int	take_dongles(t_coder *coder)
{
	long	priority;

	priority = get_priority(coder);
	pthread_mutex_lock(&coder->left->mutex);
	queue_push(&coder->left->waiters, coder, priority);
	pthread_mutex_unlock(&coder->left->mutex);
	priority = get_priority(coder);
	pthread_mutex_lock(&coder->right->mutex);
	queue_push(&coder->right->waiters, coder, priority);
	pthread_mutex_unlock(&coder->right->mutex);
	if (coder->left == coder->right)
	{
		while (simulation_running(coder->sim))
			usleep(500);
		return (1);
	}

    pthread_mutex_lock(&coder->left->mutex);
    if (wait_to_take_dongle(coder->left, coder))
    {
        pthread_mutex_unlock(&coder->left->mutex);
        return (1);
    }
    coder->left->available = 0;
    queue_pop(&coder->left->waiters);
    pthread_mutex_unlock(&coder->left->mutex);

    pthread_mutex_lock(&coder->right->mutex);
    if (wait_to_take_dongle(coder->right, coder))
    {
        pthread_mutex_unlock(&coder->right->mutex);
        return (1);
    }
    coder->right->available = 0;
    queue_pop(&coder->right->waiters);
    pthread_mutex_unlock(&coder->right->mutex);
    if (!log_action(coder, "has taken a dongle"))
        return (1);
    if (!log_action(coder, "has taken a dongle"))
        return (1);
    return (0);
}

static void	release_dongles(t_coder *coder)
{
	release_dongle(coder->left, coder->sim);
	release_dongle(coder->right, coder->sim);
}

static void compile(t_coder *coder)
{
    pthread_mutex_lock(&coder->sim->state_mutex);
    coder->last_compile_start = current_time_ms();
    pthread_mutex_unlock(&coder->sim->state_mutex);
    log_action(coder, "is compiling");
    msleep(coder->sim, coder->sim->params.time_to_compile);
    pthread_mutex_lock(&coder->sim->state_mutex);
    coder->compiles_done++;
    pthread_mutex_unlock(&coder->sim->state_mutex);
}

void	sleep_odd_coders(t_coder *coder)
{
	if (coder->id % 2 != 0)
		msleep(coder->sim, 100);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	sleep_odd_coders(coder);
	while (simulation_running(coder->sim) && !coder_done(coder))
	{
		if (take_dongles(coder))
			break ;
		if (!simulation_running(coder->sim))
		{
			release_dongles(coder);
			break ;
		}
		compile(coder);
		release_dongles(coder);
		if (!simulation_running(coder->sim))
			break ;
		log_action(coder, "is debugging");
		msleep(coder->sim, coder->sim->params.time_to_debug);
		if (!simulation_running(coder->sim))
			break ;
		log_action(coder, "is refactoring");
		msleep(coder->sim, coder->sim->params.time_to_refactor);
	}
	return (NULL);
}




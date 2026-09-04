#include "codex.h"

static int	is_available_for(t_dongle *dongle, t_coder *coder)
{
	return (dongle->available
		&& current_time_ms() >= dongle->available_at
		&& queue_front(&dongle->waiters) == coder);
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
	while (simulation_running(coder->sim))
	{
		pthread_mutex_lock(&coder->left->mutex);
		if (is_available_for(coder->left, coder))
		{
			pthread_mutex_lock(&coder->right->mutex);
			if (is_available_for(coder->right, coder))
			{
				coder->left->available = 0;
				coder->right->available = 0;
				queue_pop(&coder->left->waiters);
				queue_pop(&coder->right->waiters);
				pthread_mutex_unlock(&coder->right->mutex);
				pthread_mutex_unlock(&coder->left->mutex);
				if (!log_action(coder, "has taken a dongle"))
					return (1);
				if (!log_action(coder, "has taken a dongle"))
					return (1);
				return (0);
			}
			pthread_mutex_unlock(&coder->right->mutex);
		}
		pthread_mutex_unlock(&coder->left->mutex);
		usleep(500);
	}
	return (1);
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




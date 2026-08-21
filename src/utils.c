#include "codexion.h"

long	current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int	simulation_running(t_systeme *sim)
{
	int	running;

	pthread_mutex_lock(&sim->state_mutex);
	running = sim->running;
	pthread_mutex_unlock(&sim->state_mutex);
	return (running);
}

int	coder_done(t_coder *coder)
{
	int	done;

	pthread_mutex_lock(&coder->sim->state_mutex);
	done = coder->compiles_done >= coder->sim->params.nb_required_compiles;
	pthread_mutex_unlock(&coder->sim->state_mutex);
	return (done);
}

void	msleep(t_systeme *sim, long ms)
{
	long	start;

	start = current_time_ms();
	while (simulation_running(sim))
	{
		if (current_time_ms() - start >= ms)
			break ;
		usleep(500);
	}
}

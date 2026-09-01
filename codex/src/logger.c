#include "codex.h"

int	log_action(t_coder *coder, char *msg)
{
	long	time;

	pthread_mutex_lock(&coder->sim->log_mutex);
	if (!simulation_running(coder->sim))
	{
		pthread_mutex_unlock(&coder->sim->log_mutex);
		return (0);
	}
	time = current_time_ms() - coder->sim->start_time;
	printf("%ld %d %s\n", time, coder->id, msg);
	pthread_mutex_unlock(&coder->sim->log_mutex);
	return (1);
}

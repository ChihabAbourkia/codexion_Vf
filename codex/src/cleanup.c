#include "codex.h"

int	error(char *msg)
{
	printf("%s\n", msg);
	return (1);
}

static void	destroy_dongles(t_systeme *sys, int nb_dongles)
{
	int	i;

	i = 0;
	while (i < nb_dongles)
	{
		pthread_cond_destroy(&sys->dongles[i].cond);
		pthread_mutex_destroy(&sys->dongles[i].mutex);
		i++;
	}
}

void	cleanup_system(t_systeme *sim, int nb_dongles)
{
	if (sim->coders)
	{
		free(sim->coders);
		sim->coders = NULL;
	}
	if (sim->dongles)
	{
		destroy_dongles(sim, nb_dongles);
		free(sim->dongles);
		sim->dongles = NULL;
	}
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->state_mutex);
}

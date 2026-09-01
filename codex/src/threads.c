#include "codex.h"

int	create_threads(t_systeme *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]))
		{
			while (--i >= 0)
				pthread_join(sim->coders[i].thread, NULL);
			return (error("Failed to create thread"));
		}
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor, sim))
	{
		join_threads(sim);
		return (error("Failed to create monitor thread"));
	}
	return (0);
}

void	join_threads(t_systeme *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.nb_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	if (sim->monitor)
		pthread_join(sim->monitor, NULL);
}

#include "codex.h"

static int	init_dongles(t_systeme *sys)
{
	int	i;

	i = 0;
	while (i < sys->params.nb_coders)
	{
		if (pthread_mutex_init(&sys->dongles[i].mutex, NULL) != 0)
		{
			cleanup_system(sys, i);
			return (error("Failed to init dongle mutex"));
		}
		if (pthread_cond_init(&sys->dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&sys->dongles[i].mutex);
			cleanup_system(sys, i);
			return (error("Failed to init dongle cond"));
		}
		sys->dongles[i].available = 1;
		sys->dongles[i].available_at = sys->start_time;
		i++;
	}
	return (0);
}

static int	init_coders(t_systeme *sys)
{
	int	i;

	i = 0;
	while (i < sys->params.nb_coders)
	{
		sys->coders[i].id = i + 1;
		sys->coders[i].last_compile_start = sys->start_time;
		sys->coders[i].compiles_done = 0;
		sys->coders[i].sim = sys;
		sys->coders[i].left = &sys->dongles[i];
		sys->coders[i].right = &sys->dongles[(i + 1) % sys->params.nb_coders];
		if (i == (sys->params.nb_coders - 1))
		{
			sys->coders[i].left = &sys->dongles[(i + 1)
				% sys->params.nb_coders];
			sys->coders[i].right = &sys->dongles[i];
		}
		i++;
	}
	return (0);
}

int	init_system(t_systeme *sys, t_params *params)
{
	sys->params = *params;
	sys->running = 1;
	sys->start_time = current_time_ms();
	if (pthread_mutex_init(&sys->state_mutex, NULL) != 0)
		return (error("Failed to init state mutex"));
	if (pthread_mutex_init(&sys->log_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sys->state_mutex);
		return (error("Failed to init log mutex"));
	}
	sys->coders = malloc(sizeof(t_coder) * sys->params.nb_coders);
	if (!sys->coders)
		return (cleanup_system(sys, 0), error("Failed to allocate coders"));
	sys->dongles = malloc(sizeof(t_dongle) * sys->params.nb_coders);
	if (!sys->dongles)
		return (cleanup_system(sys, 0), error("Failed to allocate dongles"));
	memset(sys->coders, 0, sizeof(t_coder) * sys->params.nb_coders);
	memset(sys->dongles, 0, sizeof(t_dongle) * sys->params.nb_coders);
	if (init_dongles(sys))
		return (1);
	if (init_coders(sys))
		return (cleanup_system(sys, sys->params.nb_coders), 1);
	return (0);
}

#include "codex.h"

void	release_dongle(t_dongle *dongle, t_systeme *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = 1;
	dongle->available_at = current_time_ms()
		+ sim->params.dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

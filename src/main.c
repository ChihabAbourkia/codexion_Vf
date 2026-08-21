
#include "codexion.h"
int	main(int argc, char **argv)
{
	t_params	params;
	t_systeme	sys;

	if (parse_arguments(argc, argv, &params))
		return (1);
	if (init_system(&sys, &params))
		return (1);
	return (0);
}
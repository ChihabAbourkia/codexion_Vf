#include "codex.h"

static int	ft_isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

static long	ft_atol(const char *str)
{
	long	result;
	int		i;

	if (!str || !*str)
		return (-1);
	if (str[0] == '0' && str[1] != '\0')
		return (-1);
	if (str[0] == '+')
		str++;
	else if (str[0] == '-')
		return (-1);
	result = 0;
	i = 0;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (-1);
		if (result > (LONG_MAX - (str[i] - '0')) / 10)
			return (-1);
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result);
}

static int	parse_numbers(char **av, t_params *params)
{
	long	v[7];
	int		i;

	i = 0;
	while (i < 7)
	{
		v[i] = ft_atol(av[i + 1]);
		if (v[i] < 0 || v[i] > INT_MAX)
			return (error("Invalid numeric argument"));
		i++;
	}
	params->nb_coders = (int)v[0];
	params->time_to_burnout = v[1];
	params->time_to_compile = v[2];
	params->time_to_debug = v[3];
	params->time_to_refactor = v[4];
	params->nb_required_compiles = (int)v[5];
	params->dongle_cooldown = v[6];
	return (0);
}

static int	validate_config(t_params *params)
{
	if (params->nb_coders < 1 || params->nb_coders > 1000)
		return (error("Invalid number of coders"));
	if (params->time_to_burnout < 1 || params->time_to_compile < 1)
		return (error("Invalid timing"));
	if (params->time_to_debug < 1 || params->time_to_refactor < 1)
		return (error("Invalid timing"));
	if (params->nb_required_compiles < 1)
		return (error("Invalid required compiles"));
	return (0);
}

static int	parse_scheduler(char *arg, t_params *params)
{
	if (!strcmp(arg, "fifo"))
		params->scheduler = FIFO;
	else if (!strcmp(arg, "edf"))
		params->scheduler = EDF;
	else
		return (error("Invalid scheduler"));
	return (0);
}

int	parse_arguments(int ac, char **av, t_params *params)
{
	*params = (t_params){0};
	if (ac != 9)
		return (error("Usage: ./codexion <coders> <burnout> <compile> "
				"<debug> <refactor> <required_compiles> "
				"<cooldown> <fifo|edf>"));
	if (parse_numbers(av, params))
		return (1);
	if (parse_scheduler(av[8], params))
		return (1);
	return (validate_config(params));
}


#include "codex.h"

int main(int argc, char **argv)
{
    t_params    params;
    t_systeme   sys;

    if (parse_arguments(argc, argv, &params))
        return (1);
    if (init_system(&sys, &params))
        return (1);
    if (create_threads(&sys))
    {
        cleanup_system(&sys, sys.params.nb_coders);
        return (1);
    }
    join_threads(&sys);
    cleanup_system(&sys, sys.params.nb_coders);
    return (0);
}
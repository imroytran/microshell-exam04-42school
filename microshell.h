#ifndef _MICROSHELL_H
#define _MICROSHELL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct s_list
{
	void *content;
	struct s_list	*next;
} t_list;

#endif

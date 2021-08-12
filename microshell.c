#include "microshell.h"

size_t	ft_strlen(char const *str)
{
	char const	*pstr;
	
	pstr = str;
	while (*pstr)
		pstr++;
	return (pstr - str);
}

char *ft_strdup(const char *str)
{
	char	*temp;
	size_t	i;
	
	if (!(temp = malloc(sizeof(char) * ft_strlen(str))))
		return (NULL);
	i = 0;
	while (str[i])
	{
		temp[i] = str[i];
		i++;
	}
	temp[i] = '\0';
	return (temp);
}

t_list	*ft_lstnew(void *content)
{
	t_list	*temp;
	
	if (!(temp = malloc(sizeof(t_list))))
		return (NULL);
	temp->content = content;
	temp->next = NULL;
	return (temp);
}

void ft_lstadd_back(t_list **lst, t_list *new)
{
	t_list	*temp;
	
	if (!new || !lst)
		return ;
	if (!*lst)
	{
		*lst = new;
		return ;
	}
	temp = *lst;
	while (temp->next)
		temp = temp->next;
	temp->next = new;
}

void del(char **content)
{
	int i;
	
	if (!*content || !content)
		return ;
	i = 0;
	while (content[i])
	{
		free(content[i]);
		i++;
	}
	free(content);
}

void ft_lstclear(t_list **lst)
{
	t_list	*temp;
	t_list	*next;
	
	temp = *lst;
	if (!temp || !lst)
		return ;
	while (temp)
	{
		next = temp->next;
		del(temp->content);
		free(temp);
		temp = next;
	}
	*lst = NULL;
}
void ft_putstr_fd(char *str, int fd)
{
	if (!str)
		return ;
	write(fd, str, ft_strlen(str));
}

void *sys_err(void)
{
	ft_putstr_fd("error: fatal\n", 2);
	return (NULL);
}

int execve_err(char *cmd)
{
	ft_putstr_fd("error: cannot execute ", 2);
	ft_putstr_fd(cmd, 2);
	ft_putstr_fd("\n", 2);
	return (0);
}

int cd_err_arg(void)
{
	ft_putstr_fd("error: cd: bad arguments\n", 2);
	return (0);
}

int cd_err(char *dir)
{
	ft_putstr_fd("error: cd: cannot change directory to ", 2);
	ft_putstr_fd(dir, 2);
	ft_putstr_fd("\n", 2);
	return (0);
}

char **add_cmd(char **cmd, char *arg, int count)
{
	char **temp;
	int 	i;
	if (!(temp = malloc(sizeof(char *) * (count + 2))))
		return (sys_err());
	i = 0;
	while (cmd && cmd[i])
	{
		temp[i] = ft_strdup(cmd[i]);
		free(cmd[i]);
		i++;
	}
	if (cmd)
		free(cmd);
	temp[i] = ft_strdup(arg);
	temp[i + 1] = NULL;
	return (temp);
}

void set_fd(int fdin, int fdout)
{
	if (fdin != 0)
	{
		dup2(fdin, 0);
		close(fdin);
	}
	if (fdout != 1)
	{
		dup2(fdout, 1);
		close(fdout);
	}
}

int exec(int fdin, int fdout, char **cmd, char *env[])
{
	pid_t f;
	int ret;
	int status;
	
	f = fork();
	if (f < 0)
	{
		sys_err();
		return (0);
	}
	else if (f == 0)
	{
		set_fd(fdin, fdout);
		if ((ret = execve(cmd[0], cmd, env)) == -1)
			return (execve_err(cmd[0]));
	}
	else
	{
		waitpid(f, &status, 0);
		ret = WIFEXITED(status);
	}
	return (ret);
}

int do_cd(char **cmd)
{
	if (cmd[2] != NULL || cmd[1] == NULL)
		return (cd_err_arg());
	if (chdir(cmd[1]) == -1)
		return (cd_err(cmd[1]));
	return (1);
}

int do_cmds(t_list *cmds, char *env[])
{
	char	**cmd;
	t_list	*temp;
	int	fd[2];
	int fdpipe[2];
	int ret;
	
	if (!cmds)
		return (0);
	fd[0] = 0;
	fd[1] = 1;
	temp = cmds;
	while (temp->next)
	{
		cmd = temp->content;
		pipe(fdpipe);
		ret = exec(fd[0], fdpipe[1], cmd, env);
		if (ret == 0)
			return (0);
		close(fdpipe[1]);
		if (fd[0] != 0)
			close(fd[0]);
		fd[0] = fdpipe[0];
		temp = temp->next;
	}
	cmd = temp->content;
	if (cmd[0] && !strcmp(cmd[0], "cd"))
		return (do_cd(cmd));
	ret = exec(fd[0], fd[1], cmd, env);
	return (ret);
}

int do_exec(int argc, char **argv, char *env[])
{
	int i;
	t_list	*cmds;
	char	**cmd;
	int 	count;
	int	ret;
	
	i = 1;
	while (i <= argc)
	{
		cmds = NULL;
		while (i < argc && strcmp(argv[i], ";") != 0)
		{
			count = 0;
			cmd = NULL;
			while (i < argc && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
			{
				if (!(cmd = add_cmd(cmd, argv[i], count)))
					return (0);
				count++;
				i++;
			}
			ft_lstadd_back(&cmds, ft_lstnew(cmd));
			if (argv[i] && !strcmp(argv[i], "|"))
				i++;
		}
		ret = do_cmds(cmds, env);
		ft_lstclear(&cmds);
		i++;
	}
	return (ret);
}

int main(int argc, char **argv, char *env[])
{
	int ret;
	if (argc == 1)
		return (0);
	ret = do_exec(argc, argv, env);
	return (ret);
}


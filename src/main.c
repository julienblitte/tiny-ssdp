#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include "ssdp.h"
#include "logfilter.h"

#define PID_FILE	"/var/run/ssdpd.pid"

#define SIGNAL_CATCH	{SIGHUP,SIGTERM}

void sig_handler(int signo)
{
	switch(signo)
	{
	case SIGHUP: /* not implemented */
		break;
	case SIGTERM:
		remove(PID_FILE);
		exit(EXIT_SUCCESS);
		break;
	}
}

void daemonize() 
{
	FILE *run;
	int i, sig[] = SIGNAL_CATCH;

	/* fork, detach, reset stdios */
	if (daemon(0, 0) == -1)
	{
		perror("Fatal error unable to become deamon!");
		exit(EXIT_FAILURE);
	}
	/* pid file */
	if(run = fopen(PID_FILE, "wt"))
	{
		fprintf(run, "%d", getpid());
		fclose(run);
	}
	/* signals */
	for (i=0; i < sizeof(sig)/sizeof(int); i++)
	{
		if (signal(i[sig], sig_handler) == SIG_ERR)
		{
			perror("Error registering signal handler\n");
		}
	}
}
 
int main(int argc, char *argv[])
{
	pthread_t td_server, td_notify;
	void *ret;

	if(geteuid() != 0)
	{
		perror("You must be root to run this program!\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 2 && strcmp(argv[1], "--debug") == 0)
	{
		openlog(SYSLOG_IDENT, LOG_PERROR, LOG_DAEMON);
	}
	else
	{
		daemonize();
		syslog_filter = LOG_NOTICE;
		openlog(SYSLOG_IDENT, 0, LOG_DAEMON);
	}

	ssdp_init();
	ssdp_notify();

	if (pthread_create(&td_server, NULL, ssdp_thread_server, NULL) < 0)
	{
		writelog(LOG_ERR, "Fatal error unable to create ssdp_server thread!");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&td_notify, NULL, ssdp_thread_notify, NULL) < 0)
	{
		writelog(LOG_ERR, "Fatal error unable to create ssdp_notify thread!");
		exit(EXIT_FAILURE);
	}

	(void)pthread_join (td_server, &ret);
	(void)pthread_join (td_notify, &ret);

	return EXIT_SUCCESS;
}


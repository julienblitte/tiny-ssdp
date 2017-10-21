/* tiny-ssdp is an SSDP client announcement daemon
 *
 * Copyright (C) 2017  Julien Blitte <julien.blitte@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "ssdp.h"
#include "logfilter.h"

#define PID_FILE	"/var/run/tiny-ssdpd.pid"

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
	else if (argc == 2 && strcmp(argv[1], "--no-daemon") == 0)
	{
		syslog_filter = LOG_NOTICE;
		openlog(SYSLOG_IDENT, 0, LOG_DAEMON);
		writelog(LOG_NOTICE, "Started as angel");
	}
	else
	{
		daemonize();
		syslog_filter = LOG_NOTICE;
		openlog(SYSLOG_IDENT, 0, LOG_DAEMON);
		writelog(LOG_NOTICE, "Started as deamon");
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


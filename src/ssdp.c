#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#include "network.h"
#include "http.h"
#include "ssdp.h"
#include "config.h"
#include "logfilter.h"

const char *ip;
int socket_descriptor = -1;

static pthread_mutex_t ssdp_mutex;

text ssdp_get_st_match_list()
{
	text result;
	static char st_list[1024];
	static char service[128];
	int i;

	snprintf(st_list, sizeof(st_list), "ssdp:all\nuuid:%s\nupnp:rootdevice\nurn:schemas-upnp-org:device:%s:%d",
		configuration.device_uuid, configuration.device_type_name, configuration.device_type_version);

	for(i=0; i < configuration.service_count; i++)
	{
		if (configuration.service_name[i])
		{
			snprintf(service, sizeof(service), "\nurn:schemas-upnp-org:service:%s:%d",
					configuration.service_name[i], configuration.service_version[i]);
			strncat(st_list, service, sizeof(st_list));
		}
	}

	result = data2text(st_list);
	return result;
}

// https://embeddedinn.wordpress.com/tutorials/upnp-device-architecture/
// http://buildingskb.schneider-electric.com/view.php?AID=15197
const char *ssdp_get_notify(int type)
{
	static char notify[SSDP_MAX_PACKETSIZE];
	char nt[256];
	char usn[256];
	int service;

	switch(type)
	{
		case 0:
			snprintf(nt, sizeof(nt), "uuid:%s", configuration.device_uuid);
			snprintf(usn, sizeof(usn), "uuid:%s", configuration.device_uuid);
			break;
		case 1:
			strncpy(nt, "upnp:rootdevice", sizeof(nt));
			*usn = '\0';
			break;
		case 2:
			snprintf(nt, sizeof(nt), "urn:schemas-upnp-org:device:%s:%d",
				configuration.device_type_name, configuration.device_type_version);
			*usn = '\0';
			break;
		default:
			service = type - 3;
			// if valid service in list
			if (service < configuration.service_count && configuration.service_name[service])
			{
				snprintf(nt, sizeof(nt), "urn:schemas-upnp-org:service:%s:%d",
					configuration.service_name[service], configuration.service_version[service]);
				*usn = '\0';
				break;
			}
			else
			{
				// end of service, return NULL
				return NULL;
			}
	}

	// no USN, build it from NT
	if (strcmp(usn, "") == 0)
	{
		snprintf(usn, sizeof(usn), "uuid:%s::%s", configuration.device_uuid, nt);
	}

	snprintf(notify, sizeof(notify), "NOTIFY * HTTP/1.1\r\nHost: %s:%d\r\nServer: %s\r\nCache-Control: max-age=%d\r\n"
		"Location: http://%s:%d%s\r\nNT: %s\r\nNTS: %s\r\nUSN: %s\r\n\r\n",
		configuration.network_multicast, configuration.network_port, configuration.device_description, configuration.ssdp_allowed_cache,
		ip, configuration.base_url_port, configuration.base_url_resource, nt, SSDP_NOTIFY_NTS, usn);

	return notify;
}

void ssdp_notify()
{
	const char *notify;
	int type;

	// not initialized, try to initialize
	if (socket_descriptor <= 0)
	{
		ssdp_init();
		if (socket_descriptor <= 0)
		{
			writelog(LOG_ERR, "Fatal error while trying to initialize ssdp!");
			return;
		}
	}
	
	pthread_mutex_lock(&ssdp_mutex);
	type=0;
	notify = ssdp_get_notify(type);
	while(notify)
	{
		writelog(LOG_INFO, "Sending NOTIFY type %d", type);
		multicast_send(socket_descriptor, configuration.network_multicast, configuration.network_port, notify, strlen(notify));

		type++;
		notify = ssdp_get_notify(type);
	}
	pthread_mutex_unlock(&ssdp_mutex);
}

const char *ssdp_get_response(const char *st)
{
	static char response[SSDP_MAX_PACKETSIZE];
	time_t rawtime;
	struct tm * timeinfo;
	char now[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(now, sizeof(now), "%a, %d %b %Y %X %Z", timeinfo);

	snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nServer: %s\r\nDate: %s\r\nLocation: http://%s:%d%s\r\n"
	"ST: %s\r\nEXT: %s\r\nUSN: uuid:%s::upnp:rootdevice\r\nCache-Control: max-age=%d\r\n\r\n",
		configuration.device_description, now, ip, configuration.base_url_port, configuration.base_url_resource, st, 
		SSDP_EXT, configuration.device_uuid, configuration.ssdp_allowed_cache);

	return response;
}

void ssdp_wait_random(int s)
{
	if (!s) return;

	srand(time(NULL));
	sleep(rand() % s);
}


void ssdp_handle_request(text request, struct sockaddr_in *client_addr, socklen_t client_size)
{
	text st_match_list;
	char *st, *man, *mx;
	const char *reply;
	struct sockaddr_in local_socket;

	pthread_mutex_lock(&ssdp_mutex);
	writelog(LOG_DEBUG, "Recieved:");
	writelog_text(LOG_DEBUG, request);

	st_match_list = ssdp_get_st_match_list();

	if (http_type(request) == HTTP_TYPE_SSDP_SEARCH)
	{
		writelog(LOG_DEBUG, "Datagram type is M-SEARCH");
		st = (char *)http_search_variable(request, "ST");
		man = (char *)http_search_variable(request, "MAN");
		mx = (char *)http_search_variable(request, "MX");
		if (st && man)	
		{
			man = http_dup_clean_value(man);
			if (!strcasecmp(man, SSDP_MAN_DISCOVER))
			{
				writelog(LOG_DEBUG, "ST matching filters are:");
				writelog_text(LOG_DEBUG, st_match_list);

				st = http_dup_clean_value(st);
				if (str_find_line(st_match_list, st) >= 0)
				{
					ssdp_wait_random(atoi(mx));
					writelog(LOG_DEBUG, "Request match: %s", st);
					reply = ssdp_get_response(st);
					if (sendto(socket_descriptor, reply, strlen(reply), 0, (struct sockaddr*)client_addr, client_size) > 0)
					{
						writelog(LOG_INFO, "Request answered");
					}
					else
					{ 
						writelog(LOG_WARNING, "Error while answering request!");
					} 
				}
				free(st);
			}
			free(man);
		}
	}
	else if (http_type(request) == HTTP_TYPE_SSDP_NOTIFY)
	{
		writelog(LOG_DEBUG, "Datagram type is NOTIFY\n");
		// do nothing
	}
	pthread_mutex_unlock(&ssdp_mutex);
}

void ssdp_init()
{
	pthread_mutex_lock(&ssdp_mutex);

	// already initialized
	if (socket_descriptor > 0)
		return;

	writelog(LOG_NOTICE, "Loading configuration...");
	config_load();

	writelog(LOG_NOTICE, "Initializing network...");

	ip = iptos(get_ip_address());

	socket_descriptor = udp_listen(configuration.network_port);
	if (!socket_descriptor)
	{
		writelog(LOG_ERR, "Fatal error creating or binding datagram socket!");
		return;
	}

	if (!multicast_join_group(socket_descriptor, configuration.network_multicast, configuration.network_interface))
	{
		writelog(LOG_ERR, "Fatal error while joining multicast group!");
		writelog(LOG_ERR, "Do you have multicast-enabled network connectivity?");
		close(socket_descriptor);
		socket_descriptor = 0;
		return;
	}
	pthread_mutex_unlock(&ssdp_mutex);
}

void *ssdp_thread_server(void *arg)
{
	char buffer[SSDP_MAX_PACKETSIZE];

	struct sockaddr_in client_addr;
	socklen_t client_size;

	// not initialized, try to initialize
	if (socket_descriptor <= 0)
	{
		ssdp_init();
		if (socket_descriptor <= 0)
		{
			writelog(LOG_ERR, "Fatal error while trying to initialize ssdp!");
			pthread_exit(NULL);
			return;
		}
	}

	/* Read from the socket. */
	memset(buffer, 0, sizeof(buffer));
	client_size = sizeof(client_addr);
	while(recvfrom(socket_descriptor, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&client_addr, &client_size) >= 0)
	{
		ssdp_handle_request(data2text(buffer), &client_addr, client_size);

		memset(buffer, 0, sizeof(buffer));
		client_size = sizeof(client_addr);
	}

	writelog(LOG_ERR, "Fatal error while reading datagram message!");
	close(socket_descriptor);
	socket_descriptor = 0;
	pthread_exit(NULL);
}

void *ssdp_thread_notify(void *arg)
{
	int period = 90;

	if (arg)
	{
		period = *((int*)arg);
	}

	while(1)
	{
		ssdp_notify();
		sleep(period);
	}
}


/* tiny-ssdp: Configuration file parser and generator
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <syslog.h>
#include <stdbool.h>

#include "config.h"
#include "logfilter.h"

#define MAX_LINE 512

ssdp_param_set configuration;

int str2int(char *s, int *value);

const char *config_dummy_uuid()
{
	static char device_uuid[MAX_PARAM];

	if (device_uuid[0])
		return device_uuid;

	srand(time(NULL));
	snprintf(device_uuid, MAX_PARAM, "%08x-%08x-%08x-%08x", rand(), rand(), rand(), rand());

	return device_uuid;
}

void config_init()
{
	int i;
	char expand_set_default[MAX_PARAM];

	configuration.network_port = DEFAULT_INT_NETWORK_PORT;
	strncpy(configuration.network_multicast, DEFAULT_STR_NETWORK_MULTICAST, MAX_PARAM);
	strncpy(configuration.network_interface, DEFAULT_STR_NETWORK_INTERFACE, MAX_PARAM);
	strncpy(configuration.device_description, DEFAULT_STR_DEVICE_DESCRIPTION, MAX_PARAM);
	strncpy(configuration.device_type_name, DEFAULT_STR_DEVICE_TYPE_NAME, MAX_PARAM);
	configuration.device_type_version = DEFAULT_INT_DEVICE_TYPE_VERSION;

	strncpy(configuration.device_uuid, config_dummy_uuid(), MAX_PARAM);

	configuration.base_url_port = DEFAULT_INT_BASE_URL_PORT;
	strncpy(configuration.base_url_resource, DEFAULT_STR_BASE_URL_RESOURCE, MAX_PARAM);
	configuration.service_count = DEFAULT_INT_SERVICE_COUNT;
	for(i=0; i < configuration.service_count; i++)
	{
		snprintf(expand_set_default, sizeof(expand_set_default), DEFAULT_SET_STR_SERVICE_NAME, i);
		configuration.service_name[i] = strdup(expand_set_default); // no memory reserved by default

		snprintf(expand_set_default, sizeof(expand_set_default), DEFAULT_SET_INT_SERVICE_VERSION);
		str2int(expand_set_default, &configuration.service_version[i]);
	}
	configuration.ssdp_allowed_cache = DEFAULT_INT_SSDP_ALLOWED_CACHE;
}

char *strtrim(char *s)
{
	char *start, *end;

	if (!*s)
		return s;

	start = s;
	end = s;
	while(*(end+1)) end++;

	/* Trim and delimit right side */
	while ((isspace(*end)) && (end >= start)) end--;
	*(end+1) = '\0';

	/* Trim left side */
	while ((isspace(*start)) && (start < end)) start++;

	strcpy(s, start);
	return s;
}

char *strcomment(char *s)
{
	char *p;

	for(p = s; *p != '\0'; p++)
	{
		if (*p == '#')
		{
			*p = '\0';
			break;
		}
	}

	return s;
}

int str2int(char *s, int *value)
{
	char *start;
	for(start = s; *start; start++)
	{
		if (*start < '0' || *start > '9')
			return *value;
	}
	
	*value = atoi(s);
	return *value;
}

void config_create()
{
	FILE *f;
	int i;
	char expand_set_variable[MAX_PARAM];

	f = fopen (CONFIG_FILE, "wt");
	if (!f)
	{
		writelog(LOG_WARNING, "Cannot create configuration file '%s'", CONFIG_FILE);
		return;
	}

	fprintf(f, "%s=%d\n", VAR_INT_NETWORK_PORT, configuration.network_port); //int
	fprintf(f, "%s=%s\n", VAR_STR_NETWORK_MULTICAST, configuration.network_multicast);
	fprintf(f, "%s=%s\n", VAR_STR_NETWORK_INTERFACE, configuration.network_interface);

	fprintf(f, "\n");
	fprintf(f, "%s=%s\n", VAR_STR_DEVICE_DESCRIPTION, configuration.device_description);
	fprintf(f, "%s=%s\n", VAR_STR_DEVICE_TYPE_NAME, configuration.device_type_name);
	fprintf(f, "%s=%d\n", VAR_INT_DEVICE_TYPE_VERSION, configuration.device_type_version); //int
	fprintf(f, "%s=%s\n", VAR_STR_DEVICE_UUID, configuration.device_uuid); // random value

	fprintf(f, "\n");
	fprintf(f, "%s=%d\n", VAR_INT_BASE_URL_PORT, configuration.base_url_port); //int
	fprintf(f, "%s=%s\n", VAR_STR_BASE_URL_RESOURCE, configuration.base_url_resource);

	fprintf(f, "\n");
	fprintf(f, "%s=%d\n", VAR_INT_SERVICE_COUNT, configuration.service_count); //int
	for(i=0; i < configuration.service_count; i++)
	{
		snprintf(expand_set_variable, sizeof(expand_set_variable), VAR_SET_STR_SERVICE_NAME, i);
		fprintf(f, "%s=%s\n", expand_set_variable, configuration.service_name[i]);

		snprintf(expand_set_variable, sizeof(expand_set_variable), VAR_SET_INT_SERVICE_VERSION, i);
		fprintf(f, "%s=%d\n", expand_set_variable, configuration.service_version[i]);
	}
	fprintf(f, "\n");
	fprintf(f, "%s=%d\n", VAR_INT_SSDP_ALLOWED_CACHE, configuration.ssdp_allowed_cache); //int

	fprintf(f, "\n");
	fprintf(f, "%s=%d\n", VAR_INT_SYSLOG_LEVEL, syslog_filter); //int, global variable syslog_level

	/* values for web services part */
	fprintf(f, "\n");
	fprintf(f, "%s", VARS_WEBSERVICES);

	fclose(f);
}

void config_load()
{
	char *s, line[MAX_LINE];
	char name[MAX_PARAM], value[MAX_PARAM];
	FILE *f;
	int nline;
	char service_name[MAX_PARAM];
	bool handled;
	int i;

	config_init();

	f = fopen (CONFIG_FILE, "rt");
	if (!f)
	{
		writelog(LOG_NOTICE, "Configuration file '%s' not readable, creating default one...", CONFIG_FILE);
		config_create();
		return;
	}

	nline=0;

	while ((s = fgets(line, sizeof(line), f)) !=  NULL)
	{
		nline++;
		strcomment(s);

		s = strtok(line, "=");
		if (!s)
			continue;

		strncpy(name, s, sizeof(name));
		strtrim(name);

		if (strlen(name) == 0) /* empty line */
			continue;

		s = strtok(NULL, "=");
		if (!s)
		{
			writelog(LOG_WARNING, "Error at '%s':%d.", CONFIG_FILE, nline);
			continue;
		}

		strncpy(value, s, sizeof(value));
		strtrim(value);

		handled = false;

		if (!handled && strcasecmp(name, VAR_INT_NETWORK_PORT) == 0)
		{
			str2int(value, &configuration.network_port);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_STR_NETWORK_MULTICAST)==0)
		{
			strncpy(configuration.network_multicast, value, MAX_PARAM);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_STR_NETWORK_INTERFACE) == 0)
		{
			strncpy(configuration.network_interface, value, MAX_PARAM);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_STR_DEVICE_DESCRIPTION) == 0)
		{
			strncpy(configuration.device_description, value, MAX_PARAM);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_STR_DEVICE_TYPE_NAME) == 0)
		{
			strncpy(configuration.device_type_name, value, MAX_PARAM);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_INT_DEVICE_TYPE_VERSION) == 0)
		{
			str2int(value, &configuration.device_type_version);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_STR_DEVICE_UUID) == 0)
		{
			strncpy(configuration.device_uuid, value, MAX_PARAM);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_INT_BASE_URL_PORT) == 0)
		{
			str2int(value, &configuration.base_url_port);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_STR_BASE_URL_RESOURCE) == 0)
		{
			strncpy(configuration.base_url_resource, value, MAX_PARAM);
			handled = true;
		}
		if (!handled && strcasecmp(name, VAR_INT_SERVICE_COUNT) == 0)
		{
			str2int(value, &configuration.service_count);
			if (configuration.service_count > MAX_SERVICE)
				configuration.service_count = MAX_SERVICE;
			handled = true;
		}
		if (!handled)
		{
			for(i=0; i < MAX_SERVICE; i++)
			{
				snprintf(service_name, sizeof(service_name), VAR_SET_STR_SERVICE_NAME, i);
				if (strcasecmp(name, service_name) == 0)
				{
					// no space allocated by default to service_name
					configuration.service_name[i] = strdup(value);
					handled = true;
					break;
				}

				snprintf(service_name, sizeof(service_name), VAR_SET_INT_SERVICE_VERSION, i);
				if (strcasecmp(name, service_name) == 0)
				{
					str2int(value, &(configuration.service_version[i]));
					handled = true;
					break;
				}
			}
		}
		if (!handled && strcasecmp(name, VAR_INT_SSDP_ALLOWED_CACHE) == 0)
		{
			str2int(value, &configuration.ssdp_allowed_cache); //int
			handled = true;
		}

		if (!handled && strcasecmp(name, VAR_INT_SYSLOG_LEVEL) == 0)
		{
			str2int(value, &syslog_filter);	/* store in global syslog_level, not in configuration structure */
			handled = true;
		}
		if (!handled)
			writelog(LOG_DEBUG, "Ingored configuration statement at '%s':%d.", CONFIG_FILE, nline);
	}

	fclose (f);
}


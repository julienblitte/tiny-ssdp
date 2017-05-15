/* HTTP parser
 *
 * Copyright (C) 2017  Julien Blitte <julien.blitte@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "http.h"

const char *http_search_variable(text t, const char *variable)
{
	unsigned int l;
	char *pattern;
	size_t len;

	len = strlen(variable)+1;
	pattern = (char *)malloc(len+1);

	snprintf(pattern, len+1, "%s:", variable);

	for(l=0; t[l]; l++)
	{
		if (!strncasecmp(t[l], pattern, len))
		{
			free(pattern);
			return (const char *)(t[l] + len);
		}
	}

	free(pattern);
	return NULL;
}

char *http_dup_clean_value(const char *s)
{
	char *result;
	int len;

	/* remove leading space */
	while(*s == ' ')
		s++;
	
	len = strlen(s);
	if (len >= 2 && s[0] == '"' && s[len-1] == '"')
	{
		result = strdup(s+1);
		len -= 2;
		result[len] = '\0';
	}
	else
	{
		result = strdup(s);
	}
	
	/* remove trailing space */
	while(len > 0 && result[len-1] == ' ')
	{
		len--;
		result[len] = '\0';
	}

	return result;
}

int http_type(text t)
{
	if (!*t)
		return HTTP_TYPE_UNKNOWN;

	if (!strncasecmp(*t, "GET ", 4))
		return HTTP_TYPE_GET;
	if (!strncasecmp(*t, "POST ", 5))
		return HTTP_TYPE_POST;
	if (!strncasecmp(*t, "M-SEARCH ", 9))
		return HTTP_TYPE_SSDP_SEARCH;
	if (!strncasecmp(*t, "NOTIFY ", 7))
		return HTTP_TYPE_SSDP_NOTIFY;

	return HTTP_TYPE_UNKNOWN;
}


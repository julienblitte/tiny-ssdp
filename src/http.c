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
	pattern = (char *)malloc(len+1); // pattern must be freed (1)

	snprintf(pattern, len+1, "%s:", variable);

	for(l=0; t[l]; l++)
	{
		if (!strncasecmp(t[l], pattern, len))
		{
			free(pattern); // pattern freed here (1) 1/2
			return (const char *)(t[l] + len);
		}
	}

	free(pattern); // pattern freed here (1) 2/2
	return NULL;
}

// * Warning: returned value must be freed
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


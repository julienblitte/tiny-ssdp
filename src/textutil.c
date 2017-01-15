#include "textutil.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <syslog.h>

#include "logfilter.h"
#define MAX_LINES	1024

text data2text(char *data)
{
	char **result;
	unsigned int i;
	bool newline;

 	result = (char **)calloc(MAX_LINES, sizeof(char *));
	if (!result)
		return NULL;

	i = 0;
	newline = true;
	while(*data && i < MAX_LINES-1)
	{
		switch(*data)
		{
			case '\r':
			case '\n':
				*data = '\0';
				newline = true;
			break;
			default:
				if (newline)
				{
					newline = false;
					result[i] = data;
					i++;
				}
			break;
		}
		data++;
	}

	result[i] = NULL;

	return result;
}

void writelog_text(int priority, text t)
{
	unsigned int l;

	for(l=0; t[l]; l++)
	{
		writelog(priority, "%u: %s\n", l, t[l]);	
	}
}

int str_find_line(text t, const char *line)
{
	int l;

	for(l=0; t[l]; l++)
	{
		if (!strcasecmp(t[l], line))
		{
			return l;
		}
	}

	return -1;
}

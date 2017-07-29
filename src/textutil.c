/* Misc. text utilility functions
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

#include "textutil.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <strings.h>
#include <syslog.h>

#include "logfilter.h"
#define MAX_LINES	1024

// * Warning: returned value must be freed!
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

	if (i < MAX_LINES-1)
	{
		result = realloc(result, sizeof(char*)*(i+1));
	}

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

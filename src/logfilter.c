/* Logging API
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

#include <syslog.h>
#include <stdarg.h>

int syslog_filter;

void writelog(int priority, const char *format, ...)
{
	va_list ap;

	if (syslog_filter == 0 || priority <= syslog_filter)
	{
		va_start(ap, format);
		vsyslog(priority, format, ap);
		va_end(ap);
	}
}


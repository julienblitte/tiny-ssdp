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


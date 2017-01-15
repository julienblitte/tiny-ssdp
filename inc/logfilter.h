#include <syslog.h>
#include <stdarg.h>

#define SYSLOG_IDENT	"ssdpd"

extern int syslog_filter;

void writelog(int priority, const char *format, ...);

// #define _writelog(p...) if(p=<syslog_filter) syslog(p, __VA_ARGS__)



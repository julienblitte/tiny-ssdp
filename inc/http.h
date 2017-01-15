#include "textutil.h"

#define HTTP_TYPE_UNKNOWN	0
#define HTTP_TYPE_GET	1
#define HTTP_TYPE_POST	2
#define HTTP_TYPE_SSDP_SEARCH	100
#define HTTP_TYPE_SSDP_NOTIFY	101

const char *http_search_variable(text t, const char *variable);
char *http_dup_clean_value(const char *s);
int http_type(text t);


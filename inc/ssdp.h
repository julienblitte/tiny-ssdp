// keep as protocol definition
#define SSDP_MAN_DISCOVER	"ssdp:discover"
#define SSDP_NOTIFY_NTS		"ssdp:alive"
#define SSDP_EXT	""

#define SSDP_MAX_PACKETSIZE	2048

void ssdp_init();
void ssdp_notify();
void *ssdp_thread_server(void *);
void *ssdp_thread_notify(void *);


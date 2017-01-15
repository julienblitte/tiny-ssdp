int udp_listen(int port);
int multicast_join_group(int socket_handle, const char *address, const char *interface);

int multicast_speak(const char *interface);
int multicast_send(int socket_descriptor, const char *address, int port, const char *data, size_t datasize);

uint32_t get_ip_address();
const char *iptos(uint32_t ip);


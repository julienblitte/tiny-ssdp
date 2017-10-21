/* tiny-ssdp: Network API
 *
 * Copyright (C) 2017  Julien Blitte <julien.blitte@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <strings.h>
#include <unistd.h>

int udp_listen(const int port)
{
	int socket_descriptor;
	struct sockaddr_in local_socket;

	/* Create a datagram socket on which to receive. */
	socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_descriptor < 0)
	{
		return 0;
	}

	/* Bind port */
	memset((char *) &local_socket, 0, sizeof(local_socket));
	local_socket.sin_family = AF_INET;
	local_socket.sin_port = htons(port);
	local_socket.sin_addr.s_addr = INADDR_ANY;
	if(bind(socket_descriptor, (struct sockaddr*)&local_socket, sizeof(local_socket)))
	{
		close(socket_descriptor);
		return 0;
	}
	return socket_descriptor;
}

int multicast_join_group(int socket_descriptor, const char *address, const char *interface)
{
	struct ip_mreq group;

	group.imr_multiaddr.s_addr = inet_addr(address);
	group.imr_interface.s_addr = inet_addr(interface);
	if (setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		return 0;
	}

	return socket_descriptor;
}

int multicast_speak(const char *interface)
{
	int socket_descriptor;
	struct in_addr interface_addr;

	socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_descriptor < 0)
	{
		return 0;
	}

	interface_addr.s_addr = inet_addr(interface);
	if (setsockopt(socket_descriptor, IPPROTO_IP, IP_MULTICAST_IF, &interface_addr, sizeof(interface_addr)) < 0)
	{
		close(socket_descriptor);
		return 0;
	}

	return socket_descriptor;
}

int multicast_send(int socket_descriptor, const char *address, int port, const char *data, size_t datasize)
{
	struct sockaddr_in group_socket;

	memset((char *)&group_socket, 0, sizeof(group_socket));
	group_socket.sin_family = AF_INET;
	group_socket.sin_addr.s_addr = inet_addr(address);
	group_socket.sin_port = htons(port);

	if (sendto(socket_descriptor, data, datasize, 0, (struct sockaddr*)&group_socket, sizeof(group_socket)) < 0)
	{
		return 0;
	}

	return 1;
}


uint32_t get_ip_address()
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	uint32_t result;

	if (getifaddrs(&ifaddr) == -1)
	{
		return 0;
	}

	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
	{
		if (ifa->ifa_addr == NULL)
			continue;

		if (ifa->ifa_addr->sa_family != AF_INET)
			continue;

		if (strcmp(ifa->ifa_name, "lo") == 0)
			continue;

		/* AF_INET */
		result = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
		freeifaddrs(ifaddr);
		return result;
	}

	freeifaddrs(ifaddr);
	return 0;
}

const char *iptos(uint32_t ip)
{
	static char result[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, (void *)&ip, result, sizeof(result));

	return result;
}


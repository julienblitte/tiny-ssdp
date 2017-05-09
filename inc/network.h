/* Network API
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

int udp_listen(int port);
int multicast_join_group(int socket_handle, const char *address, const char *interface);

int multicast_speak(const char *interface);
int multicast_send(int socket_descriptor, const char *address, int port, const char *data, size_t datasize);

uint32_t get_ip_address();
const char *iptos(uint32_t ip);


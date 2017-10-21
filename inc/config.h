/* tiny-ssdp: Configuration file parser and generator
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

#define MAX_PARAM 256
#define MAX_SERVICE 16
#define CONFIG_FILE "/etc/ssdpd"

typedef struct _ssdp_param_set
{
	#define VAR_INT_NETWORK_PORT "network_port"
	#define DEFAULT_INT_NETWORK_PORT 1900
	int  network_port;

	#define VAR_STR_NETWORK_MULTICAST "network_multicast"
	#define DEFAULT_STR_NETWORK_MULTICAST "239.255.255.250"
	char network_multicast[MAX_PARAM];

	#define VAR_STR_NETWORK_INTERFACE "network_interface"
	#define DEFAULT_STR_NETWORK_INTERFACE "0.0.0.0"
	char network_interface[MAX_PARAM];

	#define VAR_STR_DEVICE_DESCRIPTION "device_description"
	#define DEFAULT_STR_DEVICE_DESCRIPTION "Linux 3.x, UPnP/1.1, My Device/1.0"
	char device_description[MAX_PARAM];

	#define VAR_STR_DEVICE_TYPE_NAME "device_type_name"
	#define DEFAULT_STR_DEVICE_TYPE_NAME "Basic"
	char device_type_name[MAX_PARAM];

	#define VAR_INT_DEVICE_TYPE_VERSION "device_type_version"
	#define DEFAULT_INT_DEVICE_TYPE_VERSION 1
	int  device_type_version;

	#define VAR_STR_DEVICE_UUID "device_uuid"
	char device_uuid[MAX_PARAM];

	#define VAR_INT_BASE_URL_PORT "base_url_port"
	#define DEFAULT_INT_BASE_URL_PORT 1900
	int  base_url_port;

	#define VAR_STR_BASE_URL_RESOURCE "base_url_resource"
	#define DEFAULT_STR_BASE_URL_RESOURCE "/"
	char base_url_resource[MAX_PARAM];

	#define VAR_INT_SERVICE_COUNT "service_count"
	#define DEFAULT_INT_SERVICE_COUNT 1
	int  service_count;

	#define VAR_SET_STR_SERVICE_NAME "service_name[%d]"
	#define DEFAULT_SET_STR_SERVICE_NAME "MyService%d"
	char*service_name[MAX_SERVICE];

	#define VAR_SET_INT_SERVICE_VERSION "service_version[%d]"
	#define DEFAULT_SET_INT_SERVICE_VERSION "1"
	int  service_version[MAX_SERVICE];

	#define VAR_INT_SSDP_ALLOWED_CACHE "ssdp_allowed_cache"
	#define DEFAULT_INT_SSDP_ALLOWED_CACHE 3600
	int  ssdp_allowed_cache;

	#define VAR_INT_SYSLOG_LEVEL "syslog_level"
	int  syslog_level;
} ssdp_param_set;

/* values for web services part */
#define VARS_WEBSERVICES "manufacturer_name=Julien Blitte\nmanufacturer_url=http://marin.jb.free.fr/ssdp\n\nmodel_name=My Device\nmodel_number=1.0\nmodel_url=http://mydevice.com\nfriendly_name=My Device\n"

extern ssdp_param_set configuration;

void config_init();
void config_load();


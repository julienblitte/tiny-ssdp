/* HTTP parser
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

#define HTTP_TYPE_UNKNOWN	0
#define HTTP_TYPE_GET	1
#define HTTP_TYPE_POST	2
#define HTTP_TYPE_SSDP_SEARCH	100
#define HTTP_TYPE_SSDP_NOTIFY	101

const char *http_search_variable(text t, const char *variable);
char *http_dup_clean_value(const char *s);
int http_type(text t);


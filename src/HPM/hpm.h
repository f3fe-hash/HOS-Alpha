#ifndef __HPM_H__
#define __HPM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hpm_utils.h"

package_t *get_package(const char* name, const char* version, const char* source);
package_t *__parse_package(const char* buffer, size_t buffer_size);
package_t *__install_package(package_t* package);

void __send_package_request(const char* name, const char* version, const char* source);

#endif
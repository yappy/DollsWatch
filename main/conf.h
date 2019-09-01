#ifndef CONF_H_
#define CONF_H_

#include <stdint.h>

const uint32_t MAIN_TASK_STACK_SIZE = 8 * 1024;
const uint32_t MAIN_TASK_CORE = 1;
// (low) 0.. (configMAX_PRIORITIES - 1) (high)
const uint32_t MAIN_TASK_PRIORITY = 1;
const uint32_t MAIN_LOOP_SLEEP = 100;

const char WIFI_CONF_FILE[] = "/conf/wifi.txt";
// GMT offset sec
const long WIFI_NTP_TZ = 9 * 3600;
// Daylight offset sec
const int WIFI_NTP_DAYLIGHT = 0;
const char WIFI_NTP_SERVER[] = "ntp.jst.mfeed.ad.jp";

const uint32_t HTTP_TASK_STACK_SIZE = 16 * 1024;
const char HTTP_FILE_ROOT[] = "/web/";
const uint32_t HTTP_GET_QUERY_MAX = 64;
const uint32_t HTTP_IO_BUF_SIZE = 1024;
const uint32_t HTTP_FILE_NAME_MAX = 32;
const uint32_t HTTP_FILE_PATH_MAX = sizeof(HTTP_FILE_ROOT) + HTTP_FILE_NAME_MAX;

const char *const HTTP_FILE_ROOTS[] = {
	"/sd",
	"/spiffs",
	nullptr,
};

#endif // CONF_H_

#include "http_server.h"
#include "http_mime_type.h"
#include "conf.h"
#include "res/res.h"
#include "script_lua.h"
#include <M5Stack.h>
#include <cJSON.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <memory>

namespace {

	struct ExtraSpace {
		httpd_req_t *req;
	};
	static_assert(sizeof(ExtraSpace) <= LUA_EXTRASPACE, "LUA_EXTRASPACE");

	inline ExtraSpace& get_extra(lua_State *L)
	{
		return *(ExtraSpace *)lua_getextraspace(L);
	}

	template<size_t N>
	esp_err_t send_literal_chunk(httpd_req_t *req, const char (&str)[N])
	{
		return httpd_resp_send_chunk(req, str, sizeof(str) - 1);
	}

	esp_err_t send_http_error(httpd_req_t *req, int code,
		const char *msg = nullptr)
	{
		const char *status;
		switch (code) {
		case 400:
			status = "400 Bad Request";
			break;
		case 405:
			status = "405 Method Not Allowed";
			break;
		case 500:
		default:
			status = "500 Internal Server Error";
			break;
		}
		if (msg == nullptr) {
			msg = status;
		}
		httpd_resp_set_status(req, status);
		httpd_resp_set_type(req, "text/plain");
		return httpd_resp_send(req, msg, strlen(msg));
	}

	void url_decode(char *buf)
	{
		const char *hex = "0123456789abcdefABCDEF";
		const uint32_t conv[] = { 0,1,2,3,4,5,6,7,8,9,
			10,11,12,13,14,15, 10,11,12,13,14,15 };
		char *in = buf;
		char *out = buf;
		while (*in != '\0') {
			if (*in == '%') {
				if (in[1] == '\0' || in[2] == '\0') {
					goto ASIS;
				}
				const char *p1 = strchr(hex, in[1]);
				const char *p2 = strchr(hex, in[2]);
				if (p1 == nullptr || p2 == nullptr) {
					goto ASIS;
				}
				char c = (char)((conv[p1 - hex] << 4) | conv[p2 - hex]);
				*out = c;
				in += 3;
				out++;
				continue;
			}
ASIS:
			*out = *in;
			in++;
			out++;
		}
		*out = '\0';
	}

	bool is_valid_filepath(const char *str)
	{
		bool found = false;
		for (const auto *root = HTTP_FILE_ROOTS; *root != nullptr; root++) {
			if (strncmp(str, *root, strlen(*root)) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}

		bool begin_with_alnum = false;
		while (*str != '\0') {
			if (isalnum(*str)) {
				// OK
				begin_with_alnum = true;
			}
			else if (*str == '.') {
				// ".." not allowed
				if (*(str + 1) == '.') {
					return false;
				}
			}
			else if (*str == '/' || *str == '_' || *str == '-') {
				// OK
			}
			else {
				return false;
			}
			str++;
		}
		return begin_with_alnum;
	}

	const char *search_mime_type(const char *filename)
	{
		const char *ext = strrchr(filename, '.');
		if (ext == nullptr) {
			return MIME_DEFAULT;
		}
		ext++;

		char lower_ext[MIME_EXT_MAX];
		if (strlen(ext) > MIME_EXT_MAX - 1) {
			return MIME_DEFAULT;
		}
		strcpy(lower_ext, ext);
		char *p = lower_ext;
		while (*p != '\0') {
			*p = tolower(*p);
			p++;
		}

		auto compair = [](const void *pa, const void *pb) -> int {
			auto a = (const MimeTypeElem *)pa;
			auto b = (const MimeTypeElem *)pb;
			return strcmp(a->ext, b->ext);
		};
		MimeTypeElem key = {
			.ext = lower_ext,
			.mime = nullptr,
		};
		const MimeTypeElem *result = (const MimeTypeElem *)bsearch(
			&key, MIME_LIST, MIME_LIST_COUNT, sizeof(MimeTypeElem), compair);
		if (result == nullptr) {
			return MIME_DEFAULT;
		}
		else {
			return result->mime;
		}
	}

} // namespace

void HttpServer::start()
{
	stop();

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	// config.task_priority = tskIDLE_PRIORITY+5
	config.stack_size = HTTP_TASK_STACK_SIZE;
	// config.server_port = 80
	config.max_uri_handlers = 16;

	ESP_ERROR_CHECK(httpd_start(&m_handle, &config));

	setup_pages();
}

void HttpServer::stop()
{
	if (m_handle != nullptr) {
		ESP_ERROR_CHECK(httpd_stop(m_handle));
		m_handle = nullptr;
	}
}

bool HttpServer::is_running()
{
	return (m_handle != nullptr);
}

void HttpServer::setup_pages()
{
	httpd_uri_t uri_index {
		.uri      = "/",
		.method   = HTTP_GET,
		.handler  = page_script,
		.user_ctx = this,
	};
	uri_index.method = HTTP_GET;
	ESP_ERROR_CHECK(httpd_register_uri_handler(m_handle, &uri_index));
	uri_index.method = HTTP_POST;
	ESP_ERROR_CHECK(httpd_register_uri_handler(m_handle, &uri_index));

	httpd_uri_t uri_recovery {
		.uri      = "/recovery",
		.method   = HTTP_GET,
		.handler  = page_recovery,
		.user_ctx = this,
	};
	ESP_ERROR_CHECK(httpd_register_uri_handler(m_handle, &uri_recovery));

	httpd_uri_t uri_recovery_file {
		.uri      = "/recovery/file",
		.method   = HTTP_GET,
		.handler  = page_recovery_file,
		.user_ctx = this,
	};
	uri_recovery_file.method = HTTP_GET;
	ESP_ERROR_CHECK(httpd_register_uri_handler(m_handle, &uri_recovery_file));
	uri_recovery_file.method = HTTP_POST;
	ESP_ERROR_CHECK(httpd_register_uri_handler(m_handle, &uri_recovery_file));

	httpd_uri_t uri_recovery_download {
		.uri      = "/recovery/download",
		.method   = HTTP_GET,
		.handler  = page_recovery_download,
		.user_ctx = this,
	};
	ESP_ERROR_CHECK(httpd_register_uri_handler(
		m_handle, &uri_recovery_download));
}

// send res/file_server.html
esp_err_t HttpServer::page_recovery(httpd_req_t *req)
{
	return httpd_resp_send(req, (const char *)file_server_html_start,
		file_server_html_end - file_server_html_start);
}

// file download
esp_err_t HttpServer::page_recovery_download(httpd_req_t *req)
{
	esp_err_t ret;

	char query[PATH_MAX];
	ret = httpd_req_get_url_query_str(req, query, sizeof(query));
	if (ret != ESP_OK) {
		return send_http_error(req, 400, "File path query required");
	}
	url_decode(query);
	if (!is_valid_filepath(query)) {
		return send_http_error(req, 400, "Invalid file path");
	}
	printf("download: %s\n", query);

	const char *mime = search_mime_type(query);
	httpd_resp_set_type(req, mime);

	auto fp_del = [](FILE *fp) {
		fclose(fp);
	};
	std::unique_ptr<FILE, decltype(fp_del)> fp{fopen(query, "rb"), fp_del};
	if (fp == nullptr) {
		return send_http_error(req, 404);
	}

	auto buf = std::unique_ptr<uint8_t[]>(
		new(std::nothrow) uint8_t[HTTP_IO_BUF_SIZE]);
	if (buf == nullptr) {
		return send_http_error(req, 500, "Memory error");
	}

	size_t read_size;
	while ((read_size = fread(buf.get(), 1, HTTP_IO_BUF_SIZE, fp.get())) > 0) {
		httpd_resp_send_chunk(req, (const char *)buf.get(), read_size);
	}

	return httpd_resp_send_chunk(req, nullptr, 0);
}

// REST root
esp_err_t HttpServer::page_recovery_file(httpd_req_t *req)
{
	esp_err_t ret;
	HttpServer *self = static_cast<HttpServer *>(req->user_ctx);

	char cmd[8];
	ret = httpd_req_get_hdr_value_str(req, "FILE-CMD", cmd, sizeof(cmd));
	if (ret != ESP_OK) {
		return send_http_error(req, 400, "Invalid FILE-CMD in HTTP header");
	}
	if (strcmp(cmd, "LIST") == 0) {
		if (req->method == HTTP_GET) {
			return self->file_list(req);
		}
		else {
			return send_http_error(req, 405);
		}
	}
	else if (strcmp(cmd, "STAT") == 0) {
		return send_http_error(req, 500, "Not implemented");
	}
	else if (strcmp(cmd, "UPLOAD") == 0) {
		if (req->method == HTTP_POST) {
			char path[PATH_MAX];
			ret = httpd_req_get_hdr_value_str(
				req, "FILE-PATH", path, sizeof(path));
			if (ret != ESP_OK) {
				return send_http_error(req, 400,
					"Invalid FILE-PATH in HTTP header");
			}
			return self->file_upload(req, path);
		}
		else {
			return send_http_error(req, 405);
		}
		return send_http_error(req, 500, "Not implemented");
	}
	else if (strcmp(cmd, "MKDIR") == 0) {
		if (req->method == HTTP_POST) {
			char path[PATH_MAX];
			ret = httpd_req_get_hdr_value_str(
				req, "FILE-PATH", path, sizeof(path));
			if (ret != ESP_OK) {
				return send_http_error(req, 400,
					"Invalid FILE-PATH in HTTP header");
			}
			return self->file_mkdir(req, path);
		}
		else {
			return send_http_error(req, 405);
		}
	}
	else if (strcmp(cmd, "DELETE") == 0) {
		if (req->method == HTTP_POST) {
			char path[PATH_MAX];
			ret = httpd_req_get_hdr_value_str(
				req, "FILE-PATH", path, sizeof(path));
			if (ret != ESP_OK) {
				return send_http_error(req, 400,
					"Invalid FILE-PATH in HTTP header");
			}
			return self->file_del(req, path);
		}
		else {
			return send_http_error(req, 405);
		}
	}
	else {
		return send_http_error(req, 400,
			"Valid FILE-CMD required in HTTP header");
	}
}


bool HttpServer::file_list_rec(httpd_req_t *req, char *namebuf, size_t size,
	bool is_first)
{
	printf("Scan: %s\n", namebuf);
	DIR *dirp = opendir(namebuf);
	if (dirp == nullptr) {
		// print and ignore
		perror("opendir");
		return is_first;
	}

	size_t orglen = strlen(namebuf);
	struct dirent *entry;
	while ((entry = readdir(dirp)) != nullptr) {
		// dir + "/" + name + "/"? + nul
		if (orglen + 1 + strlen(entry->d_name) + 1 >= size) {
			// path length over, ignore
			continue;
		}
		namebuf[orglen] = '/';
		strcpy(namebuf + orglen + 1, entry->d_name);

		if (entry->d_type == DT_REG) {
			cJSON *json = cJSON_CreateStringReference(namebuf);
			char *print = cJSON_PrintUnformatted(json);
			if (!is_first) {
				send_literal_chunk(req, ",\n");
			}
			httpd_resp_send_chunk(req, print, strlen(print));
			cJSON_free(print);
			is_first = false;
		}
		else if (entry->d_type == DT_DIR) {
			// mark '/' at the end of name
			strcat(namebuf, "/");

			cJSON *json = cJSON_CreateStringReference(namebuf);
			char *print = cJSON_PrintUnformatted(json);
			if (!is_first) {
				send_literal_chunk(req, ",\n");
			}
			httpd_resp_send_chunk(req, print, strlen(print));
			cJSON_free(print);
			is_first = false;

			// remove '/'
			namebuf[strlen(namebuf) - 1] = '\0';

			// recursive call
			file_list_rec(req, namebuf, size, is_first);
		}
		else {
			// ignore
		}

		// restore path
		namebuf[orglen] = '\0';
	}
	closedir(dirp);
	return is_first;
}

esp_err_t HttpServer::file_list(httpd_req_t *req)
{
	// consume stack 1024
	char namebuf[PATH_MAX];

	httpd_resp_set_type(req, "application/json");
	send_literal_chunk(req, "[\n");
	bool is_first = true;
	for (const auto *root = HTTP_FILE_ROOTS; *root != nullptr; root++) {
		strcpy(namebuf, *root);
		is_first = file_list_rec(req, namebuf, sizeof(namebuf), is_first);
	}
	send_literal_chunk(req, "]\n");

	return httpd_resp_send_chunk(req, nullptr, 0);
}

esp_err_t HttpServer::file_upload(httpd_req_t *req, const char *path)
{
	if (!is_valid_filepath(path)) {
		return send_http_error(req, 400, "Invalid file path");
	}

	printf("upload: %s\n", path);
	auto fp_del = [](FILE *fp) {
		fclose(fp);
	};
	std::unique_ptr<FILE, decltype(fp_del)> fp{fopen(path, "wb"), fp_del};
	if (fp == nullptr) {
		return send_http_error(req, 500, "Open failed");
	}

	auto buf = std::unique_ptr<uint8_t[]>(
		new(std::nothrow) uint8_t[HTTP_IO_BUF_SIZE]);
	if (buf == nullptr) {
		return send_http_error(req, 500, "Memory error");
	}
	size_t rest = req->content_len;
	while (rest > 0) {
		size_t recv_size = httpd_req_recv(
			req, (char *)buf.get(), min(rest, HTTP_IO_BUF_SIZE));
		if (recv_size > 0) {
			rest -= recv_size;
			if (fwrite(buf.get(), recv_size, 1, fp.get()) != 1) {
				return send_http_error(req, 500, "Write failed");
			}
		}
		else {
			return ESP_FAIL;
		}
	}
	httpd_resp_send(req, "", 0);
	return ESP_OK;
}

esp_err_t HttpServer::file_mkdir(httpd_req_t *req, const char *path)
{
	if (!is_valid_filepath(path)) {
		return send_http_error(req, 400, "Invalid file path");
	}

	int ret = mkdir(path, 0777);
	if (ret < 0) {
		perror("mkdir");
		return send_http_error(req, 500, "mkdir failed");
	}

	return httpd_resp_send(req, "", 0);
}

esp_err_t HttpServer::file_del(httpd_req_t *req, const char *path)
{
	if (!is_valid_filepath(path)) {
		return send_http_error(req, 400, "Invalid file path");
	}

	int ret = remove(path);
	if (ret < 0) {
		perror("remove");
		return send_http_error(req, 500, "remove failed");
	}

	return httpd_resp_send(req, "", 0);
}

esp_err_t HttpServer::page_script(httpd_req_t *req)
{
	esp_err_t ret;
	int luaret;

	const char *method = nullptr;
	switch (req->method) {
	case HTTP_GET:
		method = "GET";
		break;
	case HTTP_POST:
		method = "POST";
		break;
	default:
		return ESP_FAIL;
	}

	char query[HTTP_GET_QUERY_MAX];
	ret = httpd_req_get_url_query_str(req, query, sizeof(query));
	if (ret != ESP_OK) {
		query[0] = '\0';
	}
	url_decode(query);

	Lua lua;
	bool ok = lua.init();
	if (!ok) {
		return send_http_error(req, 500);
	}
	lua_State *L = lua.get();
	get_extra(L).req = req;

	lua_pushboolean(L, 1);
	lua_setglobal(L, "WEBAPP");
	ok = lua.eval_file(HTTP_LUA_INDEX);
	if (!ok) {
		return send_http_error(req, 500);
	}

	// TODO: may cause error
	// push global function "init"
	lua_settop(L, 0);
	lua_getglobal(L, "init");
	// call
	luaret = lua_pcall(L, 0, 0, 0);
	if (luaret != LUA_OK) {
		printf("lua_pcall init: %d\n", luaret);
		if (lua_gettop(L) >= 1) {
			printf("%s\n", lua_tostring(L, -1));
		}
		return send_http_error(req, 500);;
	}

	lua_Integer content_len = (lua_Integer)req->content_len;
	auto recv = [](lua_State *L) -> int {
		lua_Integer lsize = luaL_checkinteger(L, 1);
		if (lsize <= 0) {
			luaL_error(L, "Recv size must > 0");
			// never returns
		}
		lua_settop(L, 0);
		// allocate buffer in lua mm with gc and push
		size_t size = (size_t)lsize;
		void *buf = lua_newuserdata(L, size);
		// receive to it
		esp_err_t ret = httpd_req_recv(get_extra(L).req, (char *)buf, size);
		if (ret <= 0) {
			luaL_error(L, "httpd_req_recv: %d", ret);
			// never returns
		}
		// push received data as a string
		lua_pushlstring(L, (const char *)buf, ret);
		// discard buffer (to be gc-ed)
		lua_remove(L, 1);
		// returns received data (string)
		return 1;
	};

	auto call_loop = [L, method, &query, content_len, recv]() -> int {
		// TODO: may cause error
		// push global function "loop"
		lua_settop(L, 0);
		lua_getglobal(L, "loop");
		// push args
		lua_pushstring(L, HTTP_LUA_ROOT);
		lua_pushstring(L, method);
		lua_pushstring(L, query);
		lua_pushinteger(L, content_len);
		lua_pushcfunction(L, recv);
		// call
		int luaret = lua_pcall(L, 5, LUA_MULTRET, 0);
		if (luaret != LUA_OK) {
			printf("lua_pcall loop: %d\n", luaret);
			if (lua_gettop(L) >= 1) {
				printf("%s\n", lua_tostring(L, -1));
			}
		}
		return luaret;
	};

	// string status, string type, string body
	if (call_loop() != LUA_OK) {
		return send_http_error(req, 500);
	}
	if (lua_gettop(L) != 2) {
		printf("Expected 2, but %d\n", lua_gettop(L));
		return send_http_error(req, 500);
	}
	if (lua_tostring(L, 1) == nullptr || lua_tostring(L, 2) == nullptr) {
		printf("Expected string\n");
		return send_http_error(req, 500);
	}
	httpd_resp_set_status(req, lua_tostring(L, 1));
	httpd_resp_set_type(req, lua_tostring(L, 2));

	// TODO: http header
	if (call_loop() != LUA_OK) {
		return ESP_FAIL;
	}

	// body
	while (1) {
		if (call_loop() != LUA_OK) {
			return ESP_FAIL;
		}
		// void returned: end
		if (lua_gettop(L) == 0) {
			break;
		}
		if (lua_gettop(L) != 1) {
			printf("Expected 1, but %d\n", lua_gettop(L));
			return ESP_FAIL;
		}
		if (lua_tostring(L, 1) == nullptr) {
			printf("Expected string\n");
			return ESP_FAIL;
		}
		size_t size = 0;
		const char *body = lua_tolstring(L, 1, &size);
		httpd_resp_send_chunk(req, body, size);
	}
	httpd_resp_send_chunk(req, nullptr, 0);
	return ESP_OK;
}

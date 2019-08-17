#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <esp_http_server.h>

class HttpServer final {
public:
	HttpServer() : m_handle(nullptr) {}
	~HttpServer() = default;
	HttpServer(const HttpServer &) = delete;
	HttpServer & operator =(const HttpServer &) = delete;

	void start();
	void stop();
	bool is_running();

private:
	httpd_handle_t m_handle;

	void setup_pages();
	static esp_err_t page_recovery_get(httpd_req_t *req);
	static esp_err_t page_recovery_post(httpd_req_t *req);
	static esp_err_t page_recovery_delete(httpd_req_t *req);
	static esp_err_t page_recovery_file(httpd_req_t *req);
	static esp_err_t page_script(httpd_req_t *req);

	bool file_list_rec(httpd_req_t *req, char *namebuf, size_t size,
		bool is_first);
	esp_err_t file_list(httpd_req_t *req);
	esp_err_t file_upload(httpd_req_t *req, const char *path);
	esp_err_t file_mkdir(httpd_req_t *req, const char *path);
	esp_err_t file_del(httpd_req_t *req, const char *path);
};

#endif // HTTP_SERVER_H_

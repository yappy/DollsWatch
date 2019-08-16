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
};

#endif // HTTP_SERVER_H_

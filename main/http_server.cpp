#include "http_server.h"
#include <http_parser.h>

void HttpServer::start()
{
	stop();

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	// config.task_priority = tskIDLE_PRIORITY+5
	// config.server_port = 80
	// config.max_uri_handlers = 8

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
		.handler  = page_index,
		.user_ctx = this,
	};
	httpd_register_uri_handler(m_handle, &uri_index);
}

esp_err_t HttpServer::page_index(httpd_req_t *req)
{
	// httpd_resp_set_status()
	// httpd_resp_set_type()
	// httpd_resp_set_hdr()
	static const char DefPage[] =
R"(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8" />
<title>Hello</title>
</head>
<body>
<p>Hello my watch!!!!!</p>
</body>
</html>
)";
	httpd_resp_send(req, DefPage, sizeof(DefPage) - 1);
	return ESP_OK;
}

#include "http_server.h"
#include <http_parser.h>
#include <algorithm>

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
		.handler  = page_index_get,
		.user_ctx = this,
	};
	httpd_register_uri_handler(m_handle, &uri_index);

	httpd_uri_t uri_files {
		.uri      = "/files",
		.method   = HTTP_GET,
		.handler  = page_files_get,
		.user_ctx = this,
	};
	httpd_register_uri_handler(m_handle, &uri_files);

	httpd_uri_t uri_upload {
		.uri      = "/upload",
		.method   = HTTP_POST,
		.handler  = page_upload_post,
		.user_ctx = this,
	};
	httpd_register_uri_handler(m_handle, &uri_upload);
}

esp_err_t HttpServer::page_index_get(httpd_req_t *req)
{
	// httpd_resp_set_status()
	// httpd_resp_set_type()
	// httpd_resp_set_hdr()
	static const char Page[] =
R"(<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" />
  <title>Hello</title>
</head>
<body>
  <p>Hello my watch!!!!!</p>
  <p><a href="./files">File Server</a></p>
</body>
</html>
)";
	httpd_resp_send(req, Page, sizeof(Page) - 1);
	return ESP_OK;
}

esp_err_t HttpServer::page_files_get(httpd_req_t *req)
{
	static const char Page[] =
R"(<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" />
  <title>File Server</title>
</head>
<body>
  <div><input id="upload_file" type="file" /></div>
  <div><input id="upload_button" type="button" value="upload" /></div>
  <div><progress id="upload_prog" max="100" value="0" /></div>
  <div><p id="upload_msg"></p></div>

<script>
var post_file = function(upload_file) {
  var content_length = upload_file.size
  var content_type = upload_file.type
  var file_name = upload_file.name;
  var msg = document.getElementById("upload_msg");

  var xhr = new XMLHttpRequest();
  xhr.open('POST', './upload', true);
  xhr.setRequestHeader('Content-type', content_type);
  xhr.setRequestHeader('Content-Length', content_length);
  xhr.setRequestHeader('X-FILE-NAME', file_name);

  var progress = document.getElementById("upload_prog");
  xhr.onreadystatechange = function () {
    if(xhr.readyState == 4) {
      if (xhr.status === 200) {
        msg.innerText = xhr.responseText;
      }
      else {
        msg.innerText = "error!";
      }
    }
  };
  xhr.upload.onprogress = function(e) {
    if (e.lengthComputable) {
      progress.value = (e.loaded / e.total) * 100;
    }
  };
  xhr.send(upload_file);
};

document.getElementById("upload_button").addEventListener(
  "click",
  function() {
    var element_file = document.getElementById("upload_file");
    var upload_file = element_file.files[0];
    post_file(upload_file);
  },
  false);
</script>

</body>
</html>
)";
	httpd_resp_send(req, Page, sizeof(Page) - 1);
	return ESP_OK;
}

esp_err_t HttpServer::page_upload_post(httpd_req_t *req)
{
	const char * const HexChar = "0123456789abcdef";
	char buf[16];
	size_t rest = req->content_len;
	while (rest > 0) {
		size_t recv_size = httpd_req_recv(
			req, buf, std::min(rest, sizeof(buf)));
		if (recv_size > 0) {
			rest -= recv_size;
			for (size_t i = 0 ; i < recv_size; i++) {
				char str[3];
				str[0] = HexChar[(uint32_t)buf[i] >> 4];
				str[1] = HexChar[(uint32_t)buf[i] & 0x0f];
				str[2] = ' ';
				httpd_resp_send_chunk(req, str, 3);
			}
		}
		else {
			return ESP_FAIL;
		}
	}
	httpd_resp_send_chunk(req, nullptr, 0);
	return ESP_OK;
}

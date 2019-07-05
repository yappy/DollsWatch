#include "http_server.h"
#include "http_mime_type.h"
#include "conf.h"
#include <M5Stack.h>
#include <http_parser.h>
#include <ctype.h>
#include <memory>

namespace {

	esp_err_t send_http_error(httpd_req_t *req, int code)
	{
		const char *msg, *status;
		switch (code) {
		case 400:
			msg = status = "400 Bad Request";
			break;
		case 500:
		default:
			msg = status = "500 Internal Server Error";
			break;
		}
		httpd_resp_set_status(req, status);
		httpd_resp_set_type(req, "text/html");
		return httpd_resp_send(req, msg, strlen(msg));
	}

	bool is_valid_filename(const char *str)
	{
		bool appear_normal = false;
		while (*str != '\0') {
			if (isalnum(*str)) {
				// OK
				appear_normal = true;
			}
			else if (*str == '.' || *str == '_' || *str == '-') {
				// OK
			}
			else {
				return false;
			}
			str++;
		}
		return appear_normal;
	}

	const char *search_mime_type(const char *filename)
	{
		const char *ext = strrchr(filename, '.');
		if (ext == nullptr) {
			return MIME_DEFAULT;
		}
		ext++;

		auto compair = [](const void *pa, const void *pb) -> int {
			auto a = (const MimeTypeElem *)pa;
			auto b = (const MimeTypeElem *)pb;
			return strcmp(a->ext, b->ext);
		};
		MimeTypeElem key = {
			.ext = ext,
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
	static const char Page1[] =
R"(<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" />
  <title>File Server</title>
</head>
<body>
  <h1>File Server</h1>

  <div><input id="upload_file" type="file" /></div>
  <div><input id="upload_button" type="button" value="upload" /></div>
  <div><progress id="upload_prog" max="100" value="0" /></div>
  <div><p id="upload_msg"></p></div>

  <hr>

  <ul>
)";
static const char Page2[] =
R"(
  </ul>
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
        setTimeout(function() { location.reload(true); }, 1000);
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

	esp_err_t ret;
	int iret;

	char query[HTTP_GET_QUERY_MAX];
	ret = httpd_req_get_url_query_str(req, query, sizeof(query));
	if (ret == ESP_OK) {
		// query on, download mode
		if (!is_valid_filename(query)) {
			return send_http_error(req, 400);
		}

		char full_path[HTTP_FILE_PATH_MAX];
		iret = snprintf(full_path, sizeof(full_path),
			"%s%s", HTTP_FILE_ROOT, query);
		if (iret >= sizeof(full_path)) {
			return send_http_error(req, 500);
		}
		printf("download: %s\n", full_path);

		const char *mime = search_mime_type(query);
		httpd_resp_set_type(req, mime);

		SDFile file = SD.open(full_path, FILE_READ);
		if (!file) {
			return send_http_error(req, 500);
		}

		auto buf = std::unique_ptr<uint8_t[]>(
			new(std::nothrow) uint8_t[HTTP_IO_BUF_SIZE]);
		if (buf == nullptr) {
			return send_http_error(req, 500);
		}
		size_t read_size;
		while ((read_size = file.read(buf.get(), HTTP_IO_BUF_SIZE)) > 0) {
			httpd_resp_send_chunk(req, (const char *)buf.get(), read_size);
		}

		httpd_resp_send_chunk(req, nullptr, 0);
		return ESP_OK;
	}

	// send top half
	httpd_resp_send_chunk(req, Page1, sizeof(Page1) - 1);

	// send file list
	SDFile dir = SD.open(HTTP_FILE_ROOT, FILE_READ);
	if (!dir) {
		return send_http_error(req, 500);
	}
	while(true) {
		SDFile entry = dir.openNextFile();
		if (!entry) {
			break;
		}
		if (entry.isDirectory()) {
			continue;
		}
		const char *name = entry.name() + strlen(dir.name());
		char buf[HTTP_FILE_NAME_MAX * 2 + 32];
		iret = snprintf(buf, sizeof(buf),
			"    <li><a href='?%s'>%s</a></li>\n",
			name, name);
		if (iret >= sizeof(buf)) {
			return send_http_error(req, 500);
		}
		httpd_resp_send_chunk(req, buf, strlen(buf));
	}

	// send bottom half
	httpd_resp_send_chunk(req, Page2, sizeof(Page2) - 1);
	httpd_resp_send_chunk(req, nullptr, 0);
	return ESP_OK;
}

esp_err_t HttpServer::page_upload_post(httpd_req_t *req)
{
	esp_err_t ret;
	int iret;

	char name[HTTP_FILE_NAME_MAX];
	ret = httpd_req_get_hdr_value_str(req, "X-FILE-NAME", name, sizeof(name));
	if (ret != ESP_OK) {
		// not found or too long
		return send_http_error(req, 400);
	}
	if (!is_valid_filename(name)) {
		return send_http_error(req, 400);
	}

	char full_path[HTTP_FILE_PATH_MAX];
	iret = snprintf(full_path, sizeof(full_path),
		"%s%s", HTTP_FILE_ROOT, name);
	if (iret >= sizeof(full_path)) {
		return send_http_error(req, 500);
	}
	printf("upload: %s\n", full_path);
	SDFile file = SD.open(full_path, FILE_WRITE);
	if (!file) {
		return send_http_error(req, 500);
	}

	auto buf = std::unique_ptr<uint8_t[]>(
		new(std::nothrow) uint8_t[HTTP_IO_BUF_SIZE]);
	if (buf == nullptr) {
		return send_http_error(req, 500);
	}
	size_t rest = req->content_len;
	while (rest > 0) {
		size_t recv_size = httpd_req_recv(
			req, (char *)buf.get(), min(rest, HTTP_IO_BUF_SIZE));
		if (recv_size > 0) {
			rest -= recv_size;
			file.write(buf.get(), recv_size);
		}
		else {
			return ESP_FAIL;
		}
	}
	httpd_resp_send(req, "", 0);
	return ESP_OK;
}
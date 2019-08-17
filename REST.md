# REST API Specification

## CURL Example
```
curl http://<ipaddr>/recovery/file \
-v -X GET \
-H "FILE-CMD:LIST"
```

```
curl http://<ipaddr>/recovery/file \
-v -X POST \
-H "Content-Type: application/octet-stream" \
-H "FILE-CMD: UPLOAD" \
-H "FILE-PATH: /sd/uptest.jpg" \
--data-binary @yappy_house_240.jpg
```

* -v prints dedailed communication log. (e.g. HTTP headers)
* -X changes HTTP method.
* -H adds a HTTP header.
  * `application/x-www-form-urlencoded` (curl default) is not supported.
    Change Content-Type to send binary as is.
* --data-binary sends a local file as HTTP request body.


## Recovery SD files
http://_ipaddr_/recovery/file

Implemented by C++.
Can be used even if no data in SD card.

### GET /recovery/ (not REST)
* OUT
  * Content-Type: text/html

HTML top page

### GET /recovery/download?_path_ (not REST)
* IN (query)
  * file path
* OUT
  * Content-Type: Changed by file extension
  * Body: full file content

### GET /recovery/file CMD=LIST
* IN (HTTP header)
  * FILE-CMD: LIST

### GET /recovery/file CMD=STAT
* IN (HTTP header)
  * FILE-CMD: STAT
  * FILE-PATH: file path

### POST /recovery/file CMD=UPLOAD
* IN (HTTP header)
  * Content-Length: data size
  * FILE-CMD: UPLOAD
  * FILE-PATH: file path
* IN (HTTP body)
  * data content

### POST /recovery/file CMD=MKDIR
* IN (HTTP header)
  * FILE-CMD: MKDIR
  * FILE-PATH: file path

### POST /recovery/file CMD=DELETE
* IN (HTTP header)
  * FILE-CMD: DELETE
  * FILE-PATH: file path

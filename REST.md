# REST API Specification

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

### POST /recovery/file CMD=DEL
* IN (HTTP header)
  * FILE-CMD: DEL
  * FILE-PATH: file path

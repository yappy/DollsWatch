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

### GET /recovery/file CMD=READ
* IN (HTTP header)
  * FILE-CMD: READ
  * FILE-PATH: file path
  * FILE-OFFSET: read offset in decimal string
* OUT
  * Content-Type: application/octet-stream
  * Body: read data

### POST /recovery/file CMD=WRITE
* IN (HTTP header)
  * Content-Length: data size
  * FILE-CMD: WRITE
  * FILE-PATH: file path
  * WRITE-OFFSET: write offset in decimal string
* IN (HTTP body)
  * data content

### POST /recovery/file CMD=TRUNC
* IN (HTTP header)
  * Content-Length: data size
  * FILE-CMD: WRITE
  * WRITE-OFFSET: write offset in decimal string
* IN (HTTP body)
  * data content

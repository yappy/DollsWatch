#ifndef HTTP_MIME_TYPE_H_
#define HTTP_MIME_TYPE_H_

/*
 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
 */

typedef struct {
	const char *ext;
	const char *mime;
} MimeTypeElem;

const char * const MIME_DEFAULT = "application/octet-stream";
const uint32_t MIME_EXT_MAX = 6;

const MimeTypeElem MIME_LIST[] = {
	{ "aac",	"audio/aac"						},
	{ "avi",	"video/x-msvideo"				},
	{ "bmp",	"image/bmp"						},
	{ "bz",		"application/x-bzip"			},
	{ "bz2",	"application/x-bzip2"			},
	{ "css",	"text/css"						},
	{ "csv",	"text/csv"						},
	{ "doc",	"application/msword"			},
	{ "docx",	"application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
	{ "gif",	"image/gif"						},
	{ "htm",	"text/html"						},
	{ "html",	"text/html"						},
	{ "ico",	"image/vnd.microsoft.icon"		},
	{ "jar",	"application/java-archive"		},
	{ "jpeg",	"image/jpeg"					},
	{ "jpg",	"image/jpeg"					},
	{ "js",		"text/javascript"				},
	{ "json",	"application/json"				},
	{ "mid",	"audio/x-midi"					},
	{ "midi",	"audio/x-midi"					},
	{ "mp3",	"audio/mpeg"					},
	{ "mpeg",	"video/mpeg"					},
	{ "png",	"image/png"						},
	{ "pdf",	"application/pdf"				},
	{ "ppt",	"application/vnd.ms-powerpoint"	},
	{ "pptx",	"application/vnd.openxmlformats-officedocument.presentationml.presentation" },
	{ "rar",	"application/x-rar-compressed"	},
	{ "sh",		"application/x-sh"				},
	{ "tar",	"application/x-tar"				},
	{ "txt",	"text/plain"					},
	{ "wav",	"audio/wav"						},
	{ "xhtml",	"application/xhtml+xml"			},
	{ "xls",	"application/vnd.ms-excel"		},
	{ "xlsx",	"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
	{ "xml", 	"application/xml"				}, /* text/xml */
	{ "zip",	"application/zip"				},
	{ "7z",		"application/x-7z-compressed"	},
};
const uint32_t MIME_LIST_COUNT = sizeof(MIME_LIST) / sizeof(MIME_LIST[0]);

#endif // HTTP_MIME_TYPE_H_

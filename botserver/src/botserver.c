
#include <stdlib.h>
#include <string.h>
#include <kore/kore.h>
#include <kore/http.h>

#define scmd "/var/app/bots6/build/icetea_s6 "
#define bgdd " >> /dev/null 2>&1 &"

int	page(struct http_request *);
void get_body(struct http_request *req, char *dest);
char* escapeshellarg(char* str);

int
page(struct http_request *req)
{
	char body[req->content_length];	
	get_body(req, body);
	
	char *param = escapeshellarg(body);
	char cmd[sizeof(scmd) + sizeof(bgdd) - 2 + strlen(param)];
	sprintf(cmd, scmd "%s" bgdd, param);

	system(cmd);

	http_response(req, 200, "OK", 2);
	return (KORE_RESULT_OK);
}


char*
escapeshellarg(char* str) {
     char *escStr;
    int i,
        count = strlen(str),
            ptr_size = count+3;

    escStr = (char *) calloc(ptr_size, sizeof(char));
    if (escStr == NULL) {
        return NULL;
    }
    sprintf(escStr, "'");

    for(i=0; i<count; i++) {
        if (str[i] == '\'') {
                    ptr_size += 3;
            escStr = (char *) realloc(escStr,ptr_size * sizeof(char));
            if (escStr == NULL) {
                return NULL;
            }
            sprintf(escStr, "%s'\\''", escStr);
        } else {
            sprintf(escStr, "%s%c", escStr, str[i]);
        }
    }

    sprintf(escStr, "%s%c", escStr, '\'');
    return escStr;
}


void get_body(struct http_request *req, char *dest) {
	ssize_t			ret;
	struct kore_buf		*body;
	char			data[BUFSIZ];
	char			*string;

	if (req->method != HTTP_METHOD_POST)
		return;

	if (req->http_body != NULL) {
		body = NULL;
		req->http_body->offset = req->content_length;
		string = kore_buf_stringify(req->http_body, NULL);
	} else {
		body = kore_buf_alloc(128);
		for (;;) {
			ret = http_body_read(req, data, sizeof(data));
			if (ret == -1)
				goto out;
			if (ret == 0)
				break;
			kore_buf_append(body, data, ret);
		}
		string = kore_buf_stringify(body, NULL);
	}

retme:
	memcpy(dest, string, req->content_length);
	return;

out:
	if (body != NULL)
		kore_buf_free(body);
	goto retme;
}
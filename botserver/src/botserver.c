
#include <string.h>
#include <kore/kore.h>
#include <kore/http.h>

int	page(struct http_request *);
void get_body(struct http_request *req, char *dest);

int
page(struct http_request *req)
{
	char body[req->content_length];
	get_body(req, body);
	http_response(req, 200, body, req->content_length);
	return (KORE_RESULT_OK);
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
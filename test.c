#include <assert.h>

#include "cjson.h"

int
main(void)
{
	char buf[] = "{\"key\":\"val\"}";
	struct cjson *json = cjson_parse(buf);
	assert(strcmp(cjson_obj(json, "key")->s, "val") == 0);
	return 0;
}

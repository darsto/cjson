#include <assert.h>

#include "cjson.h"

int
main(void)
{
	{
		char buf[] = "{\"key\":\"val\"}";
		struct cjson *json = cjson_parse(buf);
		assert(strcmp(cjson_obj(json, "key")->s, "val") == 0);
		assert(cjson_obj(json, "invalid")->type == CJSON_TYPE_NONE);
		cjson_free(json);
	}

	{
		char buf[] = "[\"val1\",\"val2\"]";
		struct cjson *json = cjson_parse(buf);
		assert(strcmp(cjson_obj(json, "0")->s, "val1") == 0);
		assert(strcmp(cjson_obj(json, "1")->s, "val2") == 0);
		assert(cjson_obj(json, "2")->type == CJSON_TYPE_NONE);
		assert(cjson_obj(json, "34231343654654")->type == CJSON_TYPE_NONE);
		assert(cjson_obj(json, "string")->type == CJSON_TYPE_NONE);
		cjson_free(json);
	}

	{
		char buf[] = "{\"key\":{\"nestedkey\":\"nestedval\"}}";
		struct cjson *json = cjson_parse(buf);
		struct cjson *nested = cjson_obj(json, "key");
		assert(nested->type == CJSON_TYPE_OBJECT);
		assert(strcmp(cjson_obj(nested, "nestedkey")->s, "nestedval") == 0);
		assert(cjson_obj(nested, "invalid")->type == CJSON_TYPE_NONE);
		cjson_free(json);
	}

	return 0;
}

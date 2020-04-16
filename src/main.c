/* this is a very hastily thrown together utility */
#include <jansson.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>



ssize_t asprintf(char **string, const char *fmt, ...)
{
	va_list list;
	char *temp_string = NULL;
	char *oldstring = NULL;
	ssize_t size = 0;

	if(*string != NULL)
	{
		//free(*string);
    	oldstring = *string;
	}

	va_start(list, fmt);
	size = vsnprintf(temp_string, 0, fmt, list);
	va_end(list);
	va_start(list, fmt);

	if((temp_string = malloc(size + 1)) != NULL)
	{
    	if(vsnprintf(temp_string, size + 1, fmt, list) != -1)
    	{
    		*string = temp_string;
    		if(oldstring != NULL)
			{
				free(oldstring);
			}
    	return size;
    }
    else
    {
		*string = NULL;
		if(oldstring != NULL)
		{
			free(oldstring);
		}
			return -1;
		}
	}
	va_end(list);
}


int unescape_append(char **dest, char *src)
{
	size_t i;


	for(i = 0; i < strlen(src); ++i)
	{
		switch(src[i])
		{
			case '\"':
			case '\'':
				asprintf(dest, "%s\\%c", *dest, src[i]);
			break;
			default:
				asprintf(dest, "%s%c", *dest, src[i]);
		}
	}

	return 0;
}


char *space_string(char *space, size_t amount)
{
	size_t i;

	for(i = 0; i < amount; ++i)
	{
		space[i] = ' ';
	}

	return space;
}


int convert(char **str, ssize_t *size, json_t *json, size_t depth)
{
	json_error_t err;
	json_t *new_json = NULL;
	size_t i;
	json_int_t in;
	char space[2048] = {0};
	char *name = NULL;
	double real;


	if(!json)
	{
		printf("null object\n");
		return 1;
	}

	switch(json->type)
	{
		case JSON_OBJECT:
			asprintf(str, "%s%s[\n", *str, space_string(space, depth));
			json_object_foreach(json, name, new_json)
			{
				asprintf(str, "%s%s[\"%s\" ", *str, space_string(space, depth), name);

				if(convert(str, size, new_json, depth + 1))
				{
					printf("error converting object member %llu\n", i);
					return 1;
				}

				asprintf(str, "%s%s]\n", *str, space_string(space, depth));
			}
			asprintf(str, "%s%s]\n", *str, space_string(space, depth));
		break;
		case JSON_ARRAY:
			asprintf(str, "%s%s[", *str, space_string(space, depth));
			for(i = 0; i < json_array_size(json); ++i)
			{
				if(convert(str, size, json_array_get(json, i), depth + 1))
				{
					printf("error converting array member %llu\n", i);
					return 1;
				}
			}
			asprintf(str, "%s%s]\n", *str, space_string(space, depth));
		break;
		case JSON_STRING:
			asprintf(str, "%s\"", *str);
			unescape_append(str, json_string_value(json));
			asprintf(str, "%s\"", *str);
		break;
		case JSON_INTEGER:
			in = json_integer_value(json);

			asprintf(str, "%s'%llu'", *str, in);
		break;
		case JSON_REAL:
			real = json_real_value(json);

			asprintf(str, "%s'%f'", *str, real);
		break;
		case JSON_TRUE:
			asprintf(str, "%s TRUE", *str);
		break;
		case JSON_FALSE:
			asprintf(str, "%s FALSE", *str);
		break;
		case JSON_NULL:
			asprintf(str, "%s NULL", *str);
		break;
	}

	return 0;
}


int main(int argc, char **argv)
{
	FILE *output = NULL;
	json_t *json = NULL;
	json_error_t err;
	char *str = NULL;
	ssize_t str_len = 0;


	if(argc != 3)
	{
		printf("json2boing requires an output file argument and an input file argument\n");
		return 1;
	}


	if(!(json = json_load_file(argv[1], 0, &err)))
	{
		printf("could not parse file %s\n", argv[1]);
		return 1;
	}

	str = calloc(1, 1);

	if(convert(&str, &str_len, json, 0))
	{
		printf("could not convert file %s\n", argv[1]);
		return 1;
	}

	json_decref(json);


	if(!(output = fopen(argv[2], "w")))
	{
		printf("could not open output file %s\n", argv[2]);
		return 1;
	}


	fwrite(str, sizeof(char), strlen(str), output);


	fclose(output);


	return 0;
}
#include <string.h>
#include <k/io.h>


int puts(const char *s)
{
	return write(s, strlen(s));
}

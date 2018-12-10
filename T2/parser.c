#include "parser.h"

int getIp(urlInfo* infoStruct) {
	struct hostent* h;

	if ((h = gethostbyname(info_struct->host)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(info_struct->ip, ip);

	return 0;
}

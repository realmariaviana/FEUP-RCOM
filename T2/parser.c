#include "parser.h"

int parseUsernamePassword(urlInfo * infoStruct, char * completeUrl){
 	//ftp://[<user>:<password>@]<host>/<url-path>

  char * at = strrchr(completeUrl, '@');
  char* firstSlash = strchr(completeUrl, '/');
  firstSlash += 2; // pointing to the beggining of the username

  char* password = strchr(firstSlash, ':'); //password pointing to :
  if(password == NULL){
    fprintf(stderr, "Your link must contain a ':' separating the username and password!'\n");
    return 1;
  }

  memcpy(infoStruct->user, firstSlash, password - firstSlash); //password - slash it's the size of username in bytes
  infoStruct->user[password-firstSlash]=0;
  password++; // now pointing to the first character of the password

  memcpy(infoStruct->password, password, at - password);
  infoStruct->password[at-password] = 0; //string end character
  return 0;
}

int parseUrl(char completeUrl[], urlInfo * infoStruct){
	//see if it begins with ftp://
	if(strncmp(completeUrl, "ftp://", strlen("ftp://")) != 0){
    fprintf(stderr, "The link does not begin with 'ftp://'\n");
    return 1;
	}

  char* slashAfterHost;

  if(!strchr(completeUrl, '@')){ //if it doesnt find @ it's password and username are anonymous
    memcpy(infoStruct->user, "anonymous", strlen("anonymous") + 1);
    memcpy(infoStruct->password, "anonymous", strlen("anonymous") + 1);

    char * s1 = strchr(completeUrl,'/');
    s1++;
    s1++;

    slashAfterHost = strchr(s1, '/');
    memcpy(infoStruct->host, s1, slashAfterHost-s1);
    infoStruct->host[slashAfterHost-s1] = 0;
  }

  else{
    if(parseUsernamePassword(infoStruct,completeUrl)!=0)
      return 1;

      char * at = strrchr(completeUrl, '@');
      at++;

      slashAfterHost = strchr(at, '/');
      memcpy(infoStruct->host, at, slashAfterHost-at);
      infoStruct->host[slashAfterHost-at] = 0;
  }

  char* lastSlash = strrchr(completeUrl, '/');
  lastSlash++; //to point to the element after the slash
  memcpy(infoStruct->filePath, slashAfterHost, lastSlash-slashAfterHost);
  infoStruct->filePath[lastSlash-slashAfterHost] = 0;

  memcpy(infoStruct->fileName, lastSlash, strlen(lastSlash) + 1);

  getIp(infoStruct);

  return 0;
}

int getIp(urlInfo * infoStruct) {
	struct hostent* h;

	if ((h = gethostbyname(infoStruct->host)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	// printf("Host name  : %s\n", h->h_name);
	// printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(infoStruct->ip, ip);

	return 0;
}

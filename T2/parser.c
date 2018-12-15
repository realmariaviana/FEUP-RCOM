#include "parser.h"

int userPassword(urlInfo * infoStruct, char * completeUrl){
  char * at = strrchr(completeUrl, '@');
  char* first_slash = strchr(completeUrl, '/'); //slash is never null
  first_slash += 2; // first_slash * its increased by two to point to the beggining of the user name "//[username]"
  char* password = strchr(first_slash, ':');
  if(password == NULL){
    fprintf(stderr, "Your link must contain a ':' separating the username and password!'\n");
    return 1;
  }
  memcpy(infoStruct->user, first_slash, password - first_slash); //password - slash it's the size of username in bytes
  infoStruct->user[password-first_slash]=0;
  password++; //the password pointer was poiting to ":" and it has to point to the first character of the userPassword
  memcpy(infoStruct->password,password,at - password);
  infoStruct->password[at-password] = 0; //string end character
  return 0;
}

int parseUrl(char completeUrl[], urlInfo * infoStruct){
	if(strncmp(completeUrl, "ftp://", strlen("ftp://")) != 0){
    fprintf(stderr, "The link does not begin with 'ftp://'\n");
    return 1;
	}

  char* slash_after_host;

  if(!strchr(completeUrl, '@')){
    memcpy(infoStruct->user, "anonymous", strlen("anonymous") + 1);
    memcpy(infoStruct->password, "anonymous", strlen("anonymous") + 1);

    char * s1 = strchr(completeUrl,'/');
    s1++;
    s1++;

    slash_after_host = strchr(s1, '/');
    memcpy(infoStruct->host, s1, slash_after_host-s1);
    infoStruct->host[slash_after_host-s1] = 0;
  }

  else{
    if(userPassword(infoStruct,completeUrl)!=0)
      return 1;

      char * at = strrchr(completeUrl, '@');
      at++;

      slash_after_host = strchr(at, '/');
      memcpy(infoStruct->host, at, slash_after_host-at);
      infoStruct->host[slash_after_host-at] = 0;
  }

  char* last_slash = strrchr(completeUrl, '/');
  last_slash++; //to point to the element after the slash
  memcpy(infoStruct->filePath, slash_after_host, last_slash-slash_after_host);
  infoStruct->filePath[last_slash-slash_after_host] = 0;

  memcpy(infoStruct->fileName, last_slash, strlen(last_slash) + 1);

  getIp(infoStruct);

  return 0;
}

int getIp(urlInfo* infoStruct) {
	struct hostent* h;

	if ((h = gethostbyname(infoStruct->host)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(infoStruct->ip, ip);

	return 0;
}

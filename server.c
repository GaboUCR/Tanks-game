#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#define BACKLOG 10 //???? why is this needed

int get_listener(char domain[], char port[], struct addrinfo* hints)
{
  struct addrinfo *p, *res;
  int status, yes =1, sockfd =-1;

  if ((status = getaddrinfo(domain, port, hints, &res)) != 0)
  {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
      return -1;
  }
  for(p = res; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      {
        perror("server: socket");
        continue;
      }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    break;
    }
    freeaddrinfo(res);
    return sockfd;
}



int main() {
  int serverfd;
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  serverfd = get_listener(NULL, "6000", &hints);

  if (serverfd == -1)
  {
    fprintf(stderr, "server: failed to create");
    exit(1);
  }
  printf("Listener on port %d \nWaiting for connections ...", 6000);
  printf("%d\n", serverfd);




  }

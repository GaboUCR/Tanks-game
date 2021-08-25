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
#define PORT "6000"

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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

void add_fd(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_alloc_size)
{
  if (*fd_count == *fd_alloc_size)
  {
    *fd_alloc_size *= 2;
    *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_alloc_size));
  }

  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN;
  *fd_count += 1;
}

void del_fd(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}




int main() {
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  char buf[256];
  char remoteIP[INET6_ADDRSTRLEN];

  int serverfd, newfd;
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int fd_count = 0;
  int fd_alloc_size = 5;
  struct pollfd *pfds = malloc(sizeof *pfds * fd_alloc_size);

  serverfd = get_listener(NULL, PORT, &hints);

  if (serverfd == -1)
  {
    fprintf(stderr, "server: failed to create");
    exit(1);
  }
  printf("Listener on port %d \nWaiting for connections ...", 6000);

  add_fd(&pfds, serverfd, &fd_count, &fd_alloc_size);

  while (1)
  {
    int poll_count = poll(pfds, fd_count, -1);

    if (poll_count == -1)
    {
        perror("poll");
        exit(1);
    }

    for (int i=0; i<fd_count; i++)
    {
      if (pfds[i].revents & POLLIN)
      {
        if (pfds[i].fd == serverfd)
        {
          addrlen = sizeof remoteaddr;
          newfd = accept(serverfd,(struct sockaddr *)&remoteaddr,&addrlen);

          if (newfd == -1) {
              perror("accept");
          }
          else
          {
            add_fd(&pfds, newfd, &fd_count, &fd_alloc_size);

            printf("pollserver: new connection from %s on "
                "socket %d\n",inet_ntop(remoteaddr.ss_family,
                    get_in_addr((struct sockaddr*)&remoteaddr),
                    remoteIP, INET6_ADDRSTRLEN),newfd);
          }
        }//handling new connections
      else
      {
        int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

        int sender_fd = pfds[i].fd;

        if (nbytes <= 0)
        {
            // Got error or connection closed by client
            if (nbytes == 0) {
                // Connection closed
                printf("pollserver: socket %d hung up\n", sender_fd);
            }
            else
            {
                perror("recv");
            }

            close(pfds[i].fd);
            del_fd(pfds, i, &fd_count);
      }
      else //client send a req
      {
        send(pfds[i].fd, "hello mate", 10, 0);
        printf("%s\n", buf);
      }


      }//check fds that are ready for reading
    }
  }

  }//infinite loop
}

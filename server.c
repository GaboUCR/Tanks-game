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
#define width 1250
#define height 650
#define bufer_size 10
#define max_players 9

struct player {
  int id;
  int x;
  int y;
};

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

void del_player(struct player players[], int fd, int *n_players)
{
  for (int i=0; i< *n_players; i++)
  {
    if (fd == players[i].id)
    {
      players[i].id = players[*n_players - 1].id;
      players[*n_players - 1].id = 0;
      (*n_players)--;
      break;
    }
  }
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

void handle_message(struct pollfd pfds[], struct player players[], int *n_players,
                    int index, int *fd_count)
{
  char buf[bufer_size];
  memset(buf, 0, bufer_size);
  int nbytes = recv(pfds[index].fd, buf, bufer_size, 0);
  printf("%s\n", buf);
  if (nbytes <= 0)
  {
    // Got error or connection closed by client
    if (nbytes == 0)
    {
      // Connection closed
      printf("pollserver: socket %d hung up\n", pfds[index].fd);
    }
    else
    {
      perror("recv");
    }

    close(pfds[index].fd);
    del_fd(pfds, index, fd_count);
    del_player(players, pfds[index].fd, n_players);
  }
  else //client send a req
  {
    for (int i=0; i<*n_players; i++)
    {
      send(players[i].id, buf, bufer_size, 0);
    }
  }

}

void add_player(struct player players[], int n_players, int id)
//alerts every user that a new player entered the game
{
  char buf[2];
  sprintf(buf, "%d", id);
  printf("%s\n", buf);
  for (int i=0; i<n_players; i++)
  {
    send(players[i].id, buf, 1, 0);
  }
}

void main() {
  struct player players[max_players];
  int n_players = 0;
  //client info
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  char buf[bufer_size];
  char remoteIP[INET6_ADDRSTRLEN];

  //Server info
  int serverfd, newfd;
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  //polling initiation
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
          newfd = accept(serverfd, (struct sockaddr *)&remoteaddr, &addrlen);

          if (newfd == -1)
          {
              perror("accept");
          }
          else
          {
            add_fd(&pfds, newfd, &fd_count, &fd_alloc_size);

            players[n_players].id = newfd;
            n_players++;
            printf("new id %d\n", newfd);
            add_player(players, n_players, newfd);

            printf("pollserver: new connection from %s on "
                "socket %d\n",inet_ntop(remoteaddr.ss_family,
                    get_in_addr((struct sockaddr*)&remoteaddr),
                    remoteIP, INET6_ADDRSTRLEN),newfd);
          }
        }//handling new connections
      else
      {
        //reads the message and sends the message to every client
        handle_message(pfds, players, &n_players, i, &fd_count);
      }
    }
  }
  }//infinite loop
}

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}


int main(int argc,char *argv[]){
// variables
int listen_socket;struct sockaddr_in server; int mysock;
char buff[1024]; int rval;
//create sockets
listen_socket= socket(AF_INET,SOCK_STREAM,0);
if(listen_socket<0){
	perror("socket creation error");
	exit(-1);
}
// allowing socket descriptor to be reusable
int on=1;
int rc= setsockopt(listen_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
if(rc<0){
	perror("setsockopt() failed");
	close(listen_socket);
	exit(-1);
}
//set the socket to be nonblocking
rc=fcntl(listen_socket, F_SETFL, O_NONBLOCK);
if(rc<0){
	perror("ioctl() failed");
	close(listen_socket);
	exit(-1);
}

server.sin_family=AF_INET;
server.sin_addr.s_addr=INADDR_ANY;
server.sin_port=htons(5000);

//call bind
if(bind(listen_socket,(struct sockaddr *)&server,sizeof(server))<0){
	perror("bind failed");
	close(listen_socket);
	exit(-1);

}
//listen
rc=listen(listen_socket,20);
if(rc<0){
	perror("ioctl() failed");
	close(listen_socket);
	exit(-1);
}
//set the pollfd structure
struct pollfd fds[200];
int nfds=1;//no. of elements in poll
memset(fds,0,sizeof(fds));
//setting the listen socket to be the first one in pollfd structure
fds[0].fd=listen_socket;
fds[0].events=POLLIN;
//timeout variable for poll()
int timeout=-1; //infinite time

//loop waiting for incoming for incoming sockets
int server_ok=1; //controls the server
int new_socket; //for storing new sockets
do{
	printf("Waiting for poll()...\n");
	rc=poll(fds,nfds,timeout);
	if(rc<0){
	perror("poll() failed");
	break;
	}
// checking for timeout even if there is no time out
	if(rc==0){
	perror("poll() timed out");
	break;
	}

//now we have many socket-descriptors which are readable.
//Loop through all sockets to check for them by 
	//checking which returned POLLIN
	int i=0,current_size=nfds;
	int compress_array=1;
	for(i=0;i<current_size;i++){

		if(fds[i].revents == 0)
			continue;

		//if revents is neither 0 nor POLLIN then unexpected error and 
		//close server
		if(fds[i].revents != POLLIN){
			printf("Unexpected result with revent=%d\n",fds[i].revents);
			server_ok=0;
			break;
		}

		//now checking whether this readable socket is listener or not
		if(fds[i].fd==listen_socket){
			//if listener socket is readable than that means there are 
			//connections pending on it and we have to accept it.
			printf("New connections pending:Listening socket readable\n");
			//accepting all incoming connections that are queued up on the listening socket
			do{
				new_socket=accept(listen_socket,NULL,NULL);
				if(new_socket<0){
					if(errno != EWOULDBLOCK){
						perror("accept() failed");
						server_ok=0;
					}
					break;
				}

				printf("new connection-%d\n",new_socket);
				fds[nfds].fd=new_socket;
				fds[nfds].events=POLLIN;
				nfds++;

			} while(new_socket!=-1);//if new_socket =-1 then no more socket connections available
		} else{//this is not the listening socket,so data is incoming on this socket
			printf("Socket %d is readable",fds[i].fd);
			//close_conn=false;
			//receiving all the data on this socket.
			rc = recv(fds[i].fd,buff,sizeof(buff),0);
			if(rc<0)
			{
				perror("recv() failed");
			}

			if(rc==0){
				printf("connection closed\n");
				close(fds[i].fd);
				fds[i].fd=-1;
				//compresssing arrray since some socket is deleted
				compress_array=0;

			}
			else{
				printf("Received data%d\n", rc);
				//forward received data to other non-listening sockets
					//IMPORTANT
					int j;
					for(j=0;j<=nfds;j++){//send to others
						if(fds[j].fd != -1 && j!=0 && j!=i){
							if(sendall(fds[j].fd,buff,&rc)<0) perror("data forward fail");
						}
					}
			}

		}
		

	}

	if(compress_array){
		for(i=0;i<nfds;i++){
			if(fds[i].fd==-1){
				int j;
				for(j=i;j<nfds;j++){
					fds[j].fd=fds[j+1].fd;
				}
				i--;
				nfds--;
			}
		}
	}


}while(server_ok);

//cleaning up
int i;
for(i=0;i < nfds;i++){
	if(fds[i].fd>=0) close(fds[i].fd);
}

return 0;
}
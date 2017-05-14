#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define DATA "Hello world of socket"

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

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}



int main(int argc,char *argv[]){
	int sock;int mysock;
	struct sockaddr_in server;
	int rval;
	struct hostent *hp;
	char buff[1024];

	sock =socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		perror("socket creation failed");
		exit(1);
	}
	server.sin_family=AF_INET;

	hp= gethostbyname(argv[1]);
	if(hp==0){
		perror("gethostbyname failed");
		close(sock);
		exit(1);
	}

	memcpy(&server.sin_addr,hp->h_addr,hp->h_length);
	server.sin_port= htons(5000);
	/*
		//to make the client listen for messages from server.we will attach it to a specific port.
	struct sockaddr_in client;
	client.sin_family=AF_INET;
	client.sin_addr.s_addr=INADDR_ANY;
	client.sin_port=htons(2000);

	if(bind(sock,(struct sockaddr *)&server,sizeof(server))<0){
	perror("bind failed");
	exit(1);
	}
	printf("%s\n",(char*)client.sin_addr.s_addr);
	//sendMessage(sock,server,"");
	*/

if(connect(sock,(struct sockaddr *) &server,sizeof(server))<0){
		perror("conncet failed");
		close(sock);
		exit(1);
	}else printf("ok\n");
	//work here
	fcntl(sock, F_SETFL, O_NONBLOCK);


	char s[1024];
	printf("Enter your name(max 20 alphabets)");
	scanf ("%[^\n]%*c", s);
	char name[20];
	memcpy(name,s,20);
	

	strcat(s," just joined the chat.");
	int temp=sizeof(s);
	if(sendall(sock,s,&temp)<0){   
		perror("send failed");
		close(sock);
		exit(1);return -1;
	}else{
		memset(s,0,sizeof(s));
		memcpy(s,name,20);
		strcat(s,":");
	}

	memset(buff,0,sizeof(buff));

do{	
	//if user pressess the key i then it will enter into input mode
	
	if(kbhit()){
		if(getchar()=='i'){
		printf("Please enter the message\n");
		
		scanf ("%[^\n]%*c", buff);

		strcat(s,buff);

		temp=sizeof s;
		if(sendall(sock,s,&temp)<0){   
		perror("send failed");
		close(sock);
		exit(1);
	} else {
	memset(s,0,sizeof(s));
		memcpy(s,name,20);
		memset(buff,0,sizeof(buff));
		strcat(s,":");
	}
	
		}
		}


	///////////////////////////data receiver
	
	if((rval = recv(sock,buff,sizeof(buff),0))>0)

		{printf("%s\n",buff);
		
		}
	/////////////////////////////////////////////////////
	} while(1);

	

	return 0;
}


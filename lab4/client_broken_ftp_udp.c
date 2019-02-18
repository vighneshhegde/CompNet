#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define PORT 5001

void die(char *s)
{
    perror(s);
    exit(1);
}


int main(void)
{
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[256];
    unsigned char buff_offset[10];   // buffer to send the File offset value
    unsigned char buff_command[2];   // buffer to send the Complete File (0) or Partial File Command (1).
    int offset;                      // required to get the user input for offset in case of partial file command
    int command;                     // required to get the user input for command
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;
    int slen = sizeof(serv_addr);

    /* Create a socket first */
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // port
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


    /* Create file where data will be stored */
        FILE *fp;
        fp = fopen("destination_file.txt", "ab");
        if(NULL == fp)
        {
          printf("Error opening file");
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        offset = ftell(fp);
        fclose(fp);
        fp = fopen("destination_file.txt", "ab");
        if(NULL == fp)
        {
          printf("Error opening file");
            return 1;
        }

    printf("Enter (0) to get complete file, (1) to specify offset\n");
    scanf("%d", &command);
    sprintf(buff_command, "%d", command);
    write(sockfd, buff_command, 2);
    if (sendto(sockfd, buff_command, strlen(buff_command) , 0 , (struct sockaddr *) &serv_addr, slen)==-1)
    {
        die("sendto()");
    }

    if(command == 1 || command == 2)   // We need to specify the offset
    {

        if(command == 1)  // get the offset from the user
        {
        printf("Enter the value of File offset\n");
        scanf("%d", &offset);
        }
        // otherwise offset = size of local partial file, that we have already calculated
        sprintf(buff_offset, "%d", offset);
        /* sending the value of file offset */
        if (sendto(sockfd, buff_command, strlen(buff_offset) , 0 , (struct sockaddr *) &serv_addr, slen)==-1)
        {
            die("sendto()");
        }
    }

    // Else { command = 0 then no need to send the value of offset }


    /* Receive data in chunks of 256 bytes */
    while(1)
    {
      if ((bytesReceived=recvfrom(sockfd, recvBuff, 256, 0, (struct sockaddr *) &serv_addr, &slen)) < 0)
      {
          if (bytesReceived==-1) die("recvfrom()");
          break;
      }
      if (bytesReceived==1 && recvBuff[0]=='\0') break;
        printf("Bytes received %d\n",bytesReceived);
        // recvBuff[n] = 0;
        fwrite(recvBuff, 1,bytesReceived,fp);
        // printf("%s \n", recvBuff);
    }
    fclose(fp);
    return 0;
}

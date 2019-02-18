#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    int listenfd = 0;
    int s = 0;
    struct sockaddr_in serv_addr,cl_addr;
    int slen = sizeof(cl_addr),recv_len;
    char sendBuff[1025];
    int numrv;

    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5001);

    if( bind(s , (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) == -1)
    {
        die("bind");
    }
    // if(listen(listenfd, 10) == -1)
    // {
    //     printf("Failed to listen\n");
    //     return -1;
    // }


    while(1)
    {
        printf("Waiting for a client to send the command (Full File (0) Partial File (1)\n");
        unsigned char offset_buffer[10] = {'\0'};
        unsigned char command_buffer[2] = {'\0'};
        int offset;
        int command;
        //connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
        if ((recv_len = recvfrom(s, command_buffer, 2, 0, (struct sockaddr *) &cl_addr,&slen)) == -1)
        {
            die("recvfrom()");
        }

        command = atoi(command_buffer);

        if(command == 0)
                offset = 0;
        else
        {
                printf("Waiting for the client to send the offset\n");
                if ((recv_len = recvfrom(s, offset_buffer, 10, 0, (struct sockaddr *) &cl_addr,&slen)) == -1)
                {
                    die("recvfrom()");
                }

                offset = atoi(offset_buffer);

        }


        /* Open the file that we wish to transfer */
        FILE *fp = fopen("source_file.txt","rb");
        if(fp==NULL)
        {
            printf("File open error");
            return 1;
        }

        /* Read data from file and send it */
                 fseek(fp, offset, SEEK_SET);
        while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[256]={0};
            int nread = fread(buff,1,256,fp);
            printf("Bytes read %d \n", nread);

            /* If read was success, send data. */
            if(nread > 0)
            {
                printf("Sending \n");
                //write(connfd, buff, nread);
                if (sendto(s, buff, nread , 0 , (struct sockaddr *) &cl_addr, slen)==-1)
                {
                    die("sendto()");
                }
            }

            /*
             * There is something tricky going on with read ..
             * Either there was error, or we reached end of file.
             */
            if (nread < 256)
            {
                if (feof(fp)){
                    printf("End of file\n");
                    if (sendto(s, "\0", 1 , 0 , (struct sockaddr *) &cl_addr, slen)==-1)
                    {
                        die("sendto()");
                    }
                }
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }

        //close(s);
        fclose(fp);
        sleep(1);
    }


    return 0;
}

/*
    Simple udp server
*/

#include<stdio.h>		//printf
#include<string.h>		//memset
#include<stdlib.h>		//exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>		//close()
#include<time.h>

#define BUFLEN 512		//Max length of buffer
#define PORT 8882		//The port on which to listen for incoming data

void
die (char *s)
{
  perror (s);
  exit (1);
}

typedef struct packet1
{
  int sq_no;
} ACK_PKT;

typedef struct packet2
{
  int sq_no;
  char data[BUFLEN];
} DATA_PKT;

int
main (void)
{
  struct sockaddr_in si_me, si_other;
  srand (time (NULL));

  int s, i, slen = sizeof (si_other), recv_len;
  int FLAG = 1;
  DATA_PKT rcv_pkt;
  ACK_PKT ack_pkt;

  //create a UDP socket
  if ((s = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
      die ("socket");
    }

  // zero out the structure
  memset ((char *) &si_me, 0, sizeof (si_me));

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons (PORT);
  si_me.sin_addr.s_addr = htonl (INADDR_ANY);

  //bind socket to port
  if (bind (s, (struct sockaddr *) &si_me, sizeof (si_me)) == -1)
    {
      die ("bind");
    }

  int state = 0;
  while (1)
    {

      switch (state)
	{
	case 0:
	  {
	    printf ("Waiting for packet 0 from sender...\n");
	    fflush (stdout);

	    //try to receive some data, this is a blocking call
	    while (1)
	      {
		if ((recv_len =
		     recvfrom (s, &rcv_pkt, BUFLEN, 0,
			       (struct sockaddr *) &si_other, &slen)) == -1)
		  {
		    die ("recvfrom()");
		  }
		if (rand () % 2 == 1)
		  break;	//else discard and continue
		printf ("fake discard.\n");
	      }
	    if (rcv_pkt.sq_no == 0)
	      {
		printf
		  ("Packet received with seq. no. %d. Packet content is:\n %s\n",
		   rcv_pkt.sq_no, rcv_pkt.data);
		ack_pkt.sq_no = 0;
		if (sendto
		    (s, &ack_pkt, recv_len, 0, (struct sockaddr *) &si_other,
		     slen) == -1)
		  {
		    die ("sendto()");
		  }
		state = 1;
		break;
	      }
	    else if (rcv_pkt.sq_no == 1)
	      {
		printf
		  ("Packet re-received with seq. no. %d.", rcv_pkt.sq_no);
		ack_pkt.sq_no = 1;
		if (sendto
		    (s, &ack_pkt, recv_len, 0, (struct sockaddr *) &si_other,
		     slen) == -1)
		  {
		    die ("sendto()");
		  }
		break;

	      }

	  }
	case 1:
	  {
	    printf ("Waiting for packet 1 from sender...\n");
	    fflush (stdout);

	    //try to receive some data, this is a blocking call
	    while (1)
	      {
		if ((recv_len =
		     recvfrom (s, &rcv_pkt, BUFLEN, 0,
			       (struct sockaddr *) &si_other, &slen)) == -1)
		  {
		    die ("recvfrom()");
		  }
		if (rand () % 2 == 0)
		  break;	//else discard and continue
		printf ("fake discard.\n");
	      }
	    if (rcv_pkt.sq_no == 1)
	      {
		printf
		  ("Packet received with seq. no. %d. Packet content is:\n %s\n",
		   rcv_pkt.sq_no, rcv_pkt.data);
		ack_pkt.sq_no = 1;
		if (sendto
		    (s, &ack_pkt, recv_len, 0, (struct sockaddr *) &si_other,
		     slen) == -1)
		  {
		    die ("sendto()");
		  }
		state = 0;
		break;
	      }
	    else if (rcv_pkt.sq_no == 0)
	      {
		printf
		  ("Packet re-received with seq. no. %d.\n", rcv_pkt.sq_no);
		ack_pkt.sq_no = 0;
		if (sendto
		    (s, &ack_pkt, recv_len, 0, (struct sockaddr *) &si_other,
		     slen) == -1)
		  {
		    die ("sendto()");
		  }
		break;

	      }

	  }
	}

    }

  close (s);
  return 0;
}

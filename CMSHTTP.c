USERID GCCCMS

/* version 1 */
 
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "nicofclt.h"
#include "socket.h"

#define SRV_LISTEN_ADDR "0.0.0.0"
#define PORT            7999
#define SERVER_STRING   "Server: VM370 CMS\n"
#define MAX_WORK        512

   FILE *f_in;
    int len, w_get;
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int retConn, accConn, sendConn;
    char in_buf[512];
    char hello[2024];
    char body[1500];
    char work[MAX_WORK];
    char buf1[3];
    char out_page[2048];
    char hdr[2048];
    char wk_lgth[5];
     int body_lgth, hdr_len, body_len;
    char ch, file_nm[32], rq_type[5];
     int pi, si, no_hello, flag;
    char *p;
 
void accept_request(void);
void send_webpage(void);

int main(int argc, char const *argv[])
{

    nicofclt_init();
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        perror("In socket");
        exit(0);
    }
   
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SRV_LISTEN_ADDR);
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
     
    retConn = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    if(retConn < 0)
    {
        perror("In bind");
        exit(0);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(0);
    }

    printf("\n+++++++ Waiting for new connection ++++++++\n");

    while(1)
    {
        new_socket = accept(server_fd, (struct sockaddr *)&address,
                                   (socklen_t*)&addrlen);
        if(new_socket < 0)
        {
            perror("In accept");
            exit(0);
        }
        
        valread = recv(new_socket, in_buf, MAX_WORK, 0);

        accept_request();

        no_hello = 0;

        send_webpage();
     
        hdr_len = strlen(hdr);
        memcpy(out_page, hdr, hdr_len); 
        send(new_socket , hdr , hdr_len, 0);

        if(no_hello == 1)
        {
           body_len = strlen(hello);
           memcpy(out_page, hello, body_len); 
           out_page[body_len] = (char)0x0D;
           out_page[body_len + 1] = (char)0x0A;
           send(new_socket , hello , body_len, 0);
        }
        closesocket(new_socket);
    }
    fclose(f_in);
    nicofclt_deinit();
    return 0;
}


/* *******************************************************************
 * Process the request from the web browser
 ****************************************************************** */
void accept_request()
{
   len = strlen(in_buf);
   nicofclt_ascii2ebcdic(in_buf, len, in_buf);
   in_buf[len] = '\0';
/*   strncpy(buf1,in_buf,49); */
printf("in_buf = %s\n",in_buf);
}


/* *******************************************************************
 * Process the requested web page
 ****************************************************************** */
void send_webpage()
{
    w_get = 0;
    strncpy(buf1,in_buf,3);
printf("buf1 = %s\n",buf1);

    if(strcmp(buf1, "GET") == 0)		/* GET request */
    {
      w_get = 1;
    }

    if(strcmp(buf1, "POS") == 0)		/* POST not supportted */
    {
      notfd_page();
      return;
    }

    p = strstr(in_buf, "/portal/redline");	/* bot reject request */
    if(p)
    {
      notfd_page();
      return;
    }

    if(w_get != 1)
    {
      notfd_page();
      return;
    } 


    /* get page request type */
    pi = 0;
    si = 0;
    ch = in_buf[pi];
    while(ch != ' ')
    {
      rq_type[si] = ch;
      pi++;
      si++;
      ch = in_buf[pi];
    }
    rq_type[si] = '\0';
 
    /* get file_nm and file type */
    pi++;
    pi++;
    si = 0;
    ch = in_buf[pi];
    while(ch != ' ')
    {
      file_nm[si] = ch;
      pi++;
      si++;
      ch = in_buf[pi];
    }
    file_nm[si] = '\0';
printf("si = %d file_nm = %s\n",si,file_nm);
 

/* *******************************************************************
 * Inform the client that a request to favicon.ico returns 404 error.
 ****************************************************************** */
    if(strcmp(file_nm, "favicon.ico") == 0)
    {
      strcpy(hdr, "HTTP 1.1 404 NOT FOUND\r");
      strcat(hdr, "Content-type: text/html\r");
      strcat(hdr, SERVER_STRING);
      hdr_len = strlen(hdr);
      nicofclt_ebcdic2ascii(hdr, hdr_len, hdr); 
      hdr[4] = (char)0x2F;	/* insert slash BUG */
      hdr_len = strlen(hdr);
      hdr[hdr_len] = (char)0x0D;
      hdr[hdr_len + 1] = (char)0x0A;
      hdr[hdr_len + 2] = '\0';


/* *******************************************************************
 * This sends a blank line to the browser indicating the end fo 
 * the header and the start of the body of the html page.
 ****************************************************************** */
      work[0] = (char)0x0D;
      work[1] = (char)0x0A;
      work[2] = '\0';
      strcat(hdr, work);
      no_hello = 0;
      return;
    }


/* *******************************************************************
 * Start of the processing of the requested web page. 
 ****************************************************************** */
   no_hello = 1;

/* *******************************************************************
 * Test si.  If si == 0 then no webpage name was sent.  Defaults
 * to index.html or index.htm else test for requested page
 ****************************************************************** */
   if(si == 0)
   {
      strcpy(file_nm, "index.html");
      f_in = fopen(file_nm,"r");
      if(f_in == NULL)
      {
         strcpy(file_nm, "index.htm");
         f_in = fopen(file_nm,"r");
         if(f_in == NULL)
         {
            notfd_page();
            return;
         }
      }
      fclose(f_in);
   }
   else
   {
      f_in = fopen(file_nm,"r");
      if(f_in == NULL)
      {
         notfd_page();
         return;
      }
      fclose(f_in);
   }



/* *******************************************************************
 * Read in from directory the selected web page, as identified in
 * the variable file_nm.
 ****************************************************************** */
    flag = 0;

    f_in = fopen(file_nm,"r");
    if(f_in == NULL)
    {
       notfd_page();
       return;
    }

    while(fgets(work, MAX_WORK, f_in) != NULL)
    {
       len = strlen(work);

/* *******************************************************************
 * Test for link href includes of a seperate file.
 *  <link href="/style.css" media="all" rel="Stylesheet" type="text/css" />
 ****************************************************************** */


 /* need to code */



/* *******************************************************************
 * Test for script src includes of a seperate file.
 *  <script src="/generateTree.js"></script>
 ****************************************************************** */
/*
       skip_flag = 0;
       script_flag = 0;
       p = strstr(work, "<script")
       if(p)
       { 
          p2 = strstr(work, "</script>");
          p1 = strstr(work, "scr");
          if(p1)
          {
             skip_flag = 0;
             ii = 0;
             ch = work[ii];
             while(ch != '\"')
             {
                ii++;
                ch = work[ii];
             }
             pi = 0;
             ii++;
             ch = work[ii];
             while(ch != '\"')
             {
                if(ch != '\/')
                {
                   file_inc[pi] = ch;
                   pi++;
                }
                ii++;
                ch = work[ii];
             }
             file_inc[pi] = '\0';
 printf("file_inc = %s\n",file_inc);
             strcpy(work, "<script>");
printf("file_inc work = %s\n",work);
             len = strlen(work);
             nicofclt_ebcdic2ascii(work, len, work);
             work[len] = (char)0x0D;
             work[len + 1] = (char)0x0A;
             work[len + 2] = '\0';
             strcat(hello, work);

       
             f_inc = fopen(file_inc,"r");
             if(f_inc == NULL)
             {
                notfd_page();
                return;
             }

printf("AFTER file_inc OPEN\n");
             while(fgets(work, MAX_LEN, f_inc) != NULL)
             {
printf("file_inc work = %s\n",work);
                len = strlen(work);
                nicofclt_ebcdic2ascii(work, len, work);
                work[len] = (char)0x0D;
                work[len + 1] = (char)0x0A;
                work[len + 2] = '\0';
                strcat(hello, work);
                skip_flag = 1;
             }
             fclose(f_inc);
             
             if(p2)
             {
                strcpy(work, "</script>");
printf("file_inc work = %s\n",work);
                len = strlen(work);
                nicofclt_ebcdic2ascii(work, len, work);
                work[len] = (char)0x0D;
                work[len + 1] = (char)0x0A;
                work[len + 2] = '\0';
                strcat(hello, work);
                script_flag = 1;
             }
           }
         }
*/



/* *******************************************************************
 * Continue web page processing.
 ****************************************************************** */
          len = strlen(work);
          nicofclt_ebcdic2ascii(work, len, work);
          if(flag == 0)		/* 1st record */
          {
            work[len] = (char)0x0D;
            work[len + 1] = (char)0x0A;
            work[len + 2] = '\0';
            strcpy(hello, work);
            flag = 1;
          }
          else			/* remaining records */
          {
            work[len] = (char)0x0D;
            work[len + 1] = (char)0x0A;
            work[len + 2] = '\0';
            strcat(hello, work);
            flag = 1;
          }
        }
      
       

/* *******************************************************************
 * Builds header records to be sent to the browser.
 ****************************************************************** */
        strcpy(work, "HTTP 1.1 200 OK");
        len = strlen(work);
        nicofclt_ebcdic2ascii(work, len, work); 
        work[4] = (char)0x2F;
        work[len] = (char)0x0D;
        work[len + 1] = (char)0x0A;
        work[len + 2] = '\0';
        strcpy(hdr, work);

        strcpy(work, "Server: VM370 CMS");
        len = strlen(work);
        nicofclt_ebcdic2ascii(work, len, work); 
        work[len] = (char)0x0D;
        work[len + 1] = (char)0x0A;
        work[len + 2] = '\0';
        strcat(hdr, work);

        strcpy(work, "Content-type: text/html");
        len = strlen(work);
        nicofclt_ebcdic2ascii(work, len, work); 
        work[len] = (char)0x0D;
        work[len + 1] = (char)0x0A;
        work[len + 2] = '\0';
        strcat(hdr, work);


/* *******************************************************************
 * This sends a blank line to the browser indicating the end fo 
 * the header and the start of the body of the html page.
 ****************************************************************** */
        work[0] = (char)0x0D;
        work[1] = (char)0x0A;
        work[2] = '\0';
        strcat(hdr, work);
        len = strlen(work);
        hdr_len = strlen(hdr);
}


/* *******************************************************************
 * This sends a 404 notfound page for an web page request but in
 * not found in the directory.
 ****************************************************************** */
void notfd_page()
{
        strcpy(hdr, "HTTP 1.1 404 NOT FOUND\r");
        strcat(hdr, "Content-type: text/html\r");
        strcat(hdr, SERVER_STRING);
        hdr_len = strlen(hdr);
        nicofclt_ebcdic2ascii(hdr, hdr_len, hdr);
        hdr[4] = (char)0x2F;    /* insert slash BUG */
        hdr_len = strlen(hdr);
        hdr[hdr_len] = (char)0x0D;
        hdr[hdr_len + 1] = (char)0x0A;
        hdr[hdr_len + 2] = '\0';


/* *******************************************************************
 * This sends a blank line to the browser indicating the end fo
 * the header and the start of the body of the html page.
 ****************************************************************** */
        work[0] = (char)0x0D;
        work[1] = (char)0x0A;
        work[2] = '\0';
        strcat(hdr, work);
 
        no_hello = 0;
}


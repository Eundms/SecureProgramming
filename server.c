#include "common.h"
#define CERTFILE "server.pem"

int clientnum=0;
SSL *clients[10];
THREAD_TYPE *tid;
pthread_mutex_t mutex;
FILE *fp;

SSL_CTX *setup_server_ctx(void)
{
    SSL_CTX *ctx;

    ctx = SSL_CTX_new(SSLv23_method( ));
    if (SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
        int_error("Error loading certificate from file");
    if (SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
        int_error("Error loading private key from file");
    return ctx;
}

int do_server_loop(SSL *ssl)
{
    int err, nread,nwritten;
    char buf[80];
    int readfromwho;
    FILE *fp;
    char command[30] = {0};
    char * path = "client_";

    do
    {
        for(int i=0;i<clientnum;i++)
        {
            if(pthread_self()==tid[i])
                readfromwho=i;
            
        }

        for (nread=0; nread < sizeof(buf); nread += err)
        {
            err = SSL_read(ssl, buf + nread, sizeof(buf) - nread);
            if (err <= 0)
                break;
        }
    

        /*반드시 이곳에서 lock을 걸고*/
        fprintf(stdout,"Server read from client[%d]:",readfromwho);
        pthread_mutex_lock(&mutex);
        fprintf(stdout, "=======mutex_lock(%d)======\n",readfromwho);

        /*읽은 내용 서버에 보여준다*/
        fprintf(stdout, "%s", buf);

        /*SSLCLOSE요청*/    //close 했을 때 client 번호 줄어들어야 한다 (나중에 새로 client 들어오면 번호 새로 줄 수 있도록)
        if(strcmp(buf,"CLOSE\n")==0)
        {
            fprintf(stdout,"SSL CLOSE request from client[%d]\n",readfromwho);
            fprintf(stdout, "=======mutex_unlock(%d)======\n",readfromwho);
            memset(buf,0,sizeof(buf));
            pthread_mutex_unlock(&mutex);
            
            break;
        }
        /*download요청-이부분을 client에서 받기 위해 client thread를 하나 만들어야 한다*/
        if(strcmp(buf,"DOWNLOAD\n")==0)
        {
            char tmp[5]={0};
            strcat(command, "mkdir ");
            sprintf(tmp,"%d",readfromwho);
			strcat(command,path);
            strcat(command,tmp);
            system(command);    //각 client 번호에 맞춰 폴더 만들기
            printf("%s \n", command);

            memset(command,0,sizeof(command));

            strcpy(command,"cp ");
            strcat(command,"test.jpg ");
            strcat(command,path);
            strcat(command,tmp);
            system(command);

            memset(command,0,sizeof(command));
            
            strcat(command,"./");
            strcat(command,path);
            strcat(command,tmp);
            strcat(command,"/test.jpg");

            fprintf(stdout,"download request from client[%d]\n",readfromwho);
           // fp=fopen("./downloadfile.txt","r"); //downloadfile에는 파일 path가 들어있다 
            char readbuf[80];
           // fgets(readbuf,sizeof(readbuf),fp);
           strcpy(readbuf,command);
            for(nwritten=0; nwritten<sizeof(readbuf); nwritten+=err)
		    {
                err = SSL_write(clients[readfromwho], readbuf+nwritten, sizeof(readbuf)-nwritten);
                
			if(err<=0)
                return 0;
		    }
        }

        /*write 기능 처리*/
        if(strcmp(buf,"WRITE\n")==0){
            fprintf(stdout,"write request from client[%d]\n",readfromwho);
            fp=fopen("./writefile.txt","a+w");
            printf("client[%d]가 파일에 쓰는 중...\n",readfromwho); //비교: fprintf(stdout,"client[%d]가 파일에 쓰는 중...\n",readfromwho);
            
            while(1){

                for (nread=0; nread < sizeof(buf); nread += err){
                    err = SSL_read(ssl, buf + nread, sizeof(buf) - nread);
                    if (err <= 0)
                        break;
                }
                if(strcmp(buf,"DONE\n")==0)
                {
                    printf("client [%d] 쓰기 완료 \n",readfromwho);
                    fclose(fp);
                    break;
                }
                fprintf(stdout,": %s",buf);
                fprintf(fp,"client [%d] : %s",readfromwho,buf);

            }
        }
        
        fprintf(stdout, "=======mutex_unlock(%d)======\n\n",readfromwho);
       
        /*buffer를 지워야 한다*/
        memset(buf, 0, sizeof(buf));
        
        /*반드시 이곳에서 unlock을 해야한다*/
        pthread_mutex_unlock(&mutex);



    } 
    while (err > 0);
    return (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN) ? 1 : 0;
}
void THREAD_CC server_thread(void *arg)
{
    SSL *ssl = (SSL *)arg;

    #ifndef WIN32
        pthread_detach(pthread_self( ));
    #endif 
        if (SSL_accept(ssl) <= 0)
            int_error("Error accepting SSL connection");
        fprintf(stderr, "SSL Connection opened.\n");
        if (do_server_loop(ssl))
            SSL_shutdown(ssl);
        else
            SSL_clear(ssl);
        fprintf(stderr, "SSL Connection closed.\n");
        SSL_free(ssl);

        ERR_remove_state(0);

    #ifdef WIN32
        _endthread( );
    #endif
}
int main(int argc, char *argv[]){
    BIO *acc, *client;
    SSL *ssl;
    SSL_CTX *ctx;

    init_OpenSSL( );
    seed_prng( );

    ctx = setup_server_ctx( );  //ssl통신 초기화함수

    acc=BIO_new_accept(PORT);
    if (!acc)
        int_error("Error creating server socket");

    if(BIO_do_accept(acc)<=0)
        int_error("Error binding server socket");

    printf("server 준비 성공\n");
    tid = (THREAD_TYPE*)malloc(sizeof(THREAD_TYPE)*10);

    for(;;)
    {
        if(BIO_do_accept(acc)<=0)
            int_error("Error accepting connection");
        
        client = BIO_pop(acc); //client받는다
        if (!(ssl = SSL_new(ctx)))
            int_error("Error creating SSL context");
        
        printf("Client[%d]와의 SSL 연결 성공\n",clientnum);

        SSL_set_bio(ssl, client, client);

        clients[clientnum++]=ssl;
        
        pthread_create(&(tid[clientnum-1]), NULL, (void *)server_thread, ssl);
    }

    SSL_CTX_free(ctx);
    BIO_free(acc);
    free(tid);
    return 0;
}

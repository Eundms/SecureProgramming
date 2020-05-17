#include "common.h"
#include <string.h>
#define CERTFILE "server.pem"

int clientnum=0;
SSL *clients[10];
THREAD_TYPE *tid;
pthread_mutex_t mutex;

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
    int err, nread;
    char buf[80];
    int readfromwho;
    do
    {
        for (nread=0; nread < sizeof(buf); nread += err)
        {
            err = SSL_read(ssl, buf + nread, sizeof(buf) - nread);
            if (err <= 0)
                break;
        }
        /*어떤 client인지 구분하는 방법: pthread_self()이용한다.*/
        for(int i=0;i<clientnum;i++){
            if(pthread_self()==tid[i])readfromwho=i;}
        fprintf(stdout,"From client[%d]:",readfromwho);
        /*읽은 내용 서버에 보여준다*/
        fprintf(stdout, "%s", buf);
        /*download요청이 있을 때, 서버에서 할일을 적으면 됨*/
        if(strcmp(buf,"download\n")==0){
            fprintf(stdout,"download request from client[%d]",readfromwho);}
        /*buffer를 지워야 한다*/
        memset(buf, 0, sizeof(buf));

    } 
    while (err > 0);
    //종료된거니까 tid[readfromwho]에 있는 내용은 돌려주고 재사용할 수 있도록 하면 좋을텐데......
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

    ctx = setup_server_ctx( );//ssl통신 초기화함수

    acc=BIO_new_accept(PORT);
if (!acc)
    int_error("Error creating server socket");

if(BIO_do_accept(acc)<=0)
    int_error("Error binding server socket");

printf("성공\n");
tid = (THREAD_TYPE*)malloc(sizeof(THREAD_TYPE)*10);

for(;;)
{
    if(BIO_do_accept(acc)<=0)
        int_error("Error accepting connection");
    
    client = BIO_pop(acc);//client받는다
    if (!(ssl = SSL_new(ctx)))
        int_error("Error creating SSL context");
    
    printf("\nClient[%d]의 SSL 연결 성공\n",clientnum+1);


    SSL_set_bio(ssl, client, client);

    clients[clientnum++]=ssl;
    

    pthread_create(&(tid[clientnum-1]), NULL, (void *)server_thread, ssl);
}

SSL_CTX_free(ctx);
BIO_free(acc);
free(tid);
return 0;
}

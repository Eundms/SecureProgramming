#include "common.h"

#define CERTFILE "client.pem"

SSL_CTX *setup_client_ctx(void)
{
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(SSLv23_method());
    if(SSL_CTX_use_certificate_chain_file(ctx,CERTFILE)!=1)
        int_error("Error loading certificate from file");
    if(SSL_CTX_use_PrivateKey_file(ctx,CERTFILE, SSL_FILETYPE_PEM)!=1)
        int_error ("Error loading private key from file");
    return ctx;
}



int  do_read_loop(SSL *ssl)
{
	int err, nread;
	char buf[80];

	for(;;)
	{
		
		for(nread=0; nread<sizeof(buf); nread+=err)
		{
			err = SSL_read(ssl, buf+nread, sizeof(buf)-nread);
			if(err<=0)
				return 0;
		}
		fprintf(stdout,"DOWNLOAD RESULT: %s\n",buf);
		if(strcmp(buf,"CLOSE\n")==0)break;		
	}
    return 1;
}
int  do_write_loop(SSL *ssl)
{
	int err, nwritten;
	char buf[80];

	for(;;)
	{
		if(!fgets(buf, sizeof(buf), stdin))break;
		
		for(nwritten=0; nwritten<sizeof(buf); nwritten+=err)
		{
			err = SSL_write(ssl, buf+nwritten, sizeof(buf)-nwritten);
			if(err<=0)
				return 0;
		}

		if(strcmp(buf,"CLOSE\n")==0)break;	

	}
    return 1;
}
void THREAD_CC write_thread(void *arg)
{
	SSL *ssl = (SSL *)arg;

        if (do_write_loop(ssl))
            SSL_shutdown(ssl);
        else
            SSL_clear(ssl);
        fprintf(stderr, "SSL Connection closed-WRITE\n");
}
void THREAD_CC read_thread(void *arg)
{
	SSL *ssl = (SSL *)arg;

        if (do_read_loop(ssl))
            SSL_shutdown(ssl);
        else
            SSL_clear(ssl);
        fprintf(stderr, "SSL Connection closed-READ\n");
}
int main(int argc, char *argv[])
{
	BIO *conn;
    SSL *ssl;
    SSL_CTX *ctx;
	THREAD_TYPE send,recv;
	void * thread_return;


	init_OpenSSL();
    seed_prng();
    ctx = setup_client_ctx();

	conn = BIO_new_connect(SERVER ":" PORT);
	if(!conn)
		int_error("Error creating connection BIO");

	if(BIO_do_connect(conn)<=0)
		int_error("Error connecton to remote machine");

    if(!(ssl = SSL_new(ctx)))
        int_error("Error creating an SSL context");
    SSL_set_bio(ssl,conn, conn);
    if(SSL_connect(ssl)<=0)
        int_error("Error connecting an SSL object");
    
	fprintf(stderr, "Connection opened\n");

   	
	THREAD_CREATE(send,(void *)write_thread,ssl);
	THREAD_CREATE(recv,(void *)read_thread,ssl);
	pthread_join(send,&thread_return);
	pthread_join(recv,&thread_return);

	fprintf(stderr, "Connection closed\n");

	SSL_free(ssl);
    SSL_CTX_free(ctx);
	return 0;
}

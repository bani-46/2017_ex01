
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 80        /*todo ポート番号 ← 実験するとき書き換える */

#define S_BUFSIZE 100   /* 送信用バッファサイズ */
#define R_BUFSIZE 20   /* todo 受信用バッファサイズ */

int main(int argc, char *argv[])
{
	/*
	 * argv[1] = 指定するURL( http://www.is.kit.ac.jp/ 等の形式)
	 * argv[2] = httpプロキシーの指定が必要な場合には、第2引数にプロキシーサーバのホスト名（proxy.cis.kit.ac.jp 等）
	 * argv[3] = プロキシーサーバの待ち受けポート番号（8080 等）
	 * 第2引数および第3引数が指定されていない場合は、
	 * 第1引数で指定したURLで示されるhttpサーバ（上述の例であれば、 www.is.kit.ac.jp）のポート80番に直接接続
	 *
	 *
	 */

	struct hostent *server_host;
	struct sockaddr_in server_adrs;

	int sock;

	char send_text[S_BUFSIZE];
	char receive_text[R_BUFSIZE];
	int strsize;

	char *servername;//www.is.kit.ac.jp
	char *proxyserver;//proxy.cis.kit.ac.jp
	int proxyport;//8080


	/*HTTPプロトコル生成*/
	switch(argc){
	case 2:
		servername = argv[1];
		sprintf(send_text,
				"HEAD / HTTP/1.1\r\n"
				"Host:%s\r\n"
				"\r\n",servername);
		printf("%s",send_text);//use check
		break;
	case 4:
		servername = argv[1];
		proxyserver = argv[2];
		proxyport = atoi(argv[3]);
		sprintf(send_text,
				"HEAD / HTTP/1.1\r\n"
				"Host:%s:%d\r\n"
				"\r\n",proxyserver,proxyport);
		printf("%s",send_text);//use check
		break;
	default:
		fprintf(stderr,"[ERR]Not enough or too many arguments");
		exit(EXIT_FAILURE);
		break;
	}

	/* サーバ名をアドレス(hostent構造体)に変換する */
	if((server_host = gethostbyname( servername )) == NULL){
		fprintf(stderr,"[ERR]cannnot get server_host");
		exit(EXIT_FAILURE);
	}

	/* サーバの情報をsockaddr_in構造体に格納する */
	memset(&server_adrs, 0, sizeof(server_adrs));
	server_adrs.sin_family = AF_INET;
	server_adrs.sin_port = htons(PORT);
	memcpy(&server_adrs.sin_addr, server_host->h_addr, server_host->h_length);

	/* ソケットをSTREAMモードで作成する */
	if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr,"[ERR]cannot make socket");
		exit(EXIT_FAILURE);
	}

	/* ソケットにサーバの情報を対応づけてサーバに接続する */
	if(connect(sock, (struct sockaddr *)&server_adrs, sizeof(server_adrs))== -1){
		fprintf(stderr,"[ERR]cannot connect server");
		exit(EXIT_FAILURE);
	}

	/*HTTPプロトコル送信*/
	strsize = strlen(send_text);
	send(sock, send_text, strsize+1, 0);

	/*受信処理*/
//	char str1[] = "Content-Length:";
//	char str2[] = "Server:";
	int receive_byte;
	while (1){
		receive_byte = recv(sock,receive_text,R_BUFSIZE-1,0);
		if(receive_byte == 0)break;
		else{
			receive_text[receive_byte] = '\0';
			printf("%s",receive_text);
//			if(strstr(receive_text,str1) != NULL){
//				printf("[[[INFO]]]Content-LengthFound!");
//			}
//			else if(strstr(receive_text,str2) != NULL){
//				printf("[[[INFO]]]ServerFound!");
//			}
		}
	}

	close(sock);             /* ソケットを閉じる */
	exit(EXIT_SUCCESS);
}

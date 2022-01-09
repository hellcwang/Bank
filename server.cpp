#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <sys/file.h>

#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8


//Socket address 
struct sockaddr_in serv_addr, cli_addr;

//Socket file descripter
int sockfd = -1;	//Socket for listening incoming connect
int incoming_sock = -1;	//Real communicate socket
void handler_int(int);

//Log file path
const char log_path_init[] = "./log/";
char log_path[128];

//Private Account path
const char account_path_init[] = "./account/";
char account_path[128];

//Tmp string
char tmp[128] ;
constexpr unsigned int SERVER_PORT = 50544;
constexpr unsigned int MAX_BUFFER = 128;
constexpr unsigned int MSG_REPLY_LENGTH = 18;

char account[16];	//For input username
char passwd[16];	//For input password
const char* account_list = "./Account_List";	//All account

void log_write(const char*);
void money_transfer(void);
void balance_check(void);
void withdraw(void);
void deposit(void);
void manu(void);
void bank(void);	//Register or login
void login(char*, char*);
void regist(char*, char*);

int main(int argc, char* argv[]){

	//Initial tmp string
	memset(tmp, 0, 128);
	//Initial signal handler
	signal(SIGINT, handler_int);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		std::cerr << "open socket error\n";
		return 1;
	}

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, \
			(const void*) &optval, sizeof(int));

	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERVER_PORT);

	if(bind(sockfd, (struct sockaddr*) &serv_addr, \
				sizeof(serv_addr)) < 0){
		std::cerr << "bind error\n" ;
		return 2;
	}

	listen(sockfd, 5);

	socklen_t clilen = sizeof(cli_addr);
	int pid;
	while(1){

		incoming_sock = accept(sockfd, (struct sockaddr*) &cli_addr, \
						&clilen);

		pid = fork();

		//parent
		if(pid > 0){

			std::cout << "server : got connection from = "
				<< inet_ntoa(cli_addr.sin_addr)
				<< "and port = " << ntohs(cli_addr.sin_port) 
				<< std::endl;

			close(incoming_sock);
		}

		//Child
		else if(pid == 0){
			close(sockfd);
			bank();
			close(incoming_sock);
			return 0;
		}
	}
	close(sockfd);
	return 0;

}

void handler_int(int sig){
	//printf("interrupt catch!\n");
	if(incoming_sock != -1)
		close(incoming_sock);
	if(sockfd != -1)
		close(sockfd);

	//End of function
	exit(0);
}

void bank(void){
	char flag = '0';
	char buffer[128];

	memset(buffer, 0, 128);
	dprintf(incoming_sock, "Connected!\n"
			"Register input 1\nLogin input 2\n");
	read(incoming_sock, buffer, 128);
	flag = buffer[0];
	if(flag == '2'){
		dprintf(incoming_sock, "Please input account name : ");
		read(incoming_sock, account, 15);
		printf("%s\n", account);
		dprintf(incoming_sock, "Please input password : ");
		read(incoming_sock, passwd, 15);
		printf("%s\n", passwd);
		login(account, passwd);
	}else if(flag == '1'){
		dprintf(incoming_sock, "Please input username : ");
		read(incoming_sock, account, 15);
		dprintf(incoming_sock, "Please input password : ");
		read(incoming_sock, passwd, 15);
		regist(account, passwd);
	}else{
		bank();
	}
	return ;
}

void login(char* account, char* passwd){
	FILE *check;
	FILE *log;

	char acc[16];
	char pas[16];
	char* t; 
	int i;

	memset(acc, 0, 16);
	memset(pas, 0, 16);

	memset(log_path, 0, 128);
	memset(account_path, 0, 128);
	//Path of log file
	strcat(log_path, log_path_init);
	strcat(log_path, account);
	
	//indivual path
	strcat(account_path, account_path_init);
	strcat(account_path, account);
	check = fopen(account_list, "r");
	while(fscanf(check, "%s", acc) != EOF){
		fscanf(check, "%s", pas);
		if(!strcmp(acc, account) && !strcmp(pas, passwd)){
			//dprintf(incoming_sock, "login successful !\n");
			goto end;
		}
	}
	fclose(check);
	log_write("login failed !\n");
	dprintf(incoming_sock, "login failed!!\n");
	return ;
end:
	manu();
	return ;
}

void regist(char* account, char* passwd){
	FILE *check;
	FILE *new_file;

	memset(log_path, 0, 128);
	memset(account_path, 0, 128);
	//Path of log file
	strcat(log_path, log_path_init);
	strcat(log_path, account);
	
	//indivual path
	strcat(account_path, account_path_init);
	strcat(account_path, account);

	new_file = fopen(account_path, "w");
	fprintf(new_file, "%d", 0);
	fclose(new_file);

	check = fopen(account_list, "a");
	if(check == NULL){
		printf("fopen error\n");
	}
	fprintf(check, "%s", account);
	fprintf(check, " %s\n", passwd);
	dprintf(incoming_sock, "Register successful!\nPlease login later\n");
	fclose(check);
	log_write("Account initial\n");
	return;
}

void manu(void){
	char flag = 0;
	char buffer[128];
	memset(buffer, 0, 128);

	memset(account_path, 0, 128);	//Global account_path
	strcat(account_path, account_path_init);
	strcat(account_path, account);

	dprintf(incoming_sock, "Login successful!\n"
			"please choose what you want to do\n"
			"1 : Deposite\n"
			"2 : Withdraw\n"
			"3 : Balance check\n"
			"4 : Money transfer\n\n");

	read(incoming_sock, buffer, 128);
	flag = buffer[0];

	switch(flag){
		case '1':
			deposit();
			//printf("deposit");
			break;
		case '2':
			withdraw();
			//printf("withdraw");
			break;
		case '3':
			balance_check();
			//printf("balance check");
			break;
		case '4':
			//money_transfer();
			printf("money  transfer");
			break;
		default:
			manu();
	}
	return ;
}

void deposit(void){
	FILE *f;
	int rc;
	int value = -1;
	int tosave = -1;
	char buffer[16];
	char tmp[100];

	memset(buffer, 0, 16);
	memset(tmp, 0, 100);
	dprintf(incoming_sock,\
		       	"Please input how much money U want to save\n");
	read(incoming_sock, buffer, 15);	
	sscanf(buffer, "%d", &tosave);
	f = fopen(account_path, "r+");
	rc = flock(fileno(f), LOCK_EX);
	
	//Critical section
	fscanf(f, "%d", &value);
	value += tosave;
	fseek(f, 0, SEEK_SET);
	fprintf(f, "%d\n", value);
	//End 
	
	rc = flock(fileno(f), LOCK_UN);
	fclose(f);
	sprintf(tmp, "Deposit: %d + %d -> %d\n", value - tosave,\
		       	tosave, value);
	log_write(tmp);
	dprintf(incoming_sock,\
			"Done\n");
	return ;
}
	

void withdraw(void){
	FILE *f, *tmpf;
	int rc;
	int value = -1;
	int tosave = -1;
	char buffer[16];
	char tmp[100];
	char ftmp[100];

	memset(buffer, 0, 16);
	memset(tmp, 0, 100);
	memset(ftmp, 0, 100);
	dprintf(incoming_sock,\
		       	"Please input how much money U want to take out\n");
	read(incoming_sock, buffer, 15);	
	sscanf(buffer, "%d", &tosave);
	f = fopen(account_path, "r+");
	rc = flock(fileno(f), LOCK_EX);
	
	//Critical section
	strcat(ftmp, account_path);
	strcat(ftmp, ".tmp");
	tmpf = fopen(ftmp, "w");
	fscanf(f, "%d", &value);
	value -= tosave;
	if(value < 0){
		dprintf(incoming_sock, \
				"Insufficient balance\n");
		rc = flock(fileno(f), LOCK_UN);
		fclose(f);
		fclose(tmpf);
		return ;
	}
	fprintf(tmpf, "%d\n", value);
	//End 
	
	rc = flock(fileno(f), LOCK_UN);
	fclose(f);
	//Tmp -> new file
	remove(account_path);
	fclose(tmpf);
	rename(ftmp, account_path);

	//log
	sprintf(tmp, "Withdraw: %d - %d -> %d\n", value + tosave,\
		       	tosave, value);
	log_write(tmp);
	dprintf(incoming_sock,\
			"Withdraw Done\n");
	return ;
}

void log_write(const char* str){
	
	FILE *f;

	time_t rawtime;
        struct tm *timeinfo;
	char *t;

        //Get local time for logging
        //Log for login failed
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        t = asctime(timeinfo);
	int i;
        for(i = 0;i < 128;i ++){
                if(t[i] == 0)
                        break;
                if(t[i] == '\n')
                        t[i] = 0;
        }
	
	f = fopen(log_path, "a");
	if(f == NULL)
		printf("fopen error!!!\n");
        fprintf(f, "[%s][%s] %s", t, \
                        inet_ntoa(cli_addr.sin_addr), str);
        fclose(f);
	
	return ;
}

void balance_check(void){
	char tmp[128];
	FILE *f;
	int balance;

	memset(tmp, 0, 128);
	f = fopen(account_path, "r");
	fscanf(f, "%d", &balance);
	fclose(f);
	dprintf(incoming_sock, "Balance : %d\n done\n", balance);

	return;
}



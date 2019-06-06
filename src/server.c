/*     ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
	Kandidati: 		Luka Karan i Laza Jakovljevic
    
    Ime fajla:      server.c
    Opis:           TCP/IP server
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
 ********************************************************************
*/
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <pthread.h> 
#include<stdlib.h>
#include <sys/file.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015
#define MAX_CLIENTS 200

int BrojClanskeKartice;
int t;
int brLinija;

void* ThreadFunction(void *);
void SendFunction(int, char*);
void ReceiveFunction(int, char*);
int proveraUsername(char* );
int pretrazivanjeUsrClanska(char* msg1,char* msg2);
int countlines(char *);
void searchAll(char*,int);
void searchFilter(const char*,const char*,char*,int);
void Reserve(int, char*, char*, char*);
void CheckStatus(int, char*, char*, char*, char*);
void Logout();

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
   
       //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    
	int optval = 1;
	setsockopt(socket_desc,SOL_SOCKET, SO_REUSEADDR,(const void*)&optval, sizeof(int));
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
   
    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    c = sizeof(struct sockaddr_in);

    pthread_t nit[MAX_CLIENTS]; //definisemo nit
    int numClients;
	
    //accept connection from an incoming client
    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)))
    {
	//puts("Connection accepted");

	if(pthread_create(nit, NULL, ThreadFunction, (void*)&client_sock) < 0)
	{
		puts("Could not create thread.");
		return 1;
	}
	
	pthread_join(*nit, NULL);
	
	numClients++;
	if(numClients > MAX_CLIENTS)
	{
		puts("Maximal number of clients is reached!");
		return 1;
	}
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }


	return 0;
}



void* ThreadFunction(void* socket_desc)
{
	int sock = *(int*)socket_desc;
	int p;
	char client_message1[DEFAULT_BUFLEN] = "\0";
	FILE *f1;
	char greska[DEFAULT_BUFLEN] = "Pogresan unos.\n";
	char usr[DEFAULT_BUFLEN] = "Username:";
	char pass[DEFAULT_BUFLEN] = "Password:";
	char porukaGreskaUsr[DEFAULT_BUFLEN] = "Username vec postoji. Pokusajte da se ponovo registrujete.\n\0";
	char porukaUspesnoUsr[DEFAULT_BUFLEN] = "Validan username.";
	char failUsrClanska[DEFAULT_BUFLEN] = "Nepostojeci username, broj clanske kartice ili password. Pokusajte ponovo.\n\0";
	char successUsrClanska[DEFAULT_BUFLEN] = "Uspesno logovanje.";
	
	while(1){
	ReceiveFunction(sock,client_message1);
        
	if(client_message1[0] == '1')
	{
		while(1){
		char usrPrimljeno[DEFAULT_BUFLEN] = "\0";
		char passPrimljeno[DEFAULT_BUFLEN] = "\0";
		SendFunction(sock,usr);
	
		ReceiveFunction(sock, usrPrimljeno);
	
			//upis username-au fajl
			f1 = fopen("baza1.txt", "a+");
			
			p = proveraUsername(usrPrimljeno);
			
		
			
				if(p == 0)
				{
					SendFunction(sock,porukaUspesnoUsr);
					fprintf(f1, "%s", usrPrimljeno);
					fprintf(f1, " ");

					ReceiveFunction(sock, passPrimljeno);
					
					//unos password-a u fajl
					
					fprintf(f1, "%s", passPrimljeno);
					fprintf(f1, " ");
						
					//unos broja clanske kartice u fajl
                
					BrojClanskeKartice = countlines("baza1.txt");
					char memCard[DEFAULT_BUFLEN] = "\0";
					sprintf(memCard,"%d",BrojClanskeKartice);
					
                    fprintf(f1,"%s", memCard);
					fprintf(f1, " ");
					
					SendFunction(sock, memCard);
					
					fprintf(f1, "\n");
					fclose(f1);
					break;
			
				} else {
					SendFunction(sock,porukaGreskaUsr);
					break;
				}
        
               
		}
    } else if(client_message1[0] == '2'){
		 
			char passPrimljeno[DEFAULT_BUFLEN] = "\0";
			char usrClanskaPrimljeno[DEFAULT_BUFLEN] = "\0";
			char usrClanska[DEFAULT_BUFLEN] = "Unesite username ili broj clankse kartice:";
			char option[DEFAULT_BUFLEN] = "\0";
			
			SendFunction(sock,usrClanska);
		
			ReceiveFunction(sock,usrClanskaPrimljeno);
			
			SendFunction(sock,pass);

			ReceiveFunction(sock,passPrimljeno);
			
			t = pretrazivanjeUsrClanska(usrClanskaPrimljeno, passPrimljeno);

			if(t == 0){
				SendFunction(sock, failUsrClanska);
				continue;
			} else {
				SendFunction(sock, successUsrClanska);
			}
			
				while(1){
					ReceiveFunction(sock,option);
					
					char text[DEFAULT_BUFLEN] = "\0";

					int op = atoi(option);
					
					if(op == 1){
						searchAll("baza2.txt",sock);
						continue;
					} else if(op == 2 || op == 3 || op == 4 || op == 5){
						ReceiveFunction(sock, text);
						searchFilter(text,option,"baza2.txt",sock);
						continue;
					} else if(op == 6){
						ReceiveFunction(sock, text);
						Reserve(sock, text, "baza2.txt", usrClanskaPrimljeno);
						continue;
					}else if(op == 7){
						CheckStatus(sock,"baza1.txt","baza2.txt",usrClanskaPrimljeno,passPrimljeno);
						continue;
					}else if(op == 8){
						SendFunction(sock, "Logout...");
						Logout();
						return 0;
					}else {
						SendFunction(sock,greska);
						continue;
					}
				}
	}else{
		SendFunction(sock,greska);
		continue;
    }
	}
}


void SendFunction(int sock, char* msg){
	
	if( send(sock , msg, strlen(msg), 0) < 0)
	{
				puts("Send failed");
				exit(1);
	}
}

void ReceiveFunction(int sock, char* msg){
	int readSize;
	if ((readSize = recv(sock, msg , DEFAULT_BUFLEN , 0)) > 0 )
    {
	} 
}

int pretrazivanjeUsrClanska(char* msg1, char* msg2){
		
	char str[DEFAULT_BUFLEN];
	const char* temp1 = msg1;
	const char* temp2 = msg2;
	int retVal; 
   
	FILE *f1 = fopen("baza1.txt", "r");
	// provera da li je fajl otvoren
	if (f1 == NULL) {
		printf("Couldn't Open File: listaUserrnamePassword.txt!\n");
		exit(-1);
	}
	
		while ((fgets(str, DEFAULT_BUFLEN, f1)) != NULL){
			int duzina = 0, duzinaUsr = 0, duzinaPass = 0, duzinaClanska = 0;
			int duzinaProveraUsr = 0, duzinaProveraPass = 0, duzinaProveraClanska = 0;
			
			int u = 1, p = 1, c = 1;
			int i = 0, j = 0; 
			
			retVal = 1; //tacno to je resenje
			
			duzina = strlen(str)-2;
			 
			duzinaProveraUsr = strlen(temp1);
			duzinaProveraPass = strlen(temp2);
			
			duzinaProveraUsr = strlen(temp1);
			duzinaProveraPass = strlen(temp2);

			for(i = 0; i < duzina; i++){
				if(str[i] != ' '){
					duzinaUsr++;
				}else
					break;
			}
			
			if(duzinaUsr != duzinaProveraUsr){
				u = 0;
			}else{
				for(i = 0; i < duzinaUsr; i++){
					if(str[i] != temp1[i]){
						u = 0;
						break;
					}
					u = 1;
				}
			}
			for(i = duzinaUsr + 1; i < duzina; i++){
				if(str[i] != ' '){
					duzinaPass++;
				}else
					break;
			}
			if(u == 0){
				duzinaProveraClanska = strlen(temp1);	
				 for(i = duzinaPass + duzinaUsr + 2; i < duzina; i++){
					if(str[i] != ' '){
						duzinaClanska++;
					}else
						break;
					}
				if(duzinaClanska != duzinaProveraClanska){
					c = 0;
				}else{
					for(i = duzinaUsr + duzinaPass + 2, j = 0; i < duzina; i++, j++){
						if(str[i] != temp1[j]){
							c = 0;
							break;
						}
					}
				}
				if(c == 0){
					retVal = 0;
				} else {
						if(duzinaPass != duzinaProveraPass){
							p = 0;
						}else{
							for(i = duzinaUsr + 1, j = 0; i <= duzinaUsr + duzinaPass; i++, j++){
								if(str[i] != temp2[j]){
									p = 0;
									break;
								}
							}
						}
						
						if(p == 1){
								retVal = 1;
								break;
						}
				}	
			}	else if(u == 1){
					if(duzinaPass != duzinaProveraPass){
							p = 0;
						}else{
							for(i = duzinaUsr + 1, j = 0; i <= duzinaUsr + duzinaPass; i++, j++){
								if(str[i] != temp2[j]){
									p = 0;
									break;
								}
	
							}
						}
						
						if(p == 1){
								retVal = 1;
								break;
						}
			}
		}
		
		return retVal;
}

int proveraUsername(char* msg){
	
	const char* temp = msg;
	FILE *f1;
	char str[DEFAULT_BUFLEN];
	char *parse;
	int p = 0;
	
	f1 = fopen("baza1.txt", "r");
	// provera da li je fajl otvoren
	if (f1 == NULL) {
		printf("Couldn't Open File: listaUserrnamePassword.txt!\n");
		exit(-1);
	}
	
		while ((fgets(str, DEFAULT_BUFLEN, f1)) != NULL){
			parse = strtok(str, " ");
			
			if(strcmp(parse,temp) == 0){
				p = 1;
				break;
			}
		}
	fclose(f1);
	return p;
	
}

int countlines(char *filename){
  // count the number of lines in the file called filename                                    
  FILE *myfile = fopen(filename,"r");
  int ch, numberOfLines = 1;
do
{
    ch = fgetc(myfile);
    if(ch == '\n')
        ++numberOfLines;
}
while (ch != EOF);

    fclose(myfile);
  return numberOfLines;
}

void searchAll(char* file_name, int sock){
    FILE* file = fopen(file_name,"r");
    char str[DEFAULT_BUFLEN];
	char str2[DEFAULT_BUFLEN] = "\0";
	while((fgets(str, DEFAULT_BUFLEN, file)) != NULL){
		strcat(str2,str);
		strcat(str2,"\n");
	}
	SendFunction(sock, str2);
	fclose(file);
}

void searchFilter(const char* filter,const char* option, char* file_name, int sock){
    FILE* file = fopen(file_name,"r");
    char src[DEFAULT_BUFLEN], dest[DEFAULT_BUFLEN];
    char str[DEFAULT_BUFLEN] = "\0";
    char str1[DEFAULT_BUFLEN] = "\0";
	int op = atoi(option);
    switch(op){
        case 2:
            strcpy(dest,  "id:");
            strcpy(src, filter);
            strcat(dest, src);
            break;
        case 3:
            strcpy(dest,  " Manufactured:");
            strcpy(src, filter);
            strcat(dest, src);
            break;
        case 4:
            strcpy(dest,  "Model:");
            strcpy(src, filter);
            strcat(dest, src);
            break;
        case 5:
            strcpy(dest,  "Year:");
            strcpy(src, filter);
            strcat(dest, src);
            break;
        default:
            printf("Pogresan unos.");
            break;
    }
 
	
	char str2[DEFAULT_BUFLEN] = "\0";

 
	while((fgets(str, DEFAULT_BUFLEN, file)) != NULL){
	
		strcpy(str1,str);
	
		if(strstr(str1,dest) != NULL){
			strcat(str2,str);
			strcat(str2,"\n");	
       }
    }
	
	if(!strcmp(str2,"\0")){
		strcpy(str2,"Doesn't exist.");
	}
	
	SendFunction(sock, str2);
    
    fclose(file);
	
	
}


void Reserve(int sock, char* filter, char* fileName1, char* usrMember){
	
	char str3[DEFAULT_BUFLEN] = "\0";
	
	FILE* file1 = fopen(fileName1,"r");
    char src[DEFAULT_BUFLEN], dest[DEFAULT_BUFLEN];
	char str[DEFAULT_BUFLEN] = "\0";
	char str1[DEFAULT_BUFLEN] = "\0";
	char str4[DEFAULT_BUFLEN] = "\0";

	strcpy(dest,  "id:");
    strcpy(src, filter);
    strcat(dest, src);
	
	strcpy(str4,  "RESERVED:");
    strcat(str4, usrMember);
	strcat(str4, " ");
	
	while((fgets(str, DEFAULT_BUFLEN, file1)) != NULL){
		
		strcpy(str1,str);
		
		if(strstr(str1,"RESERVED:") != NULL){
			strcat(str3,str1);
		} else if(strstr(str1,dest) != NULL){
			strcat(str3,str4);
			strcat(str3,str1);
	   } else{
			strcat(str3,str1);
       }   
    }
	
	fclose(file1);
	
	
	file1 = fopen(fileName1,"r");
	char strr[DEFAULT_BUFLEN] = "\0";
	
	
	while((fgets(str, DEFAULT_BUFLEN, file1)) != NULL){
		
		char str2[DEFAULT_BUFLEN] = "\0";
		strcpy(str1,str);
		//puts(str1);
		
		if((strstr(str1,"RESERVED:") != NULL) && (strstr(str1,dest) != NULL)){
			strcpy(str2, "Already reserved.\n");
			strcpy(strr,str2);
			break;
		} else if(strstr(str1,dest) != NULL){
			strcpy(str2,str4);
			strcat(str2,str1);
			strcat(str2,"\n");
			strcpy(strr,str2);
			
			break;
	   } 
    }
	
		if(!strcmp(strr,"\0")){
			strcpy(strr, "Doesn't exist.\n");
		}
	SendFunction(sock,strr);
	fclose(file1);
	
	file1 = fopen(fileName1,"w");
	fprintf(file1, "%s", str3);
	fclose(file1);
}

void CheckStatus(int sock, char* fileName1, char* file_name2, char* usrMember, char* passPrimljeno){
	
	char str[DEFAULT_BUFLEN] = "\0";
	char username[DEFAULT_BUFLEN] = "\0";
	char password[DEFAULT_BUFLEN] = "\0";
	char memCard[DEFAULT_BUFLEN] = "\0";
	char str6[DEFAULT_BUFLEN] = "\0";
	char str8[DEFAULT_BUFLEN] = "\0";
	
	
	int l1;
	int l3;
	
	FILE* file1 = fopen(fileName1,"r");
	
	while((fgets(str, DEFAULT_BUFLEN, file1)) != NULL){
		
		int len, len1 = 0,len2 = 0,len3 = 0,i,j;
		char str1[DEFAULT_BUFLEN] = "\0";
		char str2[DEFAULT_BUFLEN] = "\0";
		char str3[DEFAULT_BUFLEN] = "\0";
		char str4[DEFAULT_BUFLEN] = "\0";
		
		strcpy(str1,str);
		
		len = strlen(str1);
		
		for(i = 0; i < len-2; i++){
			if(str1[i] != ' '){
				str2[i] = str1[i];
				len1++;	
			} else{
				break;
			}
		}
		
		strcpy(username, str2);
		l1 = len1;
		
		for(i = len1+1, j = 0; i < len-2; i++,j++){
			if(str1[i] != ' '){
				str3[j] = str1[i];
				len2++;	
			} else{
				break;
			}
		}
		
		strcpy(password, str3);
		
		for(i = len1+len2+2, j = 0; i < len-2; i++,j++){
			if(str1[i] != '\0'){
				str4[j] = str1[i];
				len3++;	
			} else{
				break;
			}
		}
		
		strcpy(memCard, str4);
		l3 = len3;
		
		if((strcmp(username,usrMember) == 0 || strcmp(memCard,usrMember) == 0)){
			if(strcmp(passPrimljeno,passPrimljeno) == 0)
				break;
		}
		
	}

//  puts(username);
//	puts(password);
//	puts(memCard);
	
	strcat(str6, "RESERVED:");
	strcat(str6, username);
	//puts(str6);
	
	strcat(str8, "RESERVED:");
	strcat(str8, memCard);
	//puts(str8);
	
	fclose(file1);
	
	FILE* file2 = fopen(file_name2,"r");
	char str5[DEFAULT_BUFLEN] = "\0";
	
	while((fgets(str, DEFAULT_BUFLEN, file2)) != NULL){
		
		//puts(str);
		
		if((9+strlen(usrMember) == 9+l1) || (9+strlen(usrMember) == 9+l3)){						if((strstr(str, str6) != NULL) || (strstr(str, str8) != NULL)){
					strcat(str5, str);
					strcat(str5,"\n");
			}
		}
	}
	if(strcmp(str5, "\0") == 0){
		strcpy(str5, "Nothing reserved.");
	}
	
	SendFunction(sock,str5);
	
	fclose(file2);
	
}

void Logout(){

	int readSize=0;
	if(readSize == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(readSize == -1)
    {
        perror("recv failed");
    }

   
}
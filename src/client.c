/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
    Kandidati: 		Laza Jakovljevic i Luka Karan
	
    Ime fajla:      client.c
    Opis:           TCP/IP klijent
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include<stdio.h>      //printf
#include<string.h>     //strlen
#include<sys/socket.h> //socket
#include<arpa/inet.h>  //inet_addr
#include <fcntl.h>     //for open
#include <unistd.h>    //for close
#include <stdlib.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015

void SendFunction(int, char*);
void ReceiveFunction(int, char*);
void ChooseOption(int,int);

int main(int argc , char *argv[])
{
		int sock;
		struct sockaddr_in server;
		char buffer2[DEFAULT_BUFLEN] = "\0"; //poruka od servera Username
		char usrClanska[DEFAULT_BUFLEN] = "\0";
		char passMsg[DEFAULT_BUFLEN] = "\0";
		char pocetak[DEFAULT_BUFLEN]; //izbor Registracija ili LOGIN koji salje klijent
    
		//Create socket
		sock = socket(AF_INET , SOCK_STREAM , 0);
		if (sock == -1)
		{
			printf("Could not create socket");
		}
	
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons(DEFAULT_PORT);

		//Connect to remote server
		if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			perror("connect failed. Error");
			return 1;
		}

		while(1){
			//poruka od servera Registracija i LOGIN
			printf("Izaberite opciju:\n 1. Registracija \n 2. LOGIN \n ");
			
			scanf("%s", pocetak);
			
			//izbor Registracija ili LOGIN koji salje klijent
			SendFunction(sock,pocetak);
		   
		   //Registration
		   if(pocetak[0] == '1'){
			
				while(1){
					char username[DEFAULT_BUFLEN] = "\0";
					char pass[DEFAULT_BUFLEN]= "\0";
					char buffer4[DEFAULT_BUFLEN] = "\0"; 
					char buffer5[DEFAULT_BUFLEN] = "\0"; 
						
					ReceiveFunction(sock, buffer2);
					puts(buffer2);
					scanf("%s", username);
					SendFunction(sock,username);
									
					char porukaUspesnoUsr[DEFAULT_BUFLEN] = "Validan username.";
					ReceiveFunction(sock,buffer4);
					puts(buffer4);
					
					
					if(!strcmp(buffer4, porukaUspesnoUsr)){
						
						printf("Password: ");
						scanf("%s",pass);
						SendFunction(sock,pass);
						ReceiveFunction(sock,buffer5);
						puts(buffer5);
						break;
					} else {
						break;
					}
				}

			} else if(pocetak[0] == '2'){
				
					char username[DEFAULT_BUFLEN] = "\0";
					char pass[DEFAULT_BUFLEN]= "\0";
					char loginMessage[DEFAULT_BUFLEN] = "\0";
					
					ReceiveFunction(sock, usrClanska);
					puts(usrClanska);
					scanf("%s", username);
					SendFunction(sock,username);
					
					ReceiveFunction(sock,passMsg);
					puts(passMsg);
					scanf("%s",pass);
					SendFunction(sock,pass);
				
					ReceiveFunction(sock, loginMessage);
					puts(loginMessage);
				
					char successUsrClanska[DEFAULT_BUFLEN] = "Uspesno logovanje.";
					if(!strcmp(loginMessage, successUsrClanska))
					{
						while(1){
							printf("*SEARCH*\n");
							printf("1. Search All\n2. Search ID\n3. Search by manufactured company\n4. Search by model name\n5. Search by year\n6. Reserve\n7. Check Status\n8. LOGOUT\n");
							char option[DEFAULT_BUFLEN] = "\0";
							
							
								printf("Unesite opciju:");
								scanf("%s",option);
					   
								SendFunction(sock,option);
								
								int op = atoi(option);
								
								ChooseOption(op,sock);
					
                                char search[DEFAULT_BUFLEN] = "\0";
								if(op == 8){
									ReceiveFunction(sock,search);
									puts(search);
									close(sock);
									return 0;
								}
								ReceiveFunction(sock,search);
								puts(search);
								continue;
						}
								
					} else {
						
						continue; //ako nije validan username ili clanska ili password
					}

			
		   
				} else {
					char greska[DEFAULT_BUFLEN] = "\0";
					ReceiveFunction(sock,greska);
					puts(greska);
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

void ChooseOption(int op,int sock){
	
	char text[DEFAULT_BUFLEN] = "\0";
	switch(op){
		case 1:
			printf("Pretrazivanje citave baze.\n");
			break;
		case 2:
			printf("ID:");
			scanf("%s", text);
			SendFunction(sock,text);
			break;
		case 3:
			printf("Manufactured:");
			scanf("%s", text);
			SendFunction(sock,text);
			break;						
		case 4:
			printf("Model:");
			scanf("%s", text);
			SendFunction(sock,text);
			break;
		case 5:
			printf("Year:");
			scanf("%s", text);
			SendFunction(sock,text);
			break;
		case 6:
			printf("Unesite ID dela koje zelite da rezervisete: ");
			scanf("%s", text);
			SendFunction(sock,text);
			break;
		case 7:
			printf("*Check status*\n");
			break;
		case 8:
			break;
		default:
			printf("Pogresna opcija.");
		}			
}


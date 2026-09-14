/******************************************************************************\
* Cliente Servidor en Uno, para probar tasa de transferencia en la red.
\******************************************************************************/

#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT 5001
#define DEFAULT_PROTO SOCK_STREAM // TCP
#define BUFF_LEN_CLI 32768
#define BULL_LEN_SRV 4096
#define DESCTO 100000


// Implementaci�n de GetTickCount para Linux //
long GetTickCount(void){
   struct timeval tv;
   long vald, milisec;
   
   gettimeofday(&tv, NULL); 

   vald = tv.tv_sec / DESCTO;
   vald *= DESCTO;
  
   milisec = (tv.tv_sec - vald)*1000 + (tv.tv_usec / 1000);

   return(milisec);
}


void Usage(char *progname) {
   fprintf(stderr,"Uso:\n");
   fprintf(stderr,"    %s -l -p [proto] -n [serv] -e [port] -s [kb_tx] -i [iface]\n\n", progname);

   fprintf(stderr,"Donde:\n");
   fprintf(stderr,"\t-l       modo de escucha (servidor)\n");
   fprintf(stderr,"\t[proto]  protocolo, TCP o UDP\n");
   fprintf(stderr,"\t[serv]   direccion IP o nombre del servidor\n");
   fprintf(stderr,"\t[port]   numero de port por el que opera\n");
   fprintf(stderr,"\t[kb_tx]  KB a enviar (modo cliente)\n");
   fprintf(stderr,"\t[iface]  Solo servidor, indica IP a usar (si tiene varias)\n\n");
   
   fprintf(stderr,"Por omision: TCP, localhost, port 5001, KB 1024, todas las interfaces\n\n");
   exit(1);
}


//**************************************************
//  Operaci�n en modo Servidor...
//**************************************************
int server(unsigned short port, int socket_type, char *interface, char * prog_name, char *file_to_receive){
   char Buffer[BULL_LEN_SRV];
   char MyName[128];
   int retval;
   int bytes_total = 0;
   int fromlen;
   struct sockaddr_in local, from;
   struct hostent *MyHostEnt;
   int listen_socket, msgsock;
   double mbps;
   FILE *fh;
   unsigned long addr;
   
   //Tick Counts from clock
   long  tick_init;
   long  tick_end;
   long  tick_diff;

   //INTENTANDO OBTENER MI DIRECCION....
   //--------------------------------------------
   
   if (gethostname(MyName, 30) != 0){
      fprintf(stderr,"gethostname() error \n");
      return -1;
   }
   
   MyHostEnt = gethostbyname( MyName);
   
   if ( MyHostEnt == NULL){
      fprintf(stderr,"gethostbyname() error \n");
      return -1;
   }
   
   //--------------------------------------------

   local.sin_family = AF_INET;
   
   memcpy((char *) &local.sin_addr, MyHostEnt->h_addr, MyHostEnt->h_length);
   
   local.sin_port = htons(port);
   
   listen_socket = socket(AF_INET, socket_type,0); // TCP socket

   if (listen_socket < 0 ){
      fprintf(stderr,"socket() error \n");
      return -1;
   }
	
   if (bind(listen_socket,(struct sockaddr*)&local,sizeof(local) ) < 0 ) {
       fprintf(stderr,"bind() error \n");
       return -1;
   }
   
   if (socket_type != SOCK_DGRAM) {
      if (listen(listen_socket,5) < 0) {
         fprintf(stderr,"listen() error \n");
         return -1;
      }
   }
   
   //Mi Direcci�n....
   if ( interface != NULL)
       printf("%s: '%s' escuchando: puerto %d; proto. %s; IP : %s\n\n",prog_name, MyName, port,
	      (socket_type == SOCK_STREAM)?"TCP":"UDP", interface);
   else
      printf("%s: '%s' escuchando: puerto %d, proto. %s IP : %s\n",prog_name, MyName, port,
	      (socket_type == SOCK_STREAM)?"TCP":"UDP", inet_ntoa(local.sin_addr ));

   fromlen =sizeof(from);

   msgsock = 0;
   bytes_total = 0;

   while(1) {
      if (socket_type != SOCK_DGRAM && msgsock == 0 ) {

         msgsock = accept(listen_socket,(struct sockaddr*)&from, &fromlen);
			
         if (msgsock < 0 ) {
	    fprintf(stderr,"accept() error\n");
	    return -1;
	 }
         
         bytes_total = 0;
         tick_init = GetTickCount();
         
         printf("\nConexion aceptada de %s, puerto %d  sock %d\n", 
                inet_ntoa(from.sin_addr),
                htons(from.sin_port), msgsock);

         //Archivo a recibir....
         if ( file_to_receive != NULL){
            if ( ( fh = fopen( file_to_receive, "w+b")) == NULL ){
               printf("Error abriendo archivo a recibir [%s]\n", file_to_receive);
               file_to_receive = NULL;
            }
         }

      }	
      else  //Si es UDP no recibo archivo....
         file_to_receive = NULL;
      
      if ( socket_type == SOCK_DGRAM)
         msgsock = listen_socket;

      if (socket_type != SOCK_DGRAM){
         retval = recv(msgsock,Buffer,sizeof (Buffer),0 );
         bytes_total += retval;
         
         if (fh != NULL)
            fwrite( (void *) Buffer, 1, retval, fh);

      }
      else {
         retval = recvfrom(msgsock,Buffer,sizeof (Buffer),0, (struct sockaddr *)&from,&fromlen);
         Buffer[retval] = '\0';
         
         ++bytes_total;
	 printf("Datagrama recibido de %s [%s]\n",inet_ntoa(from.sin_addr), Buffer);
      }
      
      while (socket_type != SOCK_DGRAM && retval > 0 ){
          
         retval = recv(msgsock,Buffer,sizeof (Buffer),0 );
         bytes_total += retval;

         if (fh != NULL)
            fwrite( (void *) Buffer, 1, retval, fh);
      }

      if (retval < 0 ) {
         fprintf(stderr,"recv() error\n");
            
         tick_end = GetTickCount();
         tick_diff = tick_end - tick_init;

         if (socket_type != SOCK_DGRAM){
            if (bytes_total != 0 && tick_diff != 0){
               mbps = ( (double)(bytes_total * 8)/(1024 * 1024) );
               mbps = mbps / ((double)tick_diff/1000);
            }
            else
               mbps = 0;

            printf("Acumulado %d Kb, en %d ms [%.3f mbps]\n", (bytes_total/1024), tick_diff, mbps);
         }
         
         close(msgsock);

         if (fh != NULL)
            fclose( fh);

         msgsock = 0;
         continue;
      }

      if (retval == 0) {
            
         tick_end = GetTickCount();
         tick_diff = tick_end - tick_init;
         
         if (bytes_total != 0 && tick_diff != 0){
             mbps = ( (double)(bytes_total * 8)/(1024 * 1024) );
             mbps = mbps / ((double)tick_diff/1000);
         }
         else
            mbps = 0;
         
         printf("Cliente cerro la conexion... Recibido %d KB en %d ms [%.3f mbps]\n\n", bytes_total/1024, tick_diff, mbps);
	 close(msgsock);

         if (fh != NULL)
            fclose( fh);

         msgsock = 0;
      }
      
      if (socket_type == SOCK_DGRAM) //{
         printf("Mensajes recibidos %d \n", bytes_total);
        
      continue;
   }

} //server



//**************************************************
// Client mode operation function.
//**************************************************
int client(char *i_server_name, unsigned short port, int socket_type, long sz_to_send, char *file_to_send){
   char Buffer[BUFF_LEN_CLI];
   int retval;
   int x;
   int m_continue = 1;
   int sz_sent = 0;
   int to_send = 0;
   unsigned int addr;
   struct sockaddr_in server;
   struct hostent *hp;
   int  conn_socket;
   double mbps;
   char server_name[255];
   FILE *fh;

   //Control de tiempo...
   long  tick_init;
   long  tick_end;
   long  tick_diff;

   strcpy(server_name, i_server_name);
   
   if (isalpha(server_name[0])) {   /* server address is a name */
      hp = gethostbyname(server_name);
   }
   else  { /* Convert nnn.nnn address to a usable one */
      addr = inet_addr(server_name);
      hp = gethostbyaddr((char *)&addr,4,AF_INET);
   }

   if (hp == NULL ) {
      if ( !isalpha(server_name[0]) ){ //server_name is an IP
         
         memset(&server,0, sizeof(server));
         
         server.sin_family = AF_INET;
         server.sin_port = htons(port);
         server.sin_addr.s_addr = inet_addr(server_name);

      }
      else{
         printf("No se ha podido resolver el Nombre del equipo\n");
         return 1;
      }

   }
   else{
      //
      // Copy the resolved information into the sockaddr_in structure
      //
      memset(&server,0,sizeof(server));
      memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
      server.sin_family = hp->h_addrtype;
      server.sin_port = htons(port);
   }

   conn_socket = socket(AF_INET,socket_type,0); /* Open a socket */
   if (conn_socket <0 ) {
      fprintf(stderr,"Client: Error abriendo socket: Error \n");
      return -1;
   }

   //Si no puedo resolver el Nombre
   if (hp != NULL){
      printf("Cliente conectandose a: %s %s:%d\n",hp->h_name, inet_ntoa(server.sin_addr), port);
   }
   else{
      printf("Cliente conectandose a: %s %s:%d\n",server_name, inet_ntoa(server.sin_addr), port );
   }

   if (connect(conn_socket,(struct sockaddr*)&server,sizeof(server)) ) {
      fprintf(stderr,"Error de Conexion\n");
      return -1;
   }

   //Abrir el archivo a transferir....
   if ( file_to_send != NULL ){
   
       if ( ( fh = fopen( file_to_send, "rb")) == NULL ){
          printf("Error abriendo archivo a enviar [%s]\n", file_to_send);
          fh = NULL;
       }
   }

   //Preparo buffer Random
   if ( socket_type != SOCK_DGRAM && fh == NULL){
      
      srand( (unsigned)GetTickCount() );
      
      for( x = 0 ; x < BUFF_LEN_CLI ; ++x )
         Buffer[x] = (char)rand();
      
   }
   else{
      strcpy( Buffer, "Prueba de transferencia UDP...");
   }
  
   //Si no es UDP
   if ( socket_type != SOCK_DGRAM){
      if (sz_to_send != 0){
         sz_to_send = sz_to_send * 1024;
         sz_sent = 0;
         to_send = 0;
      }
      else{
         sz_sent = 0;
         to_send = BUFF_LEN_CLI;
      }
   }

   //Si hay archivo sz_to_send....
   if (fh != NULL){
      fseek( fh, 0L, SEEK_END);
      sz_to_send = ftell(fh);
  
      //Vuelvo al principio....
      fseek( fh, 0L, SEEK_SET);
      
      sz_sent = 0;
      to_send = 0;

   }
   
   tick_init = GetTickCount();

   while(m_continue) {


      if (sz_to_send != 0 ){

         to_send = sz_to_send - sz_sent;

         //Lo que se envia no puede ser mayor que el Buffer
         if (to_send > BUFF_LEN_CLI)
             to_send = BUFF_LEN_CLI;

         //Lectura del archivo a enviar....
         if (to_send > 0 && fh != NULL){
            fread( (void *) Buffer, 1, to_send, fh );
         }
         
         if ( to_send <= 0){
            m_continue = 0;
            
            tick_end = GetTickCount();
            tick_diff = tick_end - tick_init;
            
            if (socket_type != SOCK_DGRAM){

               if (sz_to_send != 0 && tick_diff != 0){
                  mbps = ( (double)(sz_to_send * 8)/(1024 * 1024) );
                  mbps = mbps / ((double)tick_diff/1000);
               }
               else
                  mbps = 0;
               
               printf("Terminando la conexion...  %d Kb enviados en %d ms [%.3f mbps]\n", sz_sent/1024, tick_diff, mbps);

               if ( fh != NULL)
                  fclose(fh);
            }
            else
               printf("Terminando la transmision...  %d mensajes enviados\n", sz_to_send);
            break;
         }
            
      }
      
      //Enviar....
      if (socket_type != SOCK_DGRAM){
         retval = send( conn_socket, Buffer, (int)to_send, 0);
      }
      else{
         retval = send(conn_socket,Buffer, (int)strlen(Buffer), 0);
      }

      if (retval < 0) {
         fprintf(stderr,"send() error\n");
         
         if ( fh != NULL){

            fclose(fh);
            fh = NULL;
         }

         return -1;
      }

      //Si es UDP....
      if ( socket_type == SOCK_DGRAM)
         ++sz_sent;
      else
         sz_sent += to_send;

   } //while de envio

   if ( fh != NULL){
      fclose(fh);
      fh = NULL;
   }

   close(conn_socket);

   return 1;

} //client



//**********************************************
//  main
//**********************************************
int main(int argc, char **argv) {
   char *interface = NULL;
   char *server_name= "localhost";
   unsigned short port = DEFAULT_PORT;
   int is_server=0;
   int i;
   int socket_type = DEFAULT_PROTO;
   int sz_to_send = 1024;
   char *file_to_send;

   file_to_send = NULL;
   server_name  = NULL;

   if (argc == 1)
      Usage(argv[0]);

   if (argc >1) {
      for(i=1; i <argc; i++) {
         if ( (argv[i][0] == '-') || (argv[i][0] == '/') ) {
            switch(tolower(argv[i][1])) {
               
               case 'p':
                  if (!strncasecmp(argv[i+1], "TCP", (size_t)3 ) )
                     socket_type = SOCK_STREAM;
                  else if (!strncasecmp(argv[i+1], "UDP", (size_t)3) )
                     socket_type = SOCK_DGRAM;
                  else
                     Usage(argv[0]);

                  ++i;
                  break;

               case 'n':
                  server_name = argv[++i];
                  break;
					
               case 'e':
                  port = atoi(argv[++i]);
                  break;
					
               case 'l':
                  is_server = 1;
                  break;
               
               case 'i':
                  interface = argv[++i];
                  break;

               //Determinar el tama�o de data a enviar....
               case 's':
                  if (argv[i+1]) {
                     if (argv[i+1][0] != '-') 
                        sz_to_send = atoi(argv[i+1]);
                  }
                  else
                     Usage(argv[0]);

                  ++i;
                  break;
                  
               case 'f':
                  file_to_send = argv[++i];
                  break;
                  
               default:
                  Usage(argv[0]);
                  break;
            }
         }
         else
            Usage(argv[0]);
      }
   }
		
   if (port == 0){
      Usage(argv[0]);
   }

   if (sz_to_send == 0 && !is_server ){
      Usage(argv[0]);
   }
   
   if (is_server)
      server( port, socket_type, interface, argv[0], file_to_send);
   else
      client(server_name, port, socket_type, sz_to_send, file_to_send);
   
   return 1;

}  //main


//EOF

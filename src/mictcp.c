#include <mictcp.h>
#include <api/mictcp_core.h>

struct mic_tcp_sock sockets[10];
int num_sockets = 0;

int ports[10];
int sequences[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

float nb_envois = 0;
float nb_pertes = 0;

float perte_autorisee = 10.0;

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm) {

   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");

   int result = -1;
   result = initialize_components(sm); /* Appel obligatoire */
   if (result < 0) {
      return result;
   }
   set_loss_rate(10);

   sockets[num_sockets].fd = num_sockets;
   sockets[num_sockets].state = IDLE;

   //initialisation de l'adresse locale
   sockets[num_sockets].local_addr.ip_addr.addr = "localhost";
    if (strcmp(sockets[num_sockets].local_addr.ip_addr.addr, "localhost") != 0) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse locale du socket (addr)\n");
      return -1;
   }
   
   sockets[num_sockets].local_addr.ip_addr.addr_size = strlen(sockets[num_sockets].local_addr.ip_addr.addr);
    if (sockets[num_sockets].local_addr.ip_addr.addr_size != strlen(sockets[num_sockets].local_addr.ip_addr.addr)) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse locale du socket (addr_size)\n");
      return -1;
   }

   num_sockets++;
   return sockets[num_sockets-1].fd;

}
/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr) {

   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   printf("socket: %d\n",socket);

   //initialisation de l'adresse distante 
   // si l'adresse est NULL, c'est qu'on prend toutes les adresses 
   // de la machine "a travers" le port qu'on a selectionné 

   sockets[socket].remote_addr = addr;
   if(addr.ip_addr.addr != NULL){
       if (strcmp(sockets[socket].remote_addr.ip_addr.addr, addr.ip_addr.addr) != 0) {
         printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
         printf("Initialisation de l'adresse distante du socket\n");
         return -1;
      }
   }

   sockets[socket].local_addr.port = addr.port;
   ports[socket] = addr.port;
   if (sockets[socket].local_addr.port != addr.port) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse locale du socket (port)\n");
      return -1;
   }
   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   mic_tcp_pdu pdu_syn_ack;
   
   mic_tcp_pdu pdu_ack ;
   pdu_ack.header.syn = 0;
   pdu_ack.header.ack=1;
   pdu_ack.header.seq_num = sequences[socket];
   
   mic_tcp_pdu pdu_syn;
     pdu_syn.header.syn = 1;
   pdu_syn.header.ack=0;
   pdu_syn.header.seq_num = sequences[socket];
   while(1){
       int effective_receive = ip_recv(pdu_syn, addr, 1000);
       if(effective_receive!=-1 && pdu_syn.header.syn == 1){
          pdu_syn_ack.header.syn = 1;
          pdu_syn_ack.header.ack=1;
         pdu_syn_ack.header.seq_num = sequences[socket];
         }else{
            //gerer la erreur de reception du syn ack?? 
            return -1;
         }
        int effective_send = IP_send(pdu_syn_ack, addr->ip_addr);
        effective_receive  = IP_recv(&pdu_ack, addr, 1000); 
         if(effective_receive == 0 && pdu_ack.header.ack ==1){
            sockets[socket].state=ESTABLISHED;
         }
         else {
            return -1;
         }
   }  

    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr) {
  
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   
   sockets[socket].remote_addr = addr;
   if (strcmp(sockets[socket].remote_addr.ip_addr.addr, addr.ip_addr.addr) != 0) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse distante du socket\n");    printf("a t on terminé buffer get??\n");

      return -1;
   }


   mic_tcp_pdu pdu_syn_ack;
   pdu_syn_ack.header.syn = 1;
   pdu_syn_ack.header.ack=1;
   pdu_syn_ack.header.seq_num = sequences[socket];
   mic_tcp_pdu pdu_syn;
   pdu_syn.header.syn = 1;
   pdu_syn.header.ack=0;
   pdu_syn.header.seq_num = sequences[socket];
   IP_send(pdu_syn,addr.ip_addr);
  
   if(ip_recv(pdu_syn_ack, addr, 1000 ) == -1){
      return -1;
   }
  
   if(pdu_syn_ack.header.syn == 1 && pdu_syn_ack.header.ack == 1){
      mic_tcp_pdu pdu_ack;
      pdu_ack.header.syn=0;
      pdu_ack.header.ack=0;
      pdu_ack.header.ack_num=pdu_syn_ack.header.seq_num;
      IP_send(pdu_ack,addr.ip_addr);
      sockets[socket].state = ESTABLISHED;
   return 0;

   }
  

   

}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size) {

   printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
   
   struct mic_tcp_pdu pdu;
    
   pdu.header.source_port = ports[mic_sock];
   pdu.header.dest_port = ports[mic_sock];

   printf("[MIC-TCP] PSE: %d\n", sequences[mic_sock]);
   
   pdu.header.seq_num = sequences[mic_sock];
   pdu.header.ack_num = 0;
   pdu.header.syn = 0;
   pdu.header.ack = 0;
   pdu.header.fin = 0;

   pdu.payload.data = mesg;
   pdu.payload.size = mesg_size;
   printf("%d\n",mesg_size);

   int effective_send=IP_send(pdu, sockets[mic_sock].remote_addr.ip_addr);
   nb_envois++;

   struct mic_tcp_pdu ack;
   struct mic_tcp_ip_addr *local_addr = &sockets[mic_sock].local_addr.ip_addr;
   struct mic_tcp_ip_addr *remote_addr = &sockets[mic_sock].remote_addr.ip_addr;
   ack.header.ack_num = -1;
   ack.payload.data = " ";
   ack.payload.size = 1;
   int ack_rcv;
   int ack_send = 1;

   printf("pourcentage de pertes %f\n", (nb_pertes/nb_envois)*100.0);
   printf("nb pertes %f\n", nb_pertes);
   printf("nb envois %f\n", nb_envois);

   // tant qu'on ne recoit pas le ack avec le bon numero de seq
   while (ack.header.seq_num != sequences[mic_sock] && ack_send) {
      printf("in\n");
      ack_rcv = IP_recv(&ack, local_addr, remote_addr, 1000);
      //si expiration du timer, on renvoie notre pdu.
      if (ack_rcv == - 1) {
         printf("in2\n");
         nb_pertes++;
         if ((nb_pertes/nb_envois)*100 > perte_autorisee) {
            printf("in3\n");
            IP_send(pdu, sockets[mic_sock].remote_addr.ip_addr);
            nb_envois++;
         } else {
            ack_send = 0;
         }
      }
   }
   
   if (ack_send) {
      printf("[MIC-TCP] ACK %d reçu\n", sequences[mic_sock]);
      sequences[mic_sock]++;
   }

   return effective_send;

}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size){
   
   int result = -1;
   mic_tcp_payload pl;
   pl.data = mesg;
   pl.size = max_mesg_size;
   
   result = app_buffer_get(pl);

   return result; 
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    sockets[socket].state = CLOSED;
    return -1;

}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_ip_addr local_addr, mic_tcp_ip_addr remote_addr) {

   printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
   mic_tcp_payload pl = pdu.payload;
   mic_tcp_header header = pdu.header;

   printf("[MIC-TCP] PSA : %d\n", sequences[num_sockets]);

   mic_tcp_pdu ack;
   ack.header.ack = 1;
   ack.header.syn = 0;
   ack.header.fin = 0;
   ack.header.seq_num = header.seq_num;
   ack.header.dest_port = pdu.header.source_port;
   ack.header.source_port = pdu.header.dest_port;

   IP_send(ack, remote_addr);

   if (header.seq_num == sequences[num_sockets]) {

      app_buffer_put(pl);

      sequences[num_sockets]++;
      printf("[MIC-TCP] PSA updated: %d\n", sequences[num_sockets]);

   }

}
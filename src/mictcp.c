#include <mictcp.h>
#include <api/mictcp_core.h>

struct mic_tcp_sock sockets;
int num_sockets = 0;

int ports[10];
//tableau contenant les sequences de associés a nos sockets
int sequences[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

float nb_envois = 0;
float nb_pertes = 0;

float perte_autorisee = 0;

//"flag" de reception du syn pour pdu_process_received
int syn_received = 0;

//"flag" de reception du ack pour pdu_process_received
int ack_received = 0;

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

   if (sm == CLIENT) perte_autorisee = 12;
   if (sm == SERVER) perte_autorisee = 14;

   sockets.fd = num_sockets;
   sockets.state = IDLE;

   //initialisation de l'adresse locale
   sockets.local_addr.ip_addr.addr = "localhost";

   //verifications
    if (strcmp(sockets.local_addr.ip_addr.addr, "localhost") != 0) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse locale du socket (addr)\n");
      return -1;
   }
   
   sockets.local_addr.ip_addr.addr_size = strlen(sockets.local_addr.ip_addr.addr);
    if (sockets.local_addr.ip_addr.addr_size != strlen(sockets.local_addr.ip_addr.addr)) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse locale du socket (addr_size)\n");
      return -1;
   }

   
   return sockets.fd;

}
/*
 * Permet d'associer notre socket a son adresse LOCALE.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr) {

   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   printf("socket: %d\n",socket);

   //initialisation de l'adresse distante 
   // si l'adresse est NULL, c'est qu'on prend toutes les adresses 
   // de la machine "a travers" le port qu'on a selectionné 

   sockets.local_addr = addr;
   if(addr.ip_addr.addr != NULL){
      if (strcmp(sockets.local_addr.ip_addr.addr, addr.ip_addr.addr) != 0) {
         printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
         printf("Initialisation de l'adresse distante du socket\n");
         return -1;
      }
      if (sockets.local_addr.port != addr.port) {
         printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
         printf("Initialisation de l'adresse locale du socket (port)\n");
         return -1;
      }
   } 
   return 0;
}
/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr) {

   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   
   // étape 1: attente du syn 
   while (syn_received != 1) { 
      printf("[MIC-TCP] Attente de demande de synchronisation\n");
      sleep(1);
   } 
   // étape 2: reception du syn 
   printf("[MIC-TCP] Demande de synchronisation reçue\n");

   // étape 3: construction du synack 
   mic_tcp_pdu syn_ack;
   syn_ack.header.syn = 1;
   syn_ack.header.ack = 1;
   syn_ack.header.ack_num = 0;
   syn_ack.header.dest_port = sockets.remote_addr.port;
   syn_ack.header.fin = 0;
   syn_ack.header.seq_num = 0;
   syn_ack.header.source_port = sockets.local_addr.port;
   //! on fait passer le pourcentage de perte autorissée par la taille du payload
   syn_ack.payload.size = perte_autorisee; 
   syn_ack.payload.data = malloc(syn_ack.payload.size);
   //taille de la donnée applicative correspondant au pourcentage de pertes autorisé
   memset(syn_ack.payload.data, 'X', (int) perte_autorisee);

   //étape 4: envoi du synack
   IP_send(syn_ack, sockets.remote_addr.ip_addr);
   
   //"flag" de reception du syn pour pdu_process_received
   syn_received = 0;
   
   // étape 5:  attente du ack
   while (ack_received != 1) { 
      printf("[MIC-TCP] Attente d'acknowledgment de fin de connexion \n");
      sleep(1);
      if (ack_received != 1) {
         printf("[MIC-TCP] Acknowledgment de synchronisation envoyée:étape fin de connexion\n");
         IP_send(syn_ack, sockets.remote_addr.ip_addr);
      } 
   } 
      //"flag" de reception du syn pour pdu_process_received
      ack_received = 0;
   
   sockets.state = ESTABLISHED;

   return 0;

}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr) {
  
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   
   //adresse a laquelle on souhaite envoyer la demande 
   sockets.remote_addr = addr;
   if (strcmp(sockets.remote_addr.ip_addr.addr, addr.ip_addr.addr) != 0) {
      printf("[MIC-TCP] Erreur dans : ");  printf(__FUNCTION__); printf("\n");
      printf("Initialisation de l'adresse distante du socket\n");    printf("a t on terminé buffer get??\n");

      return -1;
   }
   
   // étape 1: construction des PDU SYN et ACK
   //PDU syn
   mic_tcp_pdu syn;
   syn.header.syn = 1;
   syn.header.ack = 0;
   syn.header.ack_num = 0;
   syn.header.dest_port = sockets.remote_addr.port;
   syn.header.fin = 0;
   syn.header.seq_num = 0;
   syn.header.source_port = sockets.local_addr.port; 
   syn.payload.size= (int) perte_autorisee;
   //! on fait passer le pourcentage de perte autorissée par la taille du payload   syn.payload.size = perte_autorisee;
   syn.payload.data = malloc(syn.payload.size);
   //on remplit data pour que data aie la taille de perte_autorisé
   memset(syn.payload.data, 'X', (int) perte_autorisee);
   
   //PDU ack
   mic_tcp_pdu ack;
   ack.header.ack = 1;
   ack.header.syn = 0;
   ack.header.ack_num = 0;
   ack.header.dest_port = sockets.remote_addr.port;
   ack.header.fin = 0;
   ack.header.seq_num = 0;
   ack.header.source_port = sockets.local_addr.port; 
   ack.payload.data = NULL;
   ack.payload.size = 0;
   
   // étape 2: envoi du PDU SYN
   IP_send(syn, addr.ip_addr);
   printf("[MIC-TCP] Demande de synchronisation envoyée\n");

   //étape 3: attente de reception du synack
   
   //Passage de notre socket en mode attente du synack
   sockets.state = WAITING_FOR_SYN_ACK;
   
   //PDU syn_ack que l'on s'attend a recevoir
   mic_tcp_pdu syn_ack;
      
   //structure d'une adresse IP pour l'IP locale
   mic_tcp_ip_addr local;
   //structure d'une adresse IP pour l'IP distante 
   mic_tcp_ip_addr remote;
   //tableau de caracteres representant l'IP locale
   char local_ip[64];
   //tableau de caracteres representant l'IP distante
   char remote_ip[64];
   
   //addresse de l'IP locale pour notre structure local
   local.addr = local_ip;
   
   //addresse de l'IP remote pour notre structure remote
   remote.addr = remote_ip;
   
   //taille de l'IP locale
   local.addr_size = sizeof(local_ip);
   // taille de l'IP remote 
   remote.addr_size = sizeof(remote_ip);

   //taille de notre futur PDU recu (-1 si erreure)
   int effective_receive = 0; 
   
   //nombre de renvois du syn 
   int nb_tries = 0;
   
   //nombres de renvois maximal du syn avant echec d'etablissement de la connexion
   int max_tries = 20;

   printf("[MIC-TCP] Attente d'acknowledgment de synchronisation\n");
   while (nb_tries < max_tries && sockets.state == WAITING_FOR_SYN_ACK) {

      printf("[MIC-TCP] Envoi du SYN (tentative %d)\n", nb_tries + 1);
      
      //envoi du syn
      IP_send(syn, addr.ip_addr);

      // on se met en etat d'attente de notre syn ack
      effective_receive = IP_recv(&syn_ack, &local, &remote, 1000);

      // traitement du cas ou on recoit le syn ack
      if (effective_receive != -1) {

         printf("[MIC-TCP] PDU reçu pendant tentative %d\n", nb_tries + 1);
         // traitemnt de la reception du syn ack
         process_received_PDU(syn_ack, local, remote);

         if (syn_ack.header.syn == 1 && syn_ack.header.ack == 1) {
            printf("[MIC-TCP] SYN-ACK reçu\n");
            //envoi du ack
            IP_send(ack, addr.ip_addr);
            //changemnet du socket state
            sockets.state = ACK_SENT;
            printf("[MIC-TCP] Acknowledgment envoyé \n");
            }
         //traitement du cas ou ne recoit pas le syn ack pendant le timeout
         } 
         else {
         printf("[MIC-TCP] Timeout... aucun PDU reçu\n");
         }

      nb_tries++;

   }
   // si on n'a toujours pas le syn_ack apres nos max_tries essai, on declare un echec de connexion
   if (sockets.state == WAITING_FOR_SYN_ACK) {
      return -1;
   }
   
   printf("Pourcentage de pertes autorisé : %f\n",perte_autorisee);
   //update des flags pour pdu_process received
   syn_received = 0;
   ack_received = 0;

   return 0;

}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size) {

   printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
   //donnée enovoyée, egale au nombre d'octets envoyés, -1 si erreur

   //notre pdu a envoyer
   struct mic_tcp_pdu pdu;
   //consturction du PDU
   
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

   //on envoi le PDU via IP
   int effective_send=IP_send(pdu, sockets.remote_addr.ip_addr);
   
   nb_envois++;
   //construction du ACK
   struct mic_tcp_pdu ack;
   struct mic_tcp_ip_addr *local_addr = &sockets.local_addr.ip_addr;
   struct mic_tcp_ip_addr *remote_addr = &sockets.remote_addr.ip_addr;
   ack.header.ack_num = -1;
   ack.payload.data = " ";
   ack.payload.size = 1;
   //stockage du ACK que l'on s'attend a recevoir
   int ack_rcv;
   
   //"flag" pour l'envoi de notre ACK
   int ack_send = 1;

   printf("Pourcentage de pertes %f/%f\n", (nb_pertes/nb_envois)*100.0, perte_autorisee);

   // tant qu'on ne recoit pas le ack avec le bon numero de sequence
   while (ack.header.seq_num != sequences[mic_sock] && ack_send) {
      ack_rcv = IP_recv(&ack, local_addr, remote_addr, 1000);
      //si expiration du timer, on renvoie notre pdu.
      if (ack_rcv == - 1) {
         //incrementation du nombre de pertes 
         nb_pertes++;

         // si on est toujours sur un nombre de pertes acceptable, on renvoi
         if ((nb_pertes/nb_envois)*100 > perte_autorisee) {
            IP_send(pdu, sockets.remote_addr.ip_addr);
            nb_envois++;
         
         }
         //sinon on ne renvoi pas  
         else {
            ack_send = 0;
         }
      }
   }
   
   if (ack_send) {
   //si on a pu renvoyer le paquet car on avait un taux de pertes acceptable:
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
    sockets.state = CLOSED;
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

   printf("[MIC-TCP] SYN : %d\n", header.syn);
   printf("[MIC-TCP] ACK : %d\n", header.ack);
   //gestion de la reception d'un synack
   if (header.syn == 1 && header.ack == 1) {
      printf("[MIC-TCP] PDU SYN-ACK reçu.\n");
      //on set nos flags
      syn_received = 1;
      ack_received = 1;
      return;
   //gestion de la reception d'un syn
   } else if (header.syn == 1 ) {
      printf("[MIC-TCP] PDU SYN reçu.\n");
      //set du flag
      syn_received = 1;
      sockets.remote_addr.ip_addr = remote_addr;

      if (perte_autorisee > pl.size) perte_autorisee = (float) pl.size;
      return;
   // gestion de la reception d'un ack
   } else if (header.ack == 1) {
      printf("[MIC-TCP] PDU ACK reçu.\n");
      //set du flag
      ack_received = 1;

      perte_autorisee = (float) pl.size;
      return;
   } 

   printf("[MIC-TCP] PSA : %d\n", sequences[num_sockets]);
   // construction d'un ack
   mic_tcp_pdu ack;
   ack.header.ack = 1;
   ack.header.syn = 0;
   ack.header.fin = 0;
   ack.header.seq_num = header.seq_num;
   ack.header.dest_port = pdu.header.source_port;
   ack.header.source_port = pdu.header.dest_port;
   //envoi d'un ack a l'addresse distante
   IP_send(ack, remote_addr);
   //si les numeros de sequences correspondent, on peut remplir notre buffer.
   if (header.seq_num == sequences[num_sockets]) {

      app_buffer_put(pl);
      //on update sequences
      sequences[num_sockets]++;
      printf("[MIC-TCP] PSA updated: %d\n", sequences[num_sockets]);

   }

}

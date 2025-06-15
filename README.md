# BE RESEAU

L’objectif de ce bureau d'études et la conception et le développement d'un protocole de transport MIC-TCP, visant à transporter un flux de messages textuels ou vidéo en temps réel. 

## Versions implémentées 

Nous avons implémenté 4 versions différentes du protocole 

### Version 1 

La v1 doit implémenter une phase de transfert de données sans garantie de fiabilité.

### Version 2 

La v2 doit étendre la phase de transfert de données MICTCP-v1 de sorte à inclure une garantie de fiabilité totale via un mécanisme de reprise des pertes de type Stop and Wait.

### Version 3

La v3 doit inclure une garantie de fiabilité partielle statique via un mécanisme de reprise des pertes de type Stop and Wait à fiabilité partielle pré-câblée, i.e. dont le % de pertes admissibles est défini de façon statique.

### Version 4.1 

La v4.1 doit inclure une phase d’établissement de connexion et une garantie de fiabilité partielle via un mécanisme de reprise des pertes de type « Stop and Wait » dont le % de pertes admissibles sera négocié durant la phase d’établissement de connexion.

**NOTE:** La v4.1 peut souffrir de problèmes de réception du PDU SYN_ACK lors de la phase de synchronisation, si vous encontrez ce problème, il faudra relancer le protocole. 

## Compilation du protocole MIC-TCP

Pour compiler mictcp et générer les exécutables des applications de test depuis le code source fourni, taper :

    make

Deux applicatoins de test sont fournies, tsock_texte et tsock_video, elles peuvent être lancées soit en mode puits, soit en mode source selon la syntaxe suivante:

    Usage: ./tsock_texte [-p|-s destination] port
    Usage: ./tsock_video [[-p|-s] [-t (tcp|mictcp)]

Seul tsock_video permet d'utiliser, au choix, votre protocole mictcp ou une émulation du comportement de tcp sur un réseau avec pertes.

## 

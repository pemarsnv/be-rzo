# BE RESEAU

L’objectif de ce bureau d'études et la conception et le développement d'un protocole de transport MIC-TCP, visant à transporter un flux de messages textuels ou vidéo en temps réel. 

***

## Versions implémentées 

Nous avons implémenté 4 versions différentes du protocole, toutes venant ajouter progressivement à la qualité de service du protocole. 

### Version 1 

La v1 implémente une phase de transfert de données sans garantie de fiabilité.

### Version 2 

La v2 étend la précédente phase de transfert de données MICTCP-v1, de sorte à inclure une garantie de fiabilité totale avec un mécanisme de reprise des pertes de type Stop and Wait.

### Version 3

La v3 inclut une garantie de fiabilité partielle via un mécanisme de reprise des pertes de type Stop and Wait à fiabilité partielle, avec un pourcentage de pertes admissibles défini de manière statique.

### Version 4.1 

La v4.1 inclut une phase d’établissement de connexion, et modifie la définition du pourcentage de pertes admissibles pour la remplacer par une négociation lors de la phase de connexion. 

***

## Compilation du protocole MIC-TCP

Pour compiler mictcp et générer les exécutables des applications de test depuis le code source fourni, taper :

    make

Deux applicatoins de test sont fournies, tsock_texte et tsock_video, elles peuvent être lancées soit en mode puits, soit en mode source selon la syntaxe suivante:

    Usage: ./tsock_texte [-p|-s destination] port
    Usage: ./tsock_video [[-p|-s] [-t (tcp|mictcp)]

***

## Problèmes rencontrés 

- Notre protocole ne fonctionne pas avec la transmission vidéo, uniquement avec la transmission textuelle.
- La v4.1 peut souffrir de problèmes de réception du PDU SYN_ACK lors de la phase de synchronisation, si vous encontrez ce problème, il faudra relancer le protocole. 

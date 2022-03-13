# Spidal

Chronometre pour l'escalade de vitesse

![Logo Spidal](https://github.com/RemyDammaretz/spidal/blob/master/confs/GUIConfs/splash.png?raw=true)


# Installation

## I - Matériel
Pour utiliser le projet, il faut avoir 2 Raspberry Pi : 
 - 1 pour le module au sol : Spidal
 - 1 pour les boutons poussoirs au sommet de la piste d'escalade : Spidal Remote

 

## II - Module Spidal

Configuration du module Spidal au sol :

### 1) Installation des paquets
Sur le module au sol Spidal, installez les modules suivants : 

 - **NTP** pour la synchronisation temporelle des deux modules :
`sudo apt-get install ntp`
- **Hostapd** et **Dnsmasq** pour faire du Raspberry Pi Spidal un point d'accès Wifi :
`sudo apt-get install hostapd`
`sudo apt-get install dnsmasq`
- **GTK** pour l'application graphique :
`sudo apt-get install libgtk-3-dev`
- **mpg123** pour jouer les sons de l'application graphique :
`sudo apt-get install mpg123`

### 2) Création des exécutables 
#### Récupération du projet
Clonez le projet `spidal` dans le répertoire `/home/pi` . Vous devez obtenir l'architecture suivante :

    /home/pi/spidal/
                   \_ programs/
                   \_ confs/

#### Configuration
Vous pouvez éditer le fichier `/home/pi/spidal/programs/config/spial.h` en fonction du hardware utilisé.
Les options possibles sont les suivantes :

 - **PIN_PEDAL_i** pour indiquer les pin GPIO où sont branchés les pédales (par défaut 16 et 13)
 - **PEDAL_i_PULL_UP_RESISTOR** pour spécifier si on utilise une résistance de pull-up interne au Raspberry Pi
     - PULL_UP si vous souhaitez utiliser une résistance de pull-up interne (peut ne pas marcher en fonction du modèle de Raspberry Pi utilisé).
     - NO_PULL_UP si vous souhaitez ne pas utiliser de résistance de pull-up interne. Il faut alors en mettre une soi-même dans le circuit électrique.
 - **PEDAL_i_REVERSE_INPUT** pour inverser logiciellement la valeur ON/OFF des interrupteurs (peut être utile dans le cas de l'utilisation de boutons de type arrêt d'urgence où un appui sur le bouton ouvre le circuit électrique au lieu de le fermer).
    - NO_REVERSE_INPUT : pour ne pas inverser (circuit électrique fermé <=> ON)
    - REVERSE_INPUT : pour inverser (circuit électrique fermé <=> OFF)

#### Compilation
Pour compiler le projet rendez-vous dans le répertoire `/home/pi/spidal/programs/` et exécutez la commande suivante (vous avez besoin des privilèges administrateur) :

    sudo make spidal

Les exécutables crées se trouvent dans le répertoire `/home/pi/spidal/programs/bin/app/`.
Vous pouvez maintenant tester si l'application se lance correctement en exécutant le programme : 
 `/home/pi/spidal/programs/bin/app/spidal`

### 3) Configuration du Raspberry PI

Pour jouer les scripts de configuration, rendez-vous dans le répertoire `/home/pi/spidal/confs/`.

#### Configuration NTP
Pour activer le protocole de synchronisation temporelle NTP exécutez la commande suivante :

    ./confNTPServer.sh enable
    
#### Configuration du point d'accès Wifi
Vous pouvez éditer le fichier `/home/pi/spidal/confs/wifiConfs/hostapd.conf` pour régler les options suivantes :
- Le nom ssid du réseau Wifi qui sera crée par le Raspberry PI. La valeur par défaut est *spidal2021*.
`ssid=spidal2021`
- Le mot de passe du réseau Wifi. La valeur par défaut est *raspberry*.
`wpa_passphrase=raspberry`

Ensuite, pour appliquer la configuration,  exécutez la commande suivante :

    ./confWifiAccess.sh enable

#### Configuration du démarrage automatique du programme
Pour que le programme se lance automatiquement au démarrage du Raspberry PI, exécutez la commande suivante :

        ./confGUI.sh enable
Il faut ensuite configurer le Raspberry PI pour démarrer en **mode console autologin**.
Vous pouvez trouver ce réglage dans le menu de configuration du Raspberry dans l'interface graphique ou bien en exécutant la commande `sudo raspi-config`.
Vous pouvez ensuite redémarrer le Raspberry Pi pour vérifier que le programme se lance bien au démarrage.

---
#### Remettre les configurations par défaut
Vous pouvez toujours à tout moment désactiver les configurations et remettre le Raspberry PI dans son état précédent en appelant les scripts de configuration précédents avec l'option *disable*.

    ./confNTPServer.sh disable
    ./confWifiAccess.sh disable
    ./confGUI.sh disable

## III - Module Spidal Remote

Configuration du module au sommet de la piste d'escalade Spidal Remote


### 1) Installation des paquets
Sur le module au sol Spidal Remote, installez le module suivant : 

 - **NTP** pour la synchronisation temporelle des deux modules :
`sudo apt-get install ntp`


### 2) Création des exécutables 
#### Récupération du projet

De la même manière que pour le module Spidal, clonez le projet `spidal` dans le répertoire `/home/pi` .

#### Configuration
Vous pouvez éditer le fichier `/home/pi/spidal/programs/config/remote.h` en fonction du hardware utilisé.
Les options possibles sont les suivantes :

 - **PIN_BTN_i** pour indiquer les pin GPIO où sont branchés les pédales (par défaut 16 et 13)
 - **BTN_i_PULL_UP_RESISTOR** pour spécifier si on utilise une résistance de pull-up interne au Raspberry Pi
     - PULL_UP si vous souhaitez utiliser une résistance de pull-up interne (peut ne pas marcher en fonction du modèle de Raspberry Pi utilisé).
     - NO_PULL_UP si vous souhaitez ne pas utiliser de résistance de pull-up interne. Il faut alors en mettre une soi-même dans le circuit électrique.
 - **BTN_i_REVERSE_INPUT** pour inverser logiciellement la valeur ON/OFF des interrupteurs (peut être utile dans le cas de l'utilisation de boutons de type arrêt d'urgence où un appui sur le bouton ouvre le circuit électrique au lieu de le fermer).
    - NO_REVERSE_INPUT : pour ne pas inverser (circuit électrique fermé <=> ON)
    - REVERSE_INPUT : pour inverser (circuit électrique fermé <=> OFF)

#### Compilation
Pour compiler le projet rendez-vous dans le répertoire `/home/pi/spidal/programs/` et exécutez la commande suivante (vous avez besoin des privilèges administrateur) :

    sudo make remote

### 3) Configuration du Raspberry PI

Pour jouer les scripts de configuration, rendez-vous dans le répertoire `/home/pi/spidal/confs/`.

#### Configuration NTP
Pour activer le protocole de synchronisation temporelle NTP exécutez la commande suivante :

    ./confNTPClient.sh enable

#### Configuration du démarrage automatique
Pour activer le lancement automatique du programme au démarrage du Raspberry PI, exécutez la commande suivante :

    ./confRemoteStart.sh enable

De la même manière que pour le module Spidal, vous pouvez désactiver l'interface graphique du Raspberry PI en configurant le mode de démarrage **console autologin** pour gagner en performances mais ce n'est pas obligatoire.

#### Configuration de la connexion avec le module au sol Spidal

Pour que les deux modules Spidal et Spidal Remote puissent se connecter, il suffit de connecter le module Spidal Remote au réseau Wifi du module Spidal. (SSID par défaut : *spidal2021*, mot de passe par défaut *raspberry*).

---
#### Remettre les configurations par défaut
Vous pouvez toujours à tout moment désactiver les configurations et remettre le Raspberry PI dans son état précédent en appelant les scripts de configuration précédents avec l'option *disable*.

    ./confNTPClient.sh disable
    ./confRemoteStart.sh disable

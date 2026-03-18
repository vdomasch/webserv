# Webserv
## Description

Ce projet est un serveur HTTP C++ développé à partir de zéro dans le cadre du programme 42.
Il prend en charge HTTP/1.1, gère plusieurs clients à l’aide d’E/S non bloquantes et de select(), et implémente des méthodes comme GET, POST et DELETE, ainsi que des blocs de serveur et d’emplacements configurables inspirés par Nginx.

## Fonctionnalités principales
 - Configuration des sockets pour une connection simultannees de multiples clients
 - Methodes GET/POST/DELETE
 - Gestion des CGI
 - Implementation d'un proxy inspire de Nginx

## Technologies
 - Frontend : HTML, CSS
 - Backend : C++, PHP, Python

## Objectif du projet
Ce projet vise à comprendre et développer un serveur HTTP en C++.

## Lancer le projet
```bash
> make
```
L’application sera ensuite accessible depuis votre navigateur à l'adresse localhost sur le port 9090 (personnalisable dans le server.conf).

## Auteurs
Projet réalisé dans le cadre du cursus 42 School.
 - Benjamin HUMEAU (https://github.com/Ibaos)
 - Oxigety (https://github.com/Oxigety)
 - Vincent DOMASCHIO (https://github.com/vdomasch)

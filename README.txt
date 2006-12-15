;; Jean-marie Codol 
;; jmecodol@gmail.com
;; Guillaume Sauvajol
;; guillaume1s@gmail.com


;; comment utiliser ce generateur :
-----------------------------------

;; pour voir un exemple complet d utilisation,
;; regarder le fichier demonstration.txt !!

;; les fichier .svn/* servent a lier le repertoire avec un repository sur code.google.com/
;; gratuitement .

1 --> aller dans java .
2 --> editer le fichier petit-dictionnaire.txt
   --> chaque ligne correspond a un mot a ajouter au dictionnaire
3 --> taper java -jar petit-dictionnaire.txt -lisp > automate.txt
4 --> aller dans clisp "clisp -C" pour avoir plus de memoire .
5 --> charger toutes les classes avec 
(load "gene/charger-tout.lisp")
6 -->
(make-vm 'toto 50)
7 -->
(generer-code "gene/java/automate.txt" "gene/code.asm")
8 -->
(charger-code 'toto "gene/code.asm")
9 -->
(charger-mot 'toto '(a b c d))
10 --> 
(execute-vm 'toto)
11 --> si l'ecran affiche T, c est que le mot est reconnu
---------- si NIL, le mot n est pas reconnu .

12 --> on peut revenir au dernier resultat avec (get-resultat 'toto)





trucs utiles a savoir .
dans les .lisp,
il y a de variables debug T ou ()
---------------------------------

-- la resolution d etiquette se fait en 1 passage,
-- en utilisant 2 tableaux .
-- (etiquettes connues et inconnues)


la fonction :
(affiche-tableau 'toto) ==> affiche la ram de 'toto !



;; ATTENTION !!
;; il ne FAUT PAS D ACCENT .

donc il faut éditer soit le dictionnaire,
soit l'automate, pour renplacer les é par e, ...

et ensuite appeller generer-code dans "clisp -C" .



Licence GNU GPL V2 

Jean-marie Codol 
jmecodol@gmail.com
Guillaume Sauvajol
guillaume1s@gmail.com



--

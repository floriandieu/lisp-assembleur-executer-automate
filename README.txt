
;; comment utiliser ce generateur :





1 --> aller dans java .
2 --> editer le fichier petit-dictionnaire.txt
   --> chaque ligne correspond a un mot a ajouter au dictionnaire
3 --> taper java -jar petit-dictionnaire.txt -lisp > automate.txt
4 --> aller dans clisp 
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


trucs utiles a savoir .
dans les .lisp,
il y a de variables debug T ou ()
---------------------------------
la fonction :
(affiche-tableau 'toto) ==> affiche la ram de 'toto !


Licence GNU GPL V2 

Jean-marie codol 
jmecodol@gmail.com

--
a partir d'un automate déterministe .

generer de l assembleur pour reconnaitre un mot,

charger cet assembleur dans une machine virtuelle créée en lisp,

nécéssite de charger le créateur de VM, l'exécuteur de code assembleur, ...

charger le code assembleur généré.
charger un mot quelconque.

et exécuter avec lisp le code assembleur,

rendre le résultat dans un registre R de la machine virtuelle .


on peut revenir sur le résultat ou charger un autre mot.


durée 3 mois à 2 élèves.
note obtenue: 14/20.
+ notions de lisp.
+ notions de machines virtuelles.
+ excellents cours de Mr. Lafourcade.

GPL 2.0



exemple d'utilisation:

(load "charger-tout.lisp")

(make-vm 'toto 100)

(charger-code 'toto "code.asm")

(charger-mot 'toto '(s a l u t))

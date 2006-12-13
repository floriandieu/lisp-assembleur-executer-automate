
;; version 0.1

;; auteur jean-marie
;; date 02-12-2006
;; release notes :--
;; dans le sous repertoire          "gene/"
;; - charger les 3 fichiers 
;;    - make-vm.lsp
;;    - loader.lisp
;;    - executeur.lisp



(load "make-vm.lisp")
(load "loader.lisp")
(load "executeur.lisp")
(load "generer-code.lisp")

;; aide contextuelle:
(defun usage () 
	(progn 
		(write-line "generation de code")
		(write-line " fonctions principales :")
		(write-line "  - (make-vm 'nom 'taille)")
		(write-line "  - (generer-code \"fichier\" &optionnal \"fichier-sortie\")")
		(write-line "  - (charger-code 'nom-vm \"fichier\")")
		(write-line "  - (charger-mot 'nom-vm 'liste-lettres)")
		(write-line "  - (affiche-tableau 'nom-vm)")
		(write-line "  - (execute-vm 'nom-vm)")
		(write-line "  - (get-resultat 'nom-vm)")
		T))

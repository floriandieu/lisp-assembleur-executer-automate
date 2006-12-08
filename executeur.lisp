
;; executeur.lisp

;; execute les instructions d une V.M.

;; version 0.2

;; auteur jean-marie
;; date 02-12-2006
;; release notes :--
;;  recuperer l exception "vm trop petite : impossible d executer la ligne xxx"



;; version 0.1

;; auteur jean-marie
;; date 02-12-2006
;; release notes :--
;; les instructions traitees sont :
;;       - JMP xxx
;;       - STOP
;;       - JEQ R0 xxx
;;       - CMP R0 xxx
;;       - LOAD xxx

;; xxx sont des valeurs


;; l executeur execute les instructions assemleur de la machine virtuelle
;; dans l'ordre d'execuction .

;; saute a la ligne suivante ( ou indiquee selon l instruction )

;;_________________________________
;;_________________________________
;; quelques variables globales importantes
;; si on est en mode debug :
(setf execute-debug ())


(defun execute-vm (vm &optional (ligne-a-executer '0))
  (progn
    (if (>= ligne-a-executer (get vm ':ram)) (error "impossible de lire la ligne ~S : depassement de la taille de la ram disponible ([0 .. ~S])" ligne-a-executer (- (get vm ':ram) 1)))
    (if execute-debug
	(progn
	  (princ ligne-a-executer)
	  (princ " : execution de ")
	  (princ (get-tableau vm ligne-a-executer))
	  (write-line "")))
    (cond 
     ;; saut simple,
     ;; saute a l instruction definie dans le 'cdr' et execute cette instruction
        ((eql (car (get-tableau vm ligne-a-executer)) 'JMP) 
	 (execute-vm vm (cdr (get-tableau vm ligne-a-executer))))
	
	;; l instruction STOP arrete la vm
	;; en renvoyant le code de succes T
	;; le T devrai s afficher a l'ecran
        ((eql (car (get-tableau vm ligne-a-executer)) 'STOP) 
	 (progn (if execute-debug (write-line "execution terminee avec succes")) T) )
	
	;; saut conditionnel
	;; si le flag d'egalite de la machine virtuelle, saute au 'cdr'
	;; sinon execute l instruction suivante .
        ((eql (car (get-tableau vm ligne-a-executer)) 'JEQ)
	 (let ((ligne-a-laquelle-aller-si-egal (cadr (get-tableau vm ligne-a-executer)))
	       (ligne-a-laquelle-aller-si-different (+ 1 ligne-a-executer)))
	   (if (get-reg-equal vm)
	       (execute-vm vm ligne-a-laquelle-aller-si-egal)
	     (execute-vm vm ligne-a-laquelle-aller-si-different))))
	
	
	;; le premier parametre de cmp est un registre (ici le seul est RO)
	;; le deuxieme parametre est une valeur
	;; l instruction compare la valeur du registre et la valeur du 2eme parametre .
	;; si les deux sont egaux,
	;; affecte au flag de comparaison de la machine virtuelle
	;; la valeur '1 sinon la valeur '0
	((eql (car (get-tableau vm ligne-a-executer)) 'CMP)
	 (let ((resultat
		(eql (caddr (get-tableau vm ligne-a-executer))
		     (get vm ':R0))))
	   (if (eql (cadr (get-tableau vm ligne-a-executer)) 'R0)
	       (progn
		 (if resultat (set-reg-equal vm '1) (set-reg-equal vm '0))
		 (execute-vm vm (1+ ligne-a-executer)))
	     (error "le premier parametre de CMP doit etre RO (ligne ~S) instruction ~S" ligne-a-executer (get-tableau vm ligne-a-executer)))))
	
	
	;; charge la valeur du 'caddr' (third) dans le registre indique
	;; ici seul R0 est definit,
	;; mais on peut en rajouter (des registres)
        ((eql (car (get-tableau vm ligne-a-executer)) 'LOAD)
	 (if (eql (cadr (get-tableau vm ligne-a-executer)) 'R0)
	     ;; on emmene la 3eme valeur dans R0
	     (progn (setf (get vm ':R0) (caddr (get-tableau vm ligne-a-executer)))
		    (execute-vm vm (1+ ligne-a-executer)))
	   (error "on ne peut affecter avec LOAD que dans R0 a la ligne ~S dans ~S" ligne-a-executer (get-tableau vm ligne-a-executer))))
	
	;; l instruction speciale "ne rien faire"
	;; on passe donc a l execution de la ligne suivante
        ((eql (car (get-tableau vm ligne-a-executer)) 'NOP) 
	 (execute-vm vm (1+ ligne-a-executer)))
	
	
	;; fonction non implementee
	;; ou mala formulee
        (T (write-line "execution inposible de ~S a la ligne ~S : NYI") (get-tableau vm ligne-a-executer) ligne-a-executer))
    )
  )
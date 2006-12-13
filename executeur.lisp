
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
	  (princ " => R0 ")
	  (princ (get vm ':R0))
	  (write-line "")))
    (cond 
     ;; mettre une valeur dans R
        ((and (eql (car (get-tableau vm ligne-a-executer)) ':MOVE) (eql (cadr (get-tableau vm ligne-a-executer)) 'R))
	 (progn (setf (get vm ':R) (caddr (get-tableau vm ligne-a-executer)))
		    (execute-vm vm (1+ ligne-a-executer))))
	
     ;; incrementer le PTR
        ((and (eql (car (get-tableau vm ligne-a-executer)) ':INCR) (eql (cadr (get-tableau vm ligne-a-executer)) 'PTR))
	 (progn (setf (get vm ':PTR) (- (get vm ':PTR) 1))
		    (execute-vm vm (1+ ligne-a-executer))))
	
     ;; loader la valeur du ptr dans R0
        ((equal (get-tableau vm ligne-a-executer) '(:LOAD R0 *PTR))
	 (progn (setf (get vm ':R0) (get-tableau vm (get vm ':PTR)))
		    (execute-vm vm (1+ ligne-a-executer))))
	
	
	
     ;; saut simple,
     ;; saute a l instruction definie dans le 'cdr' et execute cette instruction
        ((eql (car (get-tableau vm ligne-a-executer)) ':JMP) 
	 (execute-vm vm (cadr (get-tableau vm ligne-a-executer))))
	
	;; l instruction STOP arrete la vm
	;; en renvoyant le code de succes T
	;; le T devrai s afficher a l'ecran
        ((eql (car (get-tableau vm ligne-a-executer)) ':STOP)
	 (progn (if execute-debug (write-line "execution terminee avec succes")) (setf (get vm ':PTR) (get vm ':ram)) (eq (get vm ':R) '1)) )
	
	;; saut conditionnel
	;; si le flag d'egalite de la machine virtuelle, saute au 'cdr'
	;; sinon execute l instruction suivante .
        ((eql (car (get-tableau vm ligne-a-executer)) ':JEQ)
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
	((and (eql (car (get-tableau vm ligne-a-executer)) ':CMP)
		  (eql (cadr (get-tableau vm ligne-a-executer)) 'R0))
	 (let ((resultat
		            (eql (caddr (get-tableau vm ligne-a-executer))
		                 (get vm ':R0))))
		(progn
		 (if resultat (set-reg-equal vm '1) (set-reg-equal vm '0))
		 (execute-vm vm (1+ ligne-a-executer)))))
	
	
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
	;; ou mal formulee
        (T (error "execution inposible de ~S a la ligne ~S : NYI"  (get-tableau vm ligne-a-executer) ligne-a-executer))
    )
  ))

;; make.vm.lsp

;; fonctions de creation et interrogation d une V.M.


;; version 0.1

;; auteur cyril
;; date 02-12-2006
;; release notes :--
;; registre de comparaison a '0 ou '1
;; affiche tableau avec retour a la ligne



(defun make-vm (nom-vm taille)
  (let ((resultat (if (= 0 taille)
		      (make-vm nom-vm 100)
		    (progn 
		      (setf (get nom-vm ':regeq) '0)
		      (setf (get nom-vm ':ram) taille)
		      (setf (get nom-vm ':R0) '0)
		      (make-array (list taille):initial-element (list 'NOP))))))
    (setf (get nom-vm ':mem) resultat)))

(defun write-tableau (nom-vm indice valeur)
  (setf (aref (get nom-vm ':mem) indice ) valeur))

(defun get-tableau (nom-vm numligne)
  (aref (get nom-vm ':mem) numligne))

(defun affiche-tableau (nom-vm)
  (labels 
   ((aide (nom-vm ligne)
	  (if (< ligne (get nom-vm ':ram))
	      (progn
		(princ (get-tableau nom-vm ligne))
		(write-line "")
		(aide nom-vm (1+ ligne)))
	    (progn
	      (princ "R0 : ")
	      (princ (get nom-vm ':R0))
	      (write-line "")
	      (princ "TAILLE : ")
	      (princ (get nom-vm ':ram))
	      (write-line "")
	      (princ "REGEQ : ")
	      (princ (get nom-vm ':regeq))
	      (write-line "fin")
	      T))))
   (aide nom-vm 0)))

(defun get-reg-equal (nom-vm)
  (eql (get nom-vm ':regeq) '1))

(defun set-reg-equal (nom-vm valeur)
  (if (eql valeur '1)
      (setf (get nom-vm ':regeq) '1)
    (if (eql valeur '0)
	(setf (get nom-vm ':regeq) '0)
      (error "On ne peut mettre que '0 ou '1 dans le registre d egalite"))))

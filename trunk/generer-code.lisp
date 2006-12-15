;; 15-12-2006
;; Generation de code

;; jean-marie codol
;; guillaume sauvajol


;; generer le code assembleur

(defun generer-code (fichier &optional (fichier-sortie "code.asm"))
  (let ((flux (open fichier :direction :input)) (EOf (gensym))
		(flux-sortie (open fichier-sortie :direction :output)))
    (let ((r (read flux () EOF)))
      (progn (loop until (eql r EOF) do (progn (generer-code-a-partir-de-l-automate r flux-sortie)
                                              (setf r (read flux () EOF))))
			 (close flux)))))


(defun generer-code-a-partir-de-l-automate (automate flux-sortie)
  ;; variables locales a la fonction
  (let ((vocabulaire (get_vocabulaire automate))
	(liste_des_etats (get_liste_des_etats automate))
	(etat_initial (get_etat_initial automate))
	(etats_finaux (get_etat_final automate))
	(liste_transitions (get_liste_transitions automate)))
    ;; pour chaque etat, on genere le code
    (progn
      (write-line "((:LABEL @INIT)" flux-sortie)
      (write-line "(:MOVE R 0)" flux-sortie)
      (loop until (null liste_des_etats) do
	(progn 
	  (princ "(:LABEL @etat" flux-sortie)(princ (car liste_des_etats) flux-sortie)(write-line ")" flux-sortie)
	  (write-line "(:INCR PTR)" flux-sortie)
	  (write-line "(:LOAD R0 *PTR)" flux-sortie)
	  ;; pour chaque lettre du vocabulaire,
	  ;;  on genere le code
	  (let ((vocabulaire_temporaire (copy-list vocabulaire)))
	    (loop until (null vocabulaire_temporaire) do
	      (progn (princ "(:CMP R0 " flux-sortie)(princ (car vocabulaire_temporaire) flux-sortie)(write-line ")" flux-sortie)
		     ;; sauter au bon endroit (bon label) @echec sinon
		     (let ((suivant (get_suivant_par (car liste_des_etats) (car vocabulaire_temporaire) liste_transitions)))
		       (if (null suivant)
			   (write-line "(:JEQ @echec)" flux-sortie)
			 (progn (princ "(:JEQ @etat" flux-sortie)(princ suivant flux-sortie)(write-line ")" flux-sortie))))
		     (setf vocabulaire_temporaire (cdr vocabulaire_temporaire)))) ;; fi de la 2eme boucle
	    ;; si on croise le $ (fin de mot)
	    ;; si c'est un final ===> label "@fin"
	    ;; si non ==============> label "@echec"
	    (progn
	      (write-line "(:CMP R0 $)" flux-sortie)
	      (let ((final (member (car liste_des_etats) etats_finaux)))
		 
		(if final (write-line "(:JEQ @fin)" flux-sortie) (write-line "(:JEQ @echec)" flux-sortie)))
		(write-line "(:JMP @echec)" flux-sortie)))
	  (setf liste_des_etats (cdr liste_des_etats))));; fin du 1er loop
      (write-line "(:LABEL @fin)" flux-sortie)
      (write-line "(:MOVE R 1)" flux-sortie)
      (write-line "(:STOP)" flux-sortie)
      (write-line "(:LABEL @echec)" flux-sortie)
      (write-line "(:MOVE R 0)" flux-sortie)
      (write-line "(:STOP))" flux-sortie)
	  ;; il ne faut pas oublier de fermer le flux,
	  ;; sinon il manquera la fin du fichier
	  (finish-output flux-sortie)
	  (close flux-sortie)
	  T)))



(defun get_vocabulaire (automate)
  (if (null automate)
      ()
    (if (eql ':voc (caar automate))
	(cdar automate)
      (get_vocabulaire (cdr automate))))
  )


(defun get_liste_des_etats (automate)
   (if (null automate)
      ()
     (if (eql ':etat (caar automate))
	 (cdar automate)
       (get_liste_des_etats (cdr automate)))))
 



(defun get_etat_initial (automate)
  (if (null automate)
      ()
    (if (eql ':etat-init (caar automate))
	(cdar automate)
      (get_etat_initial (cdr automate)))))


(defun get_etat_final (automate)
  (if (null automate)
      ()
    (if (or (eql ':etat-final (caar automate)) (eql ':etat-fin (caar automate)))
	(cdar automate)
      (get_etat_final (cdr automate)))))


(defun get_liste_transitions (automate)
  (if (null automate)
      ()
    (if (eql ':transition (caar automate))
	(cdar automate)
      (get_liste_transitions (cdr automate)))))


(defun get_suivant_par (etat_depart caractere_a_suivre liste_des_transitions)
  (if (atom liste_des_transitions)
      ()
    (if (and (eql etat_depart (caar liste_des_transitions)) (eql caractere_a_suivre (cadar  liste_des_transitions)))
	(caddar liste_des_transitions)
      (get_suivant_par etat_depart caractere_a_suivre (cdr liste_des_transitions)))))

	  
	  

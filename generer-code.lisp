
;; generer le code assembleur


(defun generercode (automate)
  ;; variables locales a la fonction
  (let ((vocabulaire (get_vocabulaire automate))
	(liste_des_etats (get_liste_des_etats automate))
	(etat_initial (get_etat_initial automate))
	(etats_finaux (get_etat_final automate))
	(liste_transitions (get_liste_transitions automate)))
    ;; pour chaque etat, on genere le code
    (progn
      (write-line "LABEL @INIT")
      (write-line "MOVE R #0")
      (loop until (null liste_des_etats) do
	(progn 
	  (princ "LABEL @etat")(princ (car liste_des_etats))(write-line "")
	  (write-line "INCR PTR")
	  (write-line "LOAD R0 *PTR")
	  ;; pour chaque lettre du vocabulaire,
	  ;;  on genere le code
	  (let ((vocabulaire_temporaire (copy-list vocabulaire)))
	    (loop until (null vocabulaire_temporaire) do
	      (progn (princ "CMP RO ")(princ (car vocabulaire_temporaire))(write-line "")
		     ;; sauter au bon endroit (bon label) @echec sinon
		     (let ((suivant (get_suivant_par (car liste_des_etats) (car vocabulaire_temporaire) liste_transitions)))
		       (if (null suivant)
			   (write-line "JEQ @echec")
			 (progn (princ "JEQ @etat")(princ suivant)(write-line ""))))
		     (setf vocabulaire_temporaire (cdr vocabulaire_temporaire)))) ;; fi de la 2eme boucle
	    ;; si on croise le $ (fin de mot)
	    ;; si c'est un final ===> label "@fin"
	    ;; si non ==============> label "@echec"
	    (progn
	      (write-line "CMP R0 $")
	      (let ((final (member (car liste_des_etats) etats_finaux)))
		(if final (write-line "JMP @fin") (write-line "JMP @echec")))))
	  (setf liste_des_etats (cdr liste_des_etats))));; fin du 1er loop
      (write-line "LABEL @fin")
      (write-line "MOVE R #1")
      (write-line "STOP")
      (write-line "LABEL @echec")
      (write-line "MOVE R #0")
      (write-line "STOP"))))



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
    (if (eql ':etat-fin (caar automate))
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

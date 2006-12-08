;; generation de code


;; charge les instructions assembleur dans une V.M.


;; version 0.2

;; auteur jean-marie
;; date 03-12-2006
;; release notes :--
;; test sur la taille de la vm
;; ==> recuperer l exception "machine virtuelle trop petite"





;; version 0.1

;; auteur jean-marie
;; date 02-12-2006
;; release notes :--
;; add asm library instruction in function "charger-code" => we can add instructions to parse in the main function
;; make link with the structure vm :mem to load instructions in this structure
;; modify error message :  "erreur : impossible de resoudre l etiquette '~S' demandee a la (aux) lignes de code ~S" (the first ~S was't)





;; il y a beaucoup de progn,
;; mais c est pour afficher des informations pour comprendre le deroulement
;; si la variable de debug est a T

;; le cahrgeur doit resoudre les etiquettes
;; par exemple
;; 349 : (:label @titi)
;; ...
;; 10034 : (JMP @titi)
;; ...

;; avec le chargement donnera
;; 349 : #instruction suivante#
;; ...
;; 10033 : (JMP 349)
;;  ...


;; donc on maintient 2 listes :

;; la liste "liste-adresse" contient les etiquettes connues
;;  de maniere a pouvoir avoir un dictionnaire de recherche au premier passage

;; la liste "liste-adresse-liste-lignes" contient les
;;  etiquettes demandees mais non resolues
;; a la fin du premier parcours,
;; on fait un parcours de cette 2eme liste,
;; et on modifie juste les parties de la memoire qu'il faut .



;_________________________________
;_________________________________
;; quelques variables globales importantes
;; si on est en mode debug :
(setf loader-debug ())

;; fonction principale:
;; les parametres sont
;; -> machine-virtuelle ===== nom de la machine virtuelle
;; -> code =========== code a charger
;; -> ligne-debut ======== la ou on commence a charger le code (par defaut '0)
(defun charger-code (machine-virtuelle code &optional (ligne-debut '0))
  ;; on fera une recursion interne sur
  ;; "code", pour cela on definit une sous fonction
  (labels(
	  ;; d abord une fonction qui ajoute une etiquette dans le tableau des
	  ;; etiquette, renvoie le tableau des etiquettes modifie
	  (ajouter-reference-dans-liste-adresse-et-retourner-la-liste (etiquette ligne tableau-de-correspondance)
								      ;; simplement ajouter au debut la reference
								      ;; (le couple "adresse |-> ligne")
								      (cons `(,etiquette ,ligne) tableau-de-correspondance)
								      )
	  ;; le tableau 2 est le tableau contenant les etiquettes non resolues.
	  ;; ex:      '((etiq1 102 105 10034)  (etiq2 99 5040))
	  ;; on parcours recursivement , on compare le caar
	  (l-etiquette-est-elle-dans-le-tableau2 (etiq tab2)
						 (if (null tab2)
						     ;; condition d arret
						     ()
						   (if (eql (caar tab2) etiq)
						       T
						     (l-etiquette-est-elle-dans-le-tableau2 etiq (cdr tab2)))))
	  ;; ici l'etiquette est dejas dans le tableau,
	  ;; on veut juste l'ajouter dans tab2 et rendre tab2 :
	  (ajouter-etiquette-dans-tableau2-et-rendre-tab2 (etiq ligne tab2)
							  (cons (cons etiq ligne) tab2))
	  ;; on garde la structure de ajouter-etiquette-dans-tableau2-et-rendre-tab2
	  ;; au lieu de modifier un tuple,
	  ;; on en rajoute un
	  (ajouter-tuple-dans-tableau2-et-rendre-tab2 (etiq ligne tab2)
						      (cons (cons etiq ligne) tab2))
	  ;; au final,
	  ;; il faut utiliser le tableau2 pour resoudre les dernieres etiquettes
	  ;; on va parcourir tab2 et modifier la memoire la ou il faut
	  ;; rend T si tout se passe bien () sinon
	  (modifications-de-la-memoire-avec-le-tableau1-et-2 (vm tab1 tab2)
							     ;; les seules istructions que l on a sont JMP et JEQ
							     ;; donc on modifiera toujour le 2eme parametre en memoire
							     (if (null tab2)
								 T
							       (let ((ligne (cdr (assoc (caar tab2) tab1))) ;; ligne la ou on va => destination
								     (ligne-a-modifier (cdar tab2))) ;; ligne de la memoire a modifier
								 (if ligne
								     (progn 	(if loader-debug
										    (progn
										      (PRINC "il y a un truc a modifier :")
										      (PRINC ligne-a-modifier)
										      (write-line "")
										      (PRINC "destination :")
										      (PRINC ligne)
										      (write-line "")))
										(write-tableau vm ligne-a-modifier (cons (car (get-tableau vm ligne-a-modifier)) ligne))
										(modifications-de-la-memoire-avec-le-tableau1-et-2 vm tab1 (cdr tab2)))
								   (error "erreur : impossible de resoudre l etiquette '~S' demandee a la (aux) lignes de code ~S" (caar tab2) ligne-a-modifier)))))
	  
	  ;;________________________________________________
	  ;; cette fonction remplis la memoire
	  ;; avec ce qu elle peut :
	  ;; ici les etiquettes connues
	  ;; sont resolues
	  ;; les non connues
	  ;; sont stoquees dans tab2
	  ;; et renvoyees avec tab1( la fonction renvoie le (cons tab1 tab2) rempli)
	  ;; elle pourront etre traitees plus tard
	  (charger-code2 (vm code num-ligne-insertion tab1 tab2)
			 ;; le test de fin de recursion
			 ;; il n y a plus de code a charger
			 (if (null code)
			     ;; on rend tab2
			     ;; la vm a ete modifie
			     ;; pour les traitements suivants,
			     ;; il faut utiliser tab2
			     ;; et raccorder a tab1
			     ;; pour acceder a tab1 => car (x)
			     ;; pour acceder a tab2 => cdr(x)
			     (cons tab1 tab2)
			   ;; si il reste encore du code a charger
			   ;; (principal)
			   ;; on definit ligne comme la ligne a enregistrer
			   (let ((ligne (car code))
				 (liste-des-etiquettes-autorisees '(label :label etiq :etiq etiquette :etiquette))
				 (liste-des-sauts-etiquetes-autorisees '(jmp :jmp jeq :jeq))
				 (liste-des-instructions-autorisees '(STOP CMP LOAD NOP)))
			     (progn 
			       (if (>= num-ligne-insertion (get vm ':ram)) (error "impossible d'ecrire la ligne ~S : depassement de la taille de la ram disponible ([0 .. ~S])" num-ligne-insertion (- (get vm ':ram) 1)))
			       (cond 
				((member (car ligne) liste-des-etiquettes-autorisees)
				 ;; on a trouve une etiquette
				 ;; on maintient a jour la liste des 
				 ;; etiquettes connues (tab1)
				 ;; le nom de l etiquette est dans le 2eme parametre
				 ;; (cadr ligne)
				 (if (assoc (cadr ligne) tab1)
				     (error "erreur a la ligne de code a charger ~S : l etiquette ~S est dejas utilise a la ligne ~S" num-ligne-insertion (cadr ligne) (cdr (assoc (cadr ligne) tab1)))
				   ;; sinon c est bon
				   ;; on peut ajouter la corresppondance
				   ;; dans la liste tab1
				   ;; et appeller charger-code2 avec la nouvelle valeur
				   ;; on ne modifie pas la ligne d insertion
				   ;; ( ca evite de mettre un NOP dans la memoire)
				   ;; =>on ne touche pas a la memoire
				   ;;   pour le traitement des labels
				   ;; appelle donc
				   ;; charger(code2 tab1mod 
				   (charger-code2 vm (cdr code) num-ligne-insertion (ajouter-reference-dans-liste-adresse-et-retourner-la-liste (cadr ligne)  num-ligne-insertion tab1) tab2)))
				
					; le cas de jmp jeq 
				((member (car ligne) liste-des-sauts-etiquetes-autorisees)
				 ;; si l etiquette passee est connue :
				 (if (assoc (cadr ligne) tab1)
				     ;; cettte apartie la il faut l adapter a la machine virtuelle
				     (progn 
				       (if loader-debug
					   (progn
					     (PRINC num-ligne-insertion)
					     (PRINC " : ")
					     (PRINC  (cons (car ligne) (cdr (assoc (cadr ligne) tab1))))
					     (write-line "")))
				       (write-tableau vm num-ligne-insertion (cons (car ligne) (cdr (assoc (cadr ligne) tab1))))
				       (charger-code2 vm (cdr code) (1+ num-ligne-insertion) tab1 tab2))
				   ;;(error " etiquette inconnue ~S" (cadr ligne)))
				   ;; si l etiquette est inconnue,
				   ;; on va sauver dans tab2 le nom et la ligne correspondante
				   (if (l-etiquette-est-elle-dans-le-tableau2 (cadr ligne) tab2)
				       (progn  
					 (if loader-debug
					     (progn
					       (PRINC num-ligne-insertion)
					       (PRINC " : ")
					       (PRINC ligne)
					       (write-line "")))
					 (write-tableau vm num-ligne-insertion ligne)
					 (charger-code2 vm (cdr code) (1+ num-ligne-insertion) tab1 (ajouter-etiquette-dans-tableau2-et-rendre-tab2 (cadr ligne) num-ligne-insertion tab2)))
				     ;; l etiquette n est pas dans le tableau 2
				     ;; donc il faut creer le tuple dans tab2 en appellant charger-code2
				     (progn 	
				       (if loader-debug
					   (progn
					     (PRINC num-ligne-insertion)
					     (PRINC " : ")
					     (PRINC ligne)
					     (write-line "")))
				       (write-tableau vm num-ligne-insertion ligne)
				       (charger-code2 vm (cdr code) (1+ num-ligne-insertion) tab1 (ajouter-tuple-dans-tableau2-et-rendre-tab2 (cadr ligne) num-ligne-insertion tab2))))))
				;; les instructions a charger mais pas a traiter,
				;; simplement ecrire dans le machine virtuelle
				;; on ne resoud pas d etiquettes
				;; le tableau liste-des-instructions-autorisees est definie par un let au debut de la fonction (labels)
				((member (car ligne) liste-des-instructions-autorisees)
				 (progn 	
				   (if loader-debug
				       (progn
					 (PRINC num-ligne-insertion)
					 (PRINC " : ")
					 (PRINC ligne)
					 (write-line "")))
				   (write-tableau vm num-ligne-insertion ligne)
				   (charger-code2 vm (cdr code) (1+ num-ligne-insertion) tab1 tab2)))
				
				;; si on arrive la ,c'est que la fonction (car ligne) n'est pas valide
				;; i.e. elle ne fait pasa partie du langage assembleur definit dans la machine virtuelle .
				;; l 'ensemble des valeurs possibes est definie au debut de la fonction
				(T
					(progn
					  (if loader-debug
					      (progn
						(PRINC num-ligne-insertion)
						(PRINC " : ")
						(PRINC ligne)
						(write-line "")))
					  (error "erreur a la ligne de code a charger ~S : l instruction ~S n est pas encore taitee ou le code assembleur n est pas bon" num-ligne-insertion (car ligne))))
				))))))
				
	 
	 ;;______________________________________________________________
	 ;;______________________________________________________________
	 ;; fonction principale
	 ;;______________________________________________________________
	 
	 ;; on appelle avec les bons parametres de depart
	 (let ((resultat (charger-code2 machine-virtuelle code ligne-debut '() '())))
	   ;; tab1 est dans car resultat,
	   ;; tab2 est dans cdr resultat
	   (if (cdr resultat)
	       (let ((tab1 (car resultat))
		     (tab2 (cdr resultat)))
		 ;; on obtient dans resultat les tuples des instructions
		 ;; qui n'ont pas pu etre resolues
		 (progn 	(if loader-debug
				    (progn
				      (write-line "")	
				      (PRINC "tab1:")
				      (PRINC tab1)
				      (write-line "")	
				      (PRINC "tab2:")
				      (PRINC tab2)
				      (write-line "")))
				(if (modifications-de-la-memoire-avec-le-tableau1-et-2 machine-virtuelle tab1 tab2)
				    (if loader-debug
					"FIN CORRECTE : chargement reussi"
				      T)
				  (error "impossible de finir de charger les "))))
	     ;; termine, tout c est bien passe et il n'y a pas d etiquettes irresolues
	     (progn 	(if loader-debug
			    (progn
			      (write-line "")	
			      "FIN CORRECTE 2 : chargement reussi")T)T)))))











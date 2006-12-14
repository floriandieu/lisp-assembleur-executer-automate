
import java.util.Vector;

/**
 * 
 * La Classe Automate contient
 * les informatons sur un automate
 * 
 * ainsi que quelques methodes utiles
 * pour la generation .
 * 
 * Ce modele d'automate ne gere pas les
 * epsilon transitions,
 * ni les boucles
 * (les 2 sont inutiles pour generer
 * un automate a partir d un dictionnaire)
 * 
 * il n'y a qu 1 etat initial
 * une seule operation :
 * ajouterMot  =====> UNION
 * 
 * pendant l union,
 * on determinise !
 * 
 * on arrive avec un bon automate directement
 * 
 * utiliser avec 
 * 
 * pour utiliser avec la generation de code
 * 
 * utiliser avec Main pour la generation de code
 * 
 * @author jean-marie codol
 *
 */
public class Automate {

	/*
	 * la liste des etats
	 */
	private Vector<Etat> Etats = null ;
	/*
	 * le vocabulaire
	 */
	private Vector<Character> Voc = null ;
	/*
	 * l etat initial
	 */
	private Etat etatInitial = null ;
	/*
	 * les etats finaux
	 */
	private Vector<Integer> etatsFinaux = null ;
	/*
	 * le nombre d etats crees :
	 *
	 */
	int numEtat = 0 ;

	
	/*
	 * constructeur qui initialise les attributs
	 */
	public Automate() {
		Etats = null ;
		Voc = null ;
		etatInitial = null ;
	}


	

	/*
	 * cree un Etat unique ( numero non attribue )
	 * et le retourne
	 */
	private Etat ajouterEtat() {
		int num = this.genererNumeroEtatOriginal() ;
		Etat e ;
		if (Etats==null)
		{
			etatInitial = new Etat(num) ;
			e= etatInitial ;
		}else {
			e = new Etat(num) ;
		}
		Etats.addElement(e);
		return e ;
	}

	
	/*
	 * ajoute une liaison entre 2 Etats
	 */
	private boolean relierEtatsAvec (Etat a,Etat b,char car)
	{
		a.ajouterLiaison(car,b) ;
		return true ;
	}

	
	/*
	 * genere un numero d Etat non utilise
	 * comme on ne supprime jamais d etats,
	 * on propose le precedant + 1
	 */
	private int genererNumeroEtatOriginal()
	{
		// au depart, on propose 0
		if (Etats==null)
		{
			Etats = new Vector<Etat> () ;
			return 0;
		}
		numEtat ++ ;
		return numEtat ;
	}
	
	
	
	/**
	 * ajouter un mot dans l automate .
	 * a la fin,
	 * l'automate est un AFD
	 * @param mot
	 * @return
	 */
	public boolean ajouterUnMot (String mot)
	{

		// si le mot est vide, on ne fait rien
		if (mot.length()==0)
			return true ;

		
		if ((etatInitial == null))
		{
			this.etatInitial = this.ajouterEtat() ;
		}
		
		
		// on parcours tout le mot-1 lettre et on trace un nouveau chemin
		// si le chemin existe dejas,
		// on se contente de passer au suivant
		Etat etatDeTete = this.etatInitial ;
		for (int i=0;i<mot.length();i++)
		{
			this.ajouterAuVocalulaireSiBesoin (mot.charAt(i)) ;
			System.err.print(mot.charAt(i)) ;
			// est-ce que le chemin existe dejas ???
			boolean leCeminExisteDejas = false ;
			for (Liaison l : etatDeTete.getLiaisons())
			{
				// pour chaque liaison, on va voir si le chemin existe dejas
				if (l.getCaractereDeLiaison()==mot.charAt(i))
				{
					leCeminExisteDejas=true ;
					etatDeTete = l.getEtatSuivant() ;
				}
			}
			if (!leCeminExisteDejas)
			{
				// ici le chemin n'a pas ete trouve :
				Etat nouvelEtat = ajouterEtat() ;
				this.relierEtatsAvec(etatDeTete,nouvelEtat,mot.charAt(i)) ;
				etatDeTete = nouvelEtat ;
				if (etatsFinaux == null)
					etatsFinaux = new Vector<Integer> ();
			}
			if (i==mot.length()-1)
				if (!this.estUnEtatFinal(etatDeTete.getNumero()))
					etatsFinaux.add(etatDeTete.getNumero());				
		}
		return true ;
	}
	
	
	/**
	 * rend vrai si c'est un etat final
	 * @param numero
	 * @return
	 */
	private boolean estUnEtatFinal(int numero) {
		for (Integer i : this.etatsFinaux)
		{
			if (i.intValue()==numero)
				return true;
		}
		return false;
	}




	/**
	 * ajoute le caractere s'il n est pas dans le vocabulaire
	 * @param c
	 */
	private void ajouterAuVocalulaireSiBesoin(char c) {
		// on ajoute la lettre au voc si elle n'y est pas :
		if (Voc == null)
			Voc = new Vector<Character> ();
		boolean laLettreEstDansLeVoc = false ;
		for (Character ch : Voc)
		{
			if (ch.charValue()==c)
				laLettreEstDansLeVoc = true ;
		}
		if (!laLettreEstDansLeVoc)
			Voc.add(new Character(c)) ;
	}
	
	
	
	
	/**
	 * decrit l 'automate (non lisp)
	 *
	 */
	public void decrireAutomate ()
	{
		System.out.print  ("VOC     : ");
		for (Character ch : Voc)
		{
			System.out.print("<"+ch.charValue()+">") ;
		}
		System.out.println("") ;
		System.out.println("INITIAL : <"+etatInitial.getNumero()+">");
		System.out.print  ("FINAL   : ");
		for (Integer i : etatsFinaux)
		{
			System.out.print("<"+i+">") ;
		}
		System.out.println("");
		System.out.print  ("ETATS   : ");
		for (Etat e : Etats)
		{
			System.out.print("<"+e.getNumero()+">") ;
		}
		System.out.println("");
		this.afficheInfosRec() ;
	}
	


	/**
	 * decrit l'automate (lisp)
	 *
	 */
	public void decrireAutomateLisp() {
		System.out.print  ("(:VOC ");
		for (Character ch : Voc)
		{
			System.out.print(ch.charValue()+" ") ;
		}
		System.out.print  (")\n");
		System.out.println(" (:ETAT-INIT  "+etatInitial.getNumero()+")");
		System.out.print  (" (:ETAT-FINAL ");
		for (Integer i : etatsFinaux)
		{
			System.out.print(i+" ") ;
		}
		System.out.println(")");
		System.out.print  (" (:ETAT ");
		for (Etat e : Etats)
		{
			System.out.print(e.getNumero()+" ") ;
		}
		System.out.println(")");
		//System.out.print(" (:TRANSITION");
		//this.afficheInfosRecLisp() ;
		System.out.print(")");
	}
	


	/**
	 * affiche les infos (non lisp)
	 *
	 */
	private void afficheInfosRec()
	{
		this.etatInitial.afficheInfosRec() ;
		this.etatInitial.reinitialiseAffichageInfoRec() ;		
	}





	
}

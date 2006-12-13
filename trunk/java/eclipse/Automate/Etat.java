import java.util.Vector;

public class Etat {

	private int numero ;
	private Vector<Liaison> Liaisons ;
	// pour l affichage des infos,
	// a cause des boucles
	boolean aAfficheLesInfos = false ;
	
	
	
	
	
	public Etat(int num) {
		numero = num ;
		Liaisons = null ;
		Liaisons = new Vector<Liaison> ();
	}

	
	
	
	public void ajouterLiaison (char car,Etat suiv)
	{
		if (suiv == null)
		{
			System.err.println("on ne peut pas ajouter une liaison vers null (Etat::ajouterLiaison(c,e))");
			System.exit(5) ;
		}
		if (Liaisons == null)
			Liaisons = new Vector<Liaison> ();
		System.out.print ("\n( " + this.numero + " " + car + " " + suiv.getNumero() + " )") ;
		System.err.print ("+") ;
		Liaisons.add(new Liaison(car,suiv)) ;
	}
	


	
	

	
	// affiche recursivement les infos
	public void afficheInfosRec ()
	{
		if (!aAfficheLesInfos)
		{
			aAfficheLesInfos = true ;
			// on ecrit ses propres liaisons
			for ( Liaison l : Liaisons)
			{
				System.out.print("<"+getNumero()+">") ;
				System.out.println("  <"+l.getCaractereDeLiaison()+"> -> <"+l.getEtatSuivant().getNumero()+">") ;
			}			
			for ( Liaison l : Liaisons)
			{
				l.getEtatSuivant().afficheInfosRec() ;
			}			
		}
	}
	
	// affiche recursivement les infos
	public void afficheInfosRecLisp ()
	{
		if (!aAfficheLesInfos)
		{
			aAfficheLesInfos = true ;
			String resultat ;
			// on ecrit ses propres liaisons
			for ( Liaison l : Liaisons)
			{
				resultat = "\n  (" ;
				resultat+=getNumero()+" " ;
				resultat+=l.getCaractereDeLiaison()+" "+l.getEtatSuivant().getNumero()+" )" ;
				System.out.print(resultat);
			}
			for ( Liaison l : Liaisons)
			{
				l.getEtatSuivant().afficheInfosRecLisp() ;
			}			
		}
	}
	

	
	
	
	public void reinitialiseAffichageInfoRec ()
	{
		if (aAfficheLesInfos)
		{
			this.aAfficheLesInfos = false ;
			for ( Liaison l : Liaisons)
			{
			l.getEtatSuivant().reinitialiseAffichageInfoRec() ;
			}
		}
	}
	
	public void infos ()
	{

		System.out.print("description etat <"+this.getNumero()+"> ");
			for ( Liaison l : Liaisons)
			{
				System.out.print("<"+getNumero()+">") ;
				System.out.println("  <"+l.getCaractereDeLiaison()+"> -> <"+l.getEtatSuivant().getNumero()+">") ;

			}		
	}
	
	
	
	public int getNumero() {
		return numero;
	}


	public Vector<Liaison> getLiaisons()
	{
		return Liaisons ;
	}


	
	
}

/**
 * La liaison appartient a un Etat,
 * c'est la liaison qui possede le caractere
 * @author jean-marie codol
 *
 */

public class Liaison {

	private char caractereDeLiaison ;
	private Etat etatSuivantParLeCaractere ;
	
	public Liaison(char carac,Etat suiv) {
		if (suiv == null)
		{
			System.err.println("on ne peut pas creer une liaison vers null (constructeur de Liaison(c,e))");
			System.exit(3) ;
		}
		caractereDeLiaison = carac ;
		etatSuivantParLeCaractere = suiv ;
	}
	
	// construit une Epsilone-transition
	public Liaison(Etat suiv) {
		if (suiv == null)
		{
			System.err.println("on ne peut pas creer une liaison vers null (constructeur de Liaison(e))");
			System.exit(4) ;
		}
		caractereDeLiaison = '\n' ;
		etatSuivantParLeCaractere = suiv ;
	}

	public char getCaractereDeLiaison() {
		return caractereDeLiaison;
	}


	public Etat getEtatSuivant() {
		return etatSuivantParLeCaractere;
	}
	
	
}

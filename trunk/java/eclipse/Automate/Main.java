import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;


/**
 * Main permet de lancer le programme
 * de generation d'automate
 * a partir d'un ficher de mots .
 * 
 * 
 * utiliser -> "java Main" pour afficher l usage
 * 
 * 
 * 
 * @author jean-marie codol
 *
 */
public class Main {

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		if (args.length==0)
		{
			usage() ;
			System.exit(0) ;
		}
		Automate a = new Automate() ;
		
		
		try {
			BufferedReader reader = new BufferedReader(new FileReader(args[0])) ;
		    // une ligne
			String ligne = null ; 
		    try {
		    	int numligne = 0 ;
		    	System.out.println("((:transition") ;
				while ((ligne = reader.readLine()) != null) 
				{
					System.err.println(numligne) ;
					numligne++ ;
					// pas de ""
					// et pas de "#xxxxxxxxxx"
					if (ligne.length()>=1)
						if (ligne.charAt(0)!='#')
							a.ajouterUnMot(ligne) ;
				}
				System.out.println(")") ;
				// il y a 2 arguments
				
				//if (args.length>=2)
				//{
				//	if (args[1].equalsIgnoreCase("-lisp"))
				//		{
						a.decrireAutomateLisp() ;
				//		}else
				//		{
				//			a.decrireAutomate() ;
				//		}
				//}else
				//{
					//a.decrireAutomate() ;					
				//}
			} catch (IOException e) 
			{
				System.err.println("impossible de lire le fichier "+args[0] +
				"verifier les droits du fichier en lecture");
				System.exit(2) ;
			}
		} catch (FileNotFoundException e) 
		{
			System.err.println("impossible d ouvrir le fichier "+args[0] +
				"verifier l existance du fichier");
			System.exit(1) ;
		}
	}

	private static void usage() {
		System.out.println("\nusage :" +
				"\njava Main fichier_dictionnaire [-lisp]" +
				"\n--------------------------------------" +
				"\n" +
				"\nCe programme permet de construire un automate finit deterministe" +
				"\nA partir d un dictionnaire de mots fournis dans un fichier" +
				"\n1 mot par ligne " +
				"\n(les espaces sont consideres comme des lettres)" +
				"\n(les lignes vides ne seront pas prises en compte)" +
				"\n(les lignes commencant pas # ne seront pas prises en compte)" +
				"\n l'option -lisp permet d'afficher l automate en lisp !!!" +
				"\n         -------" +
				"\n" +
				"\nAuteur : Jean-Marie CODOL" +
				"\n2006-2007 Master Informatique a Montpellier 2" +
				"\n" +
				"\nLe format de sortie lisp a ete cree pour correspondre" +
				"\nAu format du module de generation de code" +
				"\n" +
				"\njmecodol@gmail.com" +
				"\n------------------" +
				"\n");
	}

}

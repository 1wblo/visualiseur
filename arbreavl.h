/* ArbreAVL<T> : arbre binaire de recherche auto-equilibre (AVL) generique.
 * Chaque insertion/suppression maintient l'equilibre (facteur -1, 0 ou 1
 * sur chaque noeud) via des rotations simples ou doubles, garantissant une
 * hauteur en O(log n) et donc des operations de recherche/insertion/retrait
 * en temps logarithmique.
 */
#if !defined(_ARBREAVL___H_)
#define _ARBREAVL___H_

#include <algorithm>
#include <cassert>
#include <iostream>

#include "dessinarbre.h"

template <class T>
class ArbreAVL {
  public:
// Constructeurs
    ArbreAVL();
    ArbreAVL(const ArbreAVL&);

// Destructeurs 
    ~ArbreAVL();

// Fonctions de modifications
    bool vide() const;
    /// Vrai si un element equivalent (operator<) est present et actif.
    bool contient(const T&) const;
    /// Insere l'element; si un element equivalent existe deja, remplace
    /// son contenu (et le "reactive" s'il avait ete marque enleve).
    void inserer(const T&);
    /// Marque l'element comme enleve (suppression paresseuse : le noeud
    /// reste dans l'arbre mais n'est plus compte ni renvoye).
    void enlever(const T&);
    void vider();
    /// Retourne l'element equivalent (doit exister et etre actif).
    const T& rechercher(const T&) const;
    T& rechercher(const T&);

    /// Plus petit element actif >= `element`; false si aucun n'existe.
    bool rechercherEgalOuApres(const T&, T&) const;
    /// Plus grand element actif <= `element`; false si aucun n'existe.
    bool rechercherEgalOuAvant(const T&, T&) const;

    /// Copie dans `elements` tous les elements actifs de [min, max],
    /// en ordre croissant, et met `nombre` a jour avec le compte copie.
    void remplirIntervalle(const T& min, const T& max, T* elements, int& nombre) const;

    int taille() const; // retourne le nombre d'éléments

    /// Nombre total de rotations (simples ou doubles, comptees comme 2)
    /// effectuees depuis la creation de l'arbre. Utilise pour comparer
    /// le "cout" de l'equilibrage face aux autres structures.
    int nombreRotations() const;
    /// Exporte la forme de l'arbre pour l'affichage : profondeur et rang
    /// in-ordre de chaque noeud, avec les indices de ses enfants.
    DessinArbre<T> exporterPourDessin() const;

// Fonctionnalites de "requete d'intervalle" (recherche par plage de valeurs):
    ArbreAVL<T>&    operator = (const ArbreAVL<T>& autre);
    bool            operator == (const ArbreAVL<T>& autre) const;

    /// Nombre d'elements actifs dans l'intervalle [min, max].
    int compter(const T& min, const T& max) const;
    /// Nouvel arbre AVL contenant une copie des elements actifs de [min, max].
    ArbreAVL<T> sous_ensemble(const T& min, const T& max) const;

// Fonctions pour certains tests ou diagnostique
    int compter() const;
    int hauteur() const;
    void afficher() const;
    void afficher(std::ostream& out) const;

private:
    struct Noeud {
        Noeud(const T&);
        T contenu;
        bool enleve;
        int equilibre;
        int tailleSousArbre;
        Noeud *gauche,
              *droite;
    };
    Noeud* racine;
    int rotations = 0;

    int construireDessin(const Noeud*, int, int&, DessinArbre<T>&) const;

// Fonctions internes
    bool inserer(Noeud*&, const T&);
    bool enlever(Noeud*, const T&);
    void rotationGaucheDroite(Noeud*&);
    void rotationDroiteGauche(Noeud*&);
    void mettreAJourTaille(Noeud*);
    void vider(Noeud*&);
    
    void remplirTableau(const Noeud*, const T**, int&) const;

    void remplirIntervalle(const Noeud*, const T&, const T&, T*, int&) const;

    int nombreElements(const Noeud*) const;
    int compterInferieurs(const Noeud*, const T&) const;
    int compterInferieursOuEgaux(const Noeud*, const T&) const;
    
    void ajouterSousEnsemble(const Noeud*, const T&, const T&, ArbreAVL<T>&) const;

// Lab6 / Tâche 4
    void copier(const Noeud*, Noeud*&) const;

// Fonctions internes pour certains tests
    int hauteur(const Noeud*) const;
    int compter(const Noeud*) const;
    void preparer_afficher(const Noeud* n, int profondeur, int& rang, T* elements, int* profondeurs) const;
    void afficherEnOrdre(const Noeud* n, std::ostream& out) const;
};


//-----------------------------------------------------------------------------

template <class T>
ArbreAVL<T>::Noeud::Noeud(const T& c) : contenu(c), enleve(false), equilibre(0), tailleSousArbre(1), gauche(nullptr), droite(nullptr) {}

template <class T>
int ArbreAVL<T>::nombreElements(const Noeud* noeud) const {
    if(noeud == nullptr)
        return 0;

    return noeud->tailleSousArbre;
}

template <class T>
void ArbreAVL<T>::mettreAJourTaille(Noeud* noeud) {
    if(noeud == nullptr)
        return;

    noeud->tailleSousArbre =
        nombreElements(noeud->gauche)
        + nombreElements(noeud->droite)
        + (noeud->enleve ? 0 : 1);
}

template <class T>
ArbreAVL<T>::ArbreAVL() : racine(nullptr) {}

template <class T>
ArbreAVL<T>::ArbreAVL(const ArbreAVL<T>& autre) : racine(nullptr) {
    copier(autre.racine, racine);
}

template <class T>
ArbreAVL<T>::~ArbreAVL() {
    vider(racine);
}

template <class T>
bool ArbreAVL<T>::contient(const T& element) const {

    Noeud *current = racine;
    while(current)
        if(current->contenu < element)
            current = current->droite;
        else if(element < current ->contenu)
            current = current->gauche;
        else 
            return !current->enleve;
    
    return false;
}

template <class T>
void ArbreAVL<T>::inserer(const T& element) {
    inserer(racine, element);
}

template <class T>
bool ArbreAVL<T>::inserer(Noeud*& noeud, const T& element) {
    if(noeud == nullptr) {
        noeud = new Noeud(element);
        return true;
    }

    if(element < noeud->contenu) {
        bool hauteurAugmentee = inserer(noeud->gauche, element);

        mettreAJourTaille(noeud);

        if(hauteurAugmentee) {
            noeud->equilibre++;

            if(noeud->equilibre == 0)
                return false;
            if(noeud->equilibre == 1)
                return true;
            assert(noeud->equilibre == 2);

            if(noeud->gauche->equilibre == -1)
                rotationDroiteGauche(noeud->gauche);
            rotationGaucheDroite(noeud);
        }
        return false;
    } else if(noeud->contenu < element) {
        bool hauteurAugmentee = inserer(noeud->droite, element);

        mettreAJourTaille(noeud);

        if(hauteurAugmentee) {
            noeud->equilibre--;

            if(noeud->equilibre == 0)
                return false;
            if(noeud->equilibre == -1)
                return true;
            assert(noeud->equilibre == -2);

            if(noeud->droite->equilibre == 1)
                rotationGaucheDroite(noeud->droite);
            rotationDroiteGauche(noeud);
        }
        return false;
    }

    noeud->enleve = false;
    noeud->contenu = element;  
    mettreAJourTaille(noeud);
    return false;
}

template <class T>
void ArbreAVL<T>::rotationGaucheDroite(Noeud*& racinesousarbre) {
    rotations++;
    Noeud* ancienneRacine = racinesousarbre;
    Noeud *temp = racinesousarbre->gauche;

    int  ea = temp->equilibre;
    int  eb = racinesousarbre->equilibre;
    int  neb = -(ea > 0 ? ea : 0) - 1 + eb;
    int  nea = ea + (neb < 0 ? neb : 0) - 1;

    temp->equilibre = nea;
    racinesousarbre->equilibre = neb;
    racinesousarbre->gauche = temp->droite;
    temp->droite = racinesousarbre;
    racinesousarbre = temp;

    mettreAJourTaille(ancienneRacine);
    mettreAJourTaille(racinesousarbre);
}

template <class T>
void ArbreAVL<T>::rotationDroiteGauche(Noeud*& racinesousarbre) {
    rotations++;
    Noeud* ancienneRacine = racinesousarbre;
    Noeud *temp = racinesousarbre-> droite;

    int eb = temp->equilibre;
    int ea = racinesousarbre->equilibre;
    int nea = ea - (eb < 0 ? eb : 0) + 1;
    int neb = eb + (nea > 0 ? nea : 0) + 1;

    temp->equilibre = neb;
    racinesousarbre->equilibre = nea;
    racinesousarbre->droite = temp->gauche;
    temp->gauche = racinesousarbre;
    racinesousarbre = temp;
    
    mettreAJourTaille(ancienneRacine);
    mettreAJourTaille(racinesousarbre);
}

template <class T>
bool ArbreAVL<T>::vide() const
{
    return taille() == 0;
}

template <class T>
void ArbreAVL<T>::vider() {
  vider(racine);
  rotations = 0;
}

template <class T>
void ArbreAVL<T>::vider(Noeud*& noeud) {
    // À compléter.
    if(!noeud) return;

    vider(noeud->gauche);
    vider(noeud->droite);
    delete noeud;
    noeud = nullptr;
}

template <class T>
void ArbreAVL<T>::copier(const Noeud* source, Noeud*& destination) const {
    if(source == nullptr) {
        destination = nullptr;
        return;
    }

    destination = new Noeud(source->contenu);

    destination->enleve = source->enleve;
    destination->equilibre = source->equilibre;
    destination->tailleSousArbre = source->tailleSousArbre;

    copier(source->gauche, destination->gauche);
    copier(source->droite, destination->droite);
}

//
template <class T>
int ArbreAVL<T>::taille() const
{
    return nombreElements(racine);
}

//
template <class T>
ArbreAVL<T>& ArbreAVL<T>::operator=(const ArbreAVL& autre) {
    if(this == &autre)
        return *this;

    vider();
    copier(autre.racine, racine);
    return *this;
}

template <class T>
void ArbreAVL<T>::remplirTableau( const Noeud* noeud, const T** elements, int& indice ) const {
    if(noeud == nullptr)
        return;

    remplirTableau(noeud->gauche, elements, indice);

    if(!noeud->enleve) {
        elements[indice++] = &noeud->contenu;
    }

    remplirTableau(noeud->droite, elements, indice);
}


//
///
///  
///
//
template <class T>
bool ArbreAVL<T>::operator==(const ArbreAVL<T>& autre) const {
    if(this == &autre)
        return true;

    if(taille() != autre.taille())
        return false;

    int nombre = taille();

    if(nombre == 0)
        return true;

    const T** elements1 = new const T*[nombre];
    const T** elements2 = new const T*[nombre];

    int indice1 = 0;
    int indice2 = 0;

    remplirTableau(racine, elements1, indice1);
    autre.remplirTableau(autre.racine, elements2, indice2);

    bool egaux = true;

    for(int i = 0; i < nombre; i++) {
        if(!(*elements1[i] == *elements2[i])) {
            egaux = false;
            break;
        }
    }

    delete[] elements1;
    delete[] elements2;

    return egaux;
}


template <class T>
int  ArbreAVL<T>::compter() const {
    return compter(racine);
}

template <class T>
int ArbreAVL<T>::compter(const Noeud* n) const {
    if(n == nullptr)
        return 0;
    return compter(n->gauche) + compter(n->droite) + !n->enleve;
}

template <class T>
int ArbreAVL<T>::compterInferieurs(
    const Noeud* noeud,
    const T& limite
) const {
    if(noeud == nullptr)
        return 0;

    if(noeud->contenu < limite) {
        return nombreElements(noeud->gauche)
            + (noeud->enleve ? 0 : 1)
            + compterInferieurs(noeud->droite, limite);
    }

    return compterInferieurs(noeud->gauche, limite);
}

template <class T>
int ArbreAVL<T>::compterInferieursOuEgaux(
    const Noeud* noeud,
    const T& limite
) const {
    if(noeud == nullptr)
        return 0;

    if(limite < noeud->contenu)
        return compterInferieursOuEgaux(noeud->gauche, limite);

    return nombreElements(noeud->gauche)
        + (noeud->enleve ? 0 : 1)
        + compterInferieursOuEgaux(noeud->droite, limite);
}

//
///
///
///
//
template <class T>
int ArbreAVL<T>::compter(const T& min, const T& max) const {
    if(max < min) return 0;

    return compterInferieursOuEgaux(racine, max)
         - compterInferieurs(racine, min);
}

template <class T>
int  ArbreAVL<T>::hauteur() const {
    return hauteur(racine);
}

template <class T>
int  ArbreAVL<T>::hauteur(const Noeud* n) const {
    if(n == nullptr) return 0;

    return 1 + std::max(hauteur(n->gauche), hauteur(n->droite));
}


template <class T>
bool ArbreAVL<T>::enlever(Noeud* noeud, const T& element) {
    if(noeud == nullptr)
        return false;

    bool elementTrouve;

    if(element < noeud->contenu) {
        elementTrouve = enlever(noeud->gauche, element);
    }
    else if(noeud->contenu < element) {
        elementTrouve = enlever(noeud->droite, element);
    }
    else {
        if(noeud->enleve)
            return false;

        noeud->enleve = true;
        mettreAJourTaille(noeud);
        return true;
    }

    if(elementTrouve)
        mettreAJourTaille(noeud);

    return elementTrouve;
}

template <class T>
void ArbreAVL<T>::enlever(const T& element) {
    bool elementTrouve = enlever(racine, element);
    assert(elementTrouve);
}

template <class T>
void ArbreAVL<T>::ajouterSousEnsemble(
    const Noeud* noeud,
    const T& min,
    const T& max,
    ArbreAVL<T>& resultat
) const {
    if(noeud == nullptr) return;

    if(noeud->contenu < min) {
        ajouterSousEnsemble(noeud->droite, min, max, resultat);
        return;
    }

    if(max < noeud->contenu) {
        ajouterSousEnsemble(noeud->gauche, min, max, resultat);
        return;
    }

    if(!noeud->enleve)
        resultat.inserer(noeud->contenu);

    ajouterSousEnsemble(noeud->gauche, min, max, resultat);
    ajouterSousEnsemble(noeud->droite, min, max, resultat);
}


//
///
///
///
//
template <class T>
const T& ArbreAVL<T>::rechercher(const T& element) const
{
    Noeud* courant = racine;

    while(courant != nullptr)
    {
        if(element < courant->contenu)
        {
            courant = courant->gauche;
        }
        else if(courant->contenu < element)
        {
            courant = courant->droite;
        }
        else
        {
            assert(!courant->enleve);
            return courant->contenu;
        }
    }

    assert(false);
    return racine->contenu;
}

template <class T>
T& ArbreAVL<T>::rechercher(const T& element)
{
    Noeud* courant = racine;

    while(courant != nullptr)
    {
        if(element < courant->contenu)
        {
            courant = courant->gauche;
        }
        else if(courant->contenu < element)
        {
            courant = courant->droite;
        }
        else
        {
            assert(!courant->enleve);
            return courant->contenu;
        }
    }

    assert(false);
    return racine->contenu;
}

template <class T>
bool ArbreAVL<T>::rechercherEgalOuApres(
    const T& element,
    T& resultat
) const
{
    Noeud* courant = racine;
    Noeud* candidat = nullptr;

    while(courant != nullptr)
    {
        if(courant->contenu < element)
        {
            courant = courant->droite;
        }
        else
        {
            if(!courant->enleve)
                candidat = courant;

            courant = courant->gauche;
        }
    }

    if(candidat == nullptr)
        return false;

    resultat = candidat->contenu;
    return true;
}

template <class T>
bool ArbreAVL<T>::rechercherEgalOuAvant(
    const T& element,
    T& resultat
) const
{
    Noeud* courant = racine;
    Noeud* candidat = nullptr;

    while(courant != nullptr)
    {
        if(element < courant->contenu)
        {
            courant = courant->gauche;
        }
        else
        {
            if(!courant->enleve)
                candidat = courant;

            courant = courant->droite;
        }
    }

    if(candidat == nullptr)
        return false;

    resultat = candidat->contenu;
    return true;
}

//
///
///
///
//
template <class T>
void ArbreAVL<T>::remplirIntervalle(
    const T& min,
    const T& max,
    T* elements,
    int& nombre
) const
{
    nombre = 0;

    if(max < min)
        return;

    remplirIntervalle(
        racine,
        min,
        max,
        elements,
        nombre
    );
}


template <class T>
void ArbreAVL<T>::remplirIntervalle(
    const Noeud* noeud,
    const T& min,
    const T& max,
    T* elements,
    int& nombre
) const
{
    if(noeud == nullptr)
        return;

    if(min < noeud->contenu)
    {
        remplirIntervalle(
            noeud->gauche,
            min,
            max,
            elements,
            nombre
        );
    }

    bool dansIntervalle =
        !(noeud->contenu < min)
        && !(max < noeud->contenu);

    if(dansIntervalle && !noeud->enleve)
    {
        elements[nombre] = noeud->contenu;
        nombre++;
    }

    if(noeud->contenu < max)
    {
        remplirIntervalle(
            noeud->droite,
            min,
            max,
            elements,
            nombre
        );
    }
}



//
///
///
///
//
template <class T>
ArbreAVL<T> ArbreAVL<T>::sous_ensemble(const T& min, const T& max) const{
    ArbreAVL<T> resultat;

    if(max < min) return resultat;

    ajouterSousEnsemble(racine, min, max, resultat);

    return resultat;
}

//-----------------------------

// Code fourni pour afficher l'arbre.
// L'opérateur ostream& << const T&  doit être défini.
template <class T>
void ArbreAVL<T>::afficher() const {
    std::cout << "Contenu de l'arbre AVL (";
    int n = compter();
    std::cout << n << " noeuds) :\n";
    T* elements = new T[n];
    int* profondeurs = new int[n];

    n = 0;
    preparer_afficher(racine, 0, n, elements, profondeurs);
    for(int p = 0;; p++) {
        bool derniereprofondeur = true;
        for(int i = 0; i < n; i++) {
            if(profondeurs[i] == p) {
                std::cout << elements[i];
                derniereprofondeur = false;
            }
            std::cout << '\t';
        }
        std::cout << '\n';
        if(derniereprofondeur) break;
    }

    delete[] elements;
    delete[] profondeurs;
    std::cout << "--------------" << std::endl;
}

// Affiche les éléments actifs en ordre croissant.
template <class T>
void ArbreAVL<T>::afficher(std::ostream& out) const
{
    afficherEnOrdre(racine, out);
}

template <class T>
void ArbreAVL<T>::afficherEnOrdre(
    const Noeud* noeud,
    std::ostream& out
) const
{
    if(noeud == nullptr)
        return;

    afficherEnOrdre(noeud->gauche, out);

    if(!noeud->enleve)
        out << noeud->contenu << ' ';

    afficherEnOrdre(noeud->droite, out);
}

template <class T>
void ArbreAVL<T>::preparer_afficher(const Noeud* n, int profondeur, int& rang, T* elements, int* profondeurs) const {
    if(n == nullptr) return;

    preparer_afficher(n->gauche, profondeur + 1, rang, elements, profondeurs);
    if(!n->enleve) {
        elements[rang] = n->contenu;
        profondeurs[rang] = profondeur;
        rang++;
    }
    preparer_afficher(n->droite, profondeur + 1, rang, elements, profondeurs);
}

template <class T>
int ArbreAVL<T>::nombreRotations() const {
    return rotations;
}

template <class T>
int ArbreAVL<T>::construireDessin(const Noeud* n, int profondeur, int& rangCourant, DessinArbre<T>& sortie) const {
    if(n == nullptr || n->enleve)
        return -1;

    int indiceGauche = construireDessin(n->gauche, profondeur + 1, rangCourant, sortie);

    int indice = (int)sortie.size();
    sortie.push_back(NoeudDessin<T>{n->contenu, profondeur, rangCourant, indiceGauche, -1});
    rangCourant++;

    int indiceDroit = construireDessin(n->droite, profondeur + 1, rangCourant, sortie);
    sortie[indice].enfantDroit = indiceDroit;

    return indice;
}

template <class T>
DessinArbre<T> ArbreAVL<T>::exporterPourDessin() const {
    DessinArbre<T> sortie;
    int rang = 0;
    construireDessin(racine, 0, rang, sortie);
    return sortie;
}



#endif
/* dessinarbre.h
 * Structure partagee par les trois arbres (AVL, rouge-noir, BST) pour
 * exporter leur forme sous une forme facile a dessiner : un tableau de
 * noeuds avec, pour chacun, sa profondeur (axe Y), son rang in-ordre
 * (axe X, avant mise a l'echelle) et les indices de ses enfants dans ce
 * meme tableau (ou -1 s'il n'y en a pas).
 *
 * Le rang in-ordre est la technique classique de disposition d'un arbre
 * binaire : parcourir l'arbre en-ordre (gauche, noeud, droite) et
 * numeroter les noeuds dans cet ordre donne automatiquement une
 * disposition horizontale sans chevauchement, puisque l'ordre in-ordre
 * EST l'ordre trie des valeurs.
 */
#if !defined(_DESSINARBRE___H_)
#define _DESSINARBRE___H_

#include <vector>

template <class T>
struct NoeudDessin {
    T valeur;
    int profondeur;       // 0 = racine
    int rangInOrdre;      // position horizontale avant mise a l'echelle
    int enfantGauche;     // indice dans le vecteur, -1 si absent
    int enfantDroit;      // indice dans le vecteur, -1 si absent
    bool rouge = false;   // utilise seulement par l'arbre rouge-noir
};

template <class T>
using DessinArbre = std::vector<NoeudDessin<T>>;

#endif

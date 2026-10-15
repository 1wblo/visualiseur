/* arbrebst.h
 * Arbre binaire de recherche simple, SANS rééquilibrage. Sert de
 * référence dans le comparatif : montre à quoi ressemble un arbre
 * construit sans aucune stratégie d'équilibrage, pour la même séquence
 * de valeurs que l'AVL et l'arbre rouge-noir.
 */
#if !defined(_ARBREBST___H_)
#define _ARBREBST___H_

#include "dessinarbre.h"

template <class T>
class ArbreBST {
public:
    ArbreBST() : racine(nullptr) {}
    ~ArbreBST() { vider(racine); }

    void inserer(const T& valeur) { inserer(racine, valeur); }

    void effacer() { vider(racine); }

    int hauteur() const { return hauteur(racine); }

    /// Toujours 0 : un BST simple ne fait jamais de rotation. Le but est
    /// justement de montrer ce que ça coûte en hauteur de ne PAS en faire.
    int nombreRotations() const { return 0; }

    DessinArbre<T> exporterPourDessin() const {
        DessinArbre<T> sortie;
        int rang = 0;
        construireDessin(racine, 0, rang, sortie);
        return sortie;
    }

private:
    struct Noeud {
        T valeur;
        Noeud* gauche = nullptr;
        Noeud* droite = nullptr;
        Noeud(const T& v) : valeur(v) {}
    };
    Noeud* racine;

    void inserer(Noeud*& noeud, const T& valeur) {
        if(noeud == nullptr) {
            noeud = new Noeud(valeur);
            return;
        }
        if(valeur < noeud->valeur)
            inserer(noeud->gauche, valeur);
        else if(noeud->valeur < valeur)
            inserer(noeud->droite, valeur);
        // valeur deja presente : on ne fait rien (pas de doublons)
    }

    int hauteur(const Noeud* n) const {
        if(n == nullptr) return 0;
        int g = hauteur(n->gauche);
        int d = hauteur(n->droite);
        return 1 + (g > d ? g : d);
    }

    void vider(Noeud*& n) {
        if(!n) return;
        vider(n->gauche);
        vider(n->droite);
        delete n;
        n = nullptr;
    }

    int construireDessin(const Noeud* n, int profondeur, int& rangCourant, DessinArbre<T>& sortie) const {
        if(n == nullptr) return -1;

        int indiceGauche = construireDessin(n->gauche, profondeur + 1, rangCourant, sortie);

        int indice = (int)sortie.size();
        sortie.push_back(NoeudDessin<T>{n->valeur, profondeur, rangCourant, indiceGauche, -1});
        rangCourant++;

        int indiceDroit = construireDessin(n->droite, profondeur + 1, rangCourant, sortie);
        sortie[indice].enfantDroit = indiceDroit;

        return indice;
    }
};

#endif

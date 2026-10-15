/* arbrerougenoir.h
 * Arbre rouge-noir generique, ecrit pour ce projet. Un arbre rouge-noir
 * est, comme l'AVL, un arbre binaire de recherche auto-equilibre, mais
 * sa regle d'equilibre est differente : au lieu de comparer les
 * hauteurs des deux sous-arbres (AVL), chaque noeud porte une couleur
 * (rouge ou noir) et l'arbre respecte 4 regles :
 *   1. La racine est toujours noire.
 *   2. Un noeud rouge n'a jamais d'enfant rouge (pas deux rouges
 *      d'affilee).
 *   3. Tout chemin de la racine a une feuille (ou position vide)
 *      passe par le meme nombre de noeuds NOIRS.
 *   4. Un noeud "absent" (nullptr) est considere noir.
 *
 * Ces regles garantissent que l'arbre reste "raisonnablement" equilibre
 * (hauteur au plus 2*log2(n+1)), mais tolerent plus de desequilibre
 * local que l'AVL -> en general MOINS de rotations a l'insertion, mais
 * une hauteur finale un peu plus grande. C'est exactement ce que ce
 * projet rend visible en comparant les deux sur la meme sequence.
 */
#if !defined(_ARBREROUGENOIR___H_)
#define _ARBREROUGENOIR___H_

#include "dessinarbre.h"

template <class T>
class ArbreRougeNoir {
public:
    ArbreRougeNoir() : racine(nullptr) {}
    ~ArbreRougeNoir() { vider(racine); }

    void inserer(const T& valeur) {
        Noeud* parent = nullptr;
        Noeud* courant = racine;
        while(courant != nullptr) {
            parent = courant;
            if(valeur < courant->valeur)
                courant = courant->gauche;
            else if(courant->valeur < valeur)
                courant = courant->droite;
            else
                return; // deja present : pas de doublon
        }

        Noeud* z = new Noeud(valeur);
        z->parent = parent;
        if(parent == nullptr)
            racine = z;
        else if(valeur < parent->valeur)
            parent->gauche = z;
        else
            parent->droite = z;

        corrigerApresInsertion(z);
    }

    void effacer() { vider(racine); rotations = 0; recolorations = 0; }

    int hauteur() const { return hauteur(racine); }
    int nombreRotations() const { return rotations; }
    /// Compteur bonus : nombre de noeuds recolores pendant l'equilibrage.
    /// Un arbre rouge-noir recolore souvent SANS tourner ; l'AVL n'a pas
    /// cette notion (il ne fait que des rotations).
    int nombreRecolorations() const { return recolorations; }

    DessinArbre<T> exporterPourDessin() const {
        DessinArbre<T> sortie;
        int rang = 0;
        construireDessin(racine, 0, rang, sortie);
        return sortie;
    }

    /// Verifie les 4 regles d'un arbre rouge-noir valide (utilise par
    /// les tests). Retourne false au premier manquement detecte.
    bool estValide() const {
        if(estRouge(racine)) return false; // regle 1 : racine noire
        int hn = -1;
        return verifier(racine, hn);
    }

private:
    enum class Couleur { Rouge, Noir };

    struct Noeud {
        T valeur;
        Couleur couleur;
        Noeud* gauche = nullptr;
        Noeud* droite = nullptr;
        Noeud* parent = nullptr;
        Noeud(const T& v) : valeur(v), couleur(Couleur::Rouge) {}
    };

    Noeud* racine;
    int rotations = 0;
    int recolorations = 0;

    static bool estRouge(const Noeud* n) {
        return n != nullptr && n->couleur == Couleur::Rouge;
    }

    void rotationGauche(Noeud* x) {
        rotations++;
        Noeud* y = x->droite;
        x->droite = y->gauche;
        if(y->gauche) y->gauche->parent = x;
        y->parent = x->parent;
        if(x->parent == nullptr) racine = y;
        else if(x == x->parent->gauche) x->parent->gauche = y;
        else x->parent->droite = y;
        y->gauche = x;
        x->parent = y;
    }

    void rotationDroite(Noeud* x) {
        rotations++;
        Noeud* y = x->gauche;
        x->gauche = y->droite;
        if(y->droite) y->droite->parent = x;
        y->parent = x->parent;
        if(x->parent == nullptr) racine = y;
        else if(x == x->parent->droite) x->parent->droite = y;
        else x->parent->gauche = y;
        y->droite = x;
        x->parent = y;
    }

    // Suit l'algorithme classique (CLRS) : apres avoir insere z en rouge,
    // on remonte en corrigeant deux rouges consecutifs soit par
    // recoloration (si l'"oncle" est rouge), soit par rotation (sinon).
    void corrigerApresInsertion(Noeud* z) {
        while(z->parent != nullptr && estRouge(z->parent)) {
            Noeud* p = z->parent;
            Noeud* g = p->parent; // existe forcement : la racine est toujours noire

            if(p == g->gauche) {
                Noeud* oncle = g->droite;
                if(estRouge(oncle)) {
                    p->couleur = Couleur::Noir;
                    oncle->couleur = Couleur::Noir;
                    g->couleur = Couleur::Rouge;
                    recolorations += 3;
                    z = g;
                } else {
                    if(z == p->droite) {
                        z = p;
                        rotationGauche(z);
                        p = z->parent;
                    }
                    p->couleur = Couleur::Noir;
                    g->couleur = Couleur::Rouge;
                    recolorations += 2;
                    rotationDroite(g);
                }
            } else {
                Noeud* oncle = g->gauche;
                if(estRouge(oncle)) {
                    p->couleur = Couleur::Noir;
                    oncle->couleur = Couleur::Noir;
                    g->couleur = Couleur::Rouge;
                    recolorations += 3;
                    z = g;
                } else {
                    if(z == p->gauche) {
                        z = p;
                        rotationDroite(z);
                        p = z->parent;
                    }
                    p->couleur = Couleur::Noir;
                    g->couleur = Couleur::Rouge;
                    recolorations += 2;
                    rotationGauche(g);
                }
            }
        }
        if(racine->couleur != Couleur::Noir) {
            racine->couleur = Couleur::Noir;
            recolorations++;
        }
    }

    // Parcourt recursivement en verifiant l'ordre BST, l'absence de deux
    // rouges consecutifs et la hauteur-noire constante sur tous les
    // chemins ; `hauteurNoire` sert a comparer les sous-arbres entre eux
    // (-1 = pas encore fixee).
    bool verifier(const Noeud* n, int& hauteurNoire) const {
        if(n == nullptr) {
            if(hauteurNoire == -1) hauteurNoire = 0;
            return hauteurNoire == 0;
        }

        if(estRouge(n) && (estRouge(n->gauche) || estRouge(n->droite)))
            return false; // regle 2

        if(n->gauche && !(n->gauche->valeur < n->valeur)) return false;
        if(n->droite && !(n->valeur < n->droite->valeur)) return false;

        int hg = -1, hd = -1;
        if(!verifier(n->gauche, hg)) return false;
        if(!verifier(n->droite, hd)) return false;
        if(hg != hd) return false; // regle 3

        int ici = hg + (estRouge(n) ? 0 : 1);
        if(hauteurNoire == -1) hauteurNoire = ici;
        return hauteurNoire == ici;
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
        NoeudDessin<T> nd{n->valeur, profondeur, rangCourant, indiceGauche, -1};
        nd.rouge = estRouge(n);
        sortie.push_back(nd);
        rangCourant++;

        int indiceDroit = construireDessin(n->droite, profondeur + 1, rangCourant, sortie);
        sortie[indice].enfantDroit = indiceDroit;

        return indice;
    }
};

#endif

/* test_arbres.cpp
 * Tests independants de l'affichage SFML : verifie que chaque structure
 * reste correcte apres insertion (ordre, hauteur bornee, invariants
 * rouge-noir) sur des sequences connues et sur des sequences aleatoires.
 */
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include "arbreavl.h"
#include "arbrebst.h"
#include "arbrerougenoir.h"

// Verifie que l'export pour dessin respecte bien l'ordre croissant des
// rangs in-ordre = ordre trie des valeurs (sinon le dessin serait faux).
template <class Arbre>
void verifierOrdreDessin(const Arbre& a) {
    auto dessin = a.exporterPourDessin();
    std::vector<std::pair<int,int>> parRang; // (rang, valeur)
    for(auto& n : dessin) parRang.push_back({n.rangInOrdre, n.valeur});
    std::sort(parRang.begin(), parRang.end());
    for(size_t i = 0; i + 1 < parRang.size(); i++)
        assert(parRang[i].second < parRang[i+1].second);
}

int main() {
    // --- Sequence connue : force des rotations sur AVL et rouge-noir ---
    std::vector<int> seqMontante = {1, 2, 3, 4, 5, 6, 7};

    ArbreAVL<int> avl1;
    ArbreRougeNoir<int> rn1;
    ArbreBST<int> bst1;
    for(int v : seqMontante) { avl1.inserer(v); rn1.inserer(v); bst1.inserer(v); }

    // BST sans equilibrage sur une sequence montante = chaine degeneree :
    // hauteur = nombre d'elements.
    assert(bst1.hauteur() == (int)seqMontante.size());
    assert(bst1.nombreRotations() == 0);

    // AVL et rouge-noir doivent rester bien plus plats (log2(7) ~ 2.8).
    assert(avl1.hauteur() <= 4);
    assert(rn1.hauteur() <= 5); // RN tolere un peu plus de hauteur que l'AVL
    assert(avl1.nombreRotations() > 0);  // la sequence montante DOIT forcer des rotations
    assert(rn1.estValide());

    verifierOrdreDessin(avl1);
    verifierOrdreDessin(rn1);
    verifierOrdreDessin(bst1);

    // --- Sequences aleatoires : les invariants doivent tenir a chaque fois ---
    std::mt19937 rng(42);
    for(int essai = 0; essai < 50; essai++) {
        std::vector<int> valeurs(30);
        std::iota(valeurs.begin(), valeurs.end(), 0);
        std::shuffle(valeurs.begin(), valeurs.end(), rng);

        ArbreAVL<int> avl;
        ArbreRougeNoir<int> rn;
        ArbreBST<int> bst;
        for(int v : valeurs) { avl.inserer(v); rn.inserer(v); bst.inserer(v); }

        assert(rn.estValide());
        assert(avl.exporterPourDessin().size() == 30);
        assert(rn.exporterPourDessin().size() == 30);
        assert(bst.exporterPourDessin().size() == 30);

        // Les deux arbres equilibres doivent respecter la borne theorique
        // de hauteur (AVL <= 1.44*log2(n+2), RN <= 2*log2(n+1)).
        double bornAVL = 1.45 * std::log2(32);
        double bornRN  = 2.0 * std::log2(31);
        assert(avl.hauteur() <= bornAVL);
        assert(rn.hauteur() <= bornRN);

        // L'AVL reste toujours au moins aussi plat que le BST non equilibre.
        assert(avl.hauteur() <= bst.hauteur());

        verifierOrdreDessin(avl);
        verifierOrdreDessin(rn);
        verifierOrdreDessin(bst);
    }

    // --- Doublons : ne doivent pas faire planter ni dupliquer un noeud ---
    ArbreRougeNoir<int> rn2;
    for(int v : {5, 5, 5, 3, 7, 3}) rn2.inserer(v);
    assert(rn2.exporterPourDessin().size() == 3);
    assert(rn2.estValide());

    std::cout << "Tous les tests sont passes." << std::endl;
    return 0;
}

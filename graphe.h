/* Graphe<S> : representation generique d'un graphe (oriente ou non) par
 * liste d'adjacence (std::map<S, Sommet>, chaque Sommet gardant
 * l'ensemble de ses voisins sortants). Fournit les parcours de base
 * (profondeur, largeur) et l'extraction des composantes connexes.
 */
#ifndef GRAPHE_H
#define GRAPHE_H

#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>

// Le type S est le symbole pour identifier un sommet
template<class S>
class Graphe {
 public:
  // Interface public pour créer le graphe.
  void ajouterSommet(const S& s);
  void ajouterAreteOrientee(const S& s1, const S& s2);
  void ajouterAreteNonOrientee(const S& s1, const S& s2);

  // Interface public pour les requêtes de base.
  // Amélioration possible: retourner le résultat au lieu de faire l'affichage à l'interne.
  void parcoursRechercheProfondeur(const S& s) const;
  void parcoursRechercheLargueur(const S& s) const;
  void extraireComposantesConnexes() const;

  struct Sommet {
    // À compléter.
    std::set<S> voisins;  // ensemble des sommets accessibles via les arêtes
                          // sortantes du sommet. Cela est légèrement différent
                          // de la page 128 des notes de cours. C'est voulu, car
                          // ici les arêtes ne sont pas étiquetées par un poids
                          // (ex: distance). Pour attacher une étiquette, il
                          // suffirait de modifier pour : map<S, A> sortants;
  };
  std::map<S, Sommet> sommets;  // identification --> sommet

 private:
  // Fonction auxiliaire (récursive) utilisée par parcoursRechercheProfondeur.
  void parcoursProfondeurAux(const S& s, std::set<S>& visites) const;
};

template<class S>
void Graphe<S>::ajouterSommet(const S& s) {
  // operator[] insère automatiquement un Sommet "vide" si la clé n'existe
  // pas encore. Si le sommet existe déjà, on ne touche à rien (les
  // voisins déjà enregistrés restent intacts).
  sommets[s];
}

template<class S>
void Graphe<S>::ajouterAreteNonOrientee(const S& s1, const S& s2) {
  // Doit ajouter les 2 arêtes orientées : s1->s2 et s2->s1;
  ajouterAreteOrientee(s1, s2);
  ajouterAreteOrientee(s2, s1);
}

template<class S>
void Graphe<S>::ajouterAreteOrientee(const S& s1, const S& s2) {
  // Doit uniquement ajouter s1->s2.
  // On s'assure que les deux sommets existent avant de créer l'arête,
  // même si un des deux sommets n'a jamais été ajouté explicitement.
  ajouterSommet(s1);
  ajouterSommet(s2);
  sommets[s1].voisins.insert(s2);
}

template<class S>
void Graphe<S>::parcoursProfondeurAux(const S& s, std::set<S>& visites) const {
  if (visites.find(s) != visites.end()) {
    return;  // déjà visité, on arrête cette branche
  }
  visites.insert(s);
  std::cout << s << " ";

  typename std::map<S, Sommet>::const_iterator itSommet = sommets.find(s);
  if (itSommet == sommets.end()) {
    return;  // sommet inconnu, rien à explorer
  }

  const std::set<S>& voisins = itSommet->second.voisins;
  for (typename std::set<S>::const_iterator itVoisin = voisins.begin();
       itVoisin != voisins.end(); ++itVoisin) {
    parcoursProfondeurAux(*itVoisin, visites);
  }
}

template<class S>
void Graphe<S>::parcoursRechercheProfondeur(const S& s) const {
  // Vous devrez implémenter des marqueurs «visité» sur les sommets.
  // Ici, on utilise un std::set local plutôt qu'un marqueur stocké dans le
  // Sommet, ce qui évite d'avoir à "réinitialiser" les marqueurs entre deux
  // appels (et permet de garder la fonction const).
  std::set<S> visites;
  parcoursProfondeurAux(s, visites);
  std::cout << std::endl;
}

template<class S>
void Graphe<S>::parcoursRechercheLargueur(const S& s) const {
  // Il est recommandé d'utiliser la file std::queue de la bibliothèque
  // standard de C++. Voir : http://en.cppreference.com/w/cpp/container/queue
  std::set<S> visites;
  std::queue<S> file;

  if (sommets.find(s) == sommets.end()) {
    return;  // sommet de départ inexistant
  }

  file.push(s);
  visites.insert(s);

  while (!file.empty()) {
    S courant = file.front();
    file.pop();
    std::cout << courant << " ";

    typename std::map<S, Sommet>::const_iterator itSommet = sommets.find(courant);
    const std::set<S>& voisins = itSommet->second.voisins;
    for (typename std::set<S>::const_iterator itVoisin = voisins.begin();
         itVoisin != voisins.end(); ++itVoisin) {
      if (visites.find(*itVoisin) == visites.end()) {
        visites.insert(*itVoisin);
        file.push(*itVoisin);
      }
    }
  }
  std::cout << std::endl;
}

template<class S>
void Graphe<S>::extraireComposantesConnexes() const {
  // Ne faites que l'extraction simple en premier (plus simple que
  // l'extraction de composantes *fortement* connexes).
  // On parcourt tous les sommets; pour chaque sommet pas encore visité,
  // on fait un parcours en largeur pour récupérer toute sa composante.
  std::set<S> visitesGlobal;

  for (typename std::map<S, Sommet>::const_iterator itS = sommets.begin();
       itS != sommets.end(); ++itS) {
    const S& depart = itS->first;
    if (visitesGlobal.find(depart) != visitesGlobal.end()) {
      continue;  // déjà dans une composante trouvée précédemment
    }

    std::vector<S> composante;
    std::queue<S> file;
    file.push(depart);
    visitesGlobal.insert(depart);

    while (!file.empty()) {
      S courant = file.front();
      file.pop();
      composante.push_back(courant);

      typename std::map<S, Sommet>::const_iterator itCourant = sommets.find(courant);
      const std::set<S>& voisins = itCourant->second.voisins;
      for (typename std::set<S>::const_iterator itV = voisins.begin();
           itV != voisins.end(); ++itV) {
        if (visitesGlobal.find(*itV) == visitesGlobal.end()) {
          visitesGlobal.insert(*itV);
          file.push(*itV);
        }
      }
    }

    std::cout << "Communaute (" << composante.size() << " membres) : ";
    for (std::size_t i = 0; i < composante.size(); ++i) {
      std::cout << composante[i];
      if (i + 1 < composante.size()) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
  }
}

#endif

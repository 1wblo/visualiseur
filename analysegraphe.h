/* analysegraphe.h
 * Fonctions d'analyse d'un Graphe<string> generique, utilisees par la
 * vue "graphe" du visualiseur pour mettre en evidence ses points
 * d'articulation (algorithme de Tarjan) : les sommets dont le retrait
 * deconnecterait le graphe en plusieurs morceaux.
 */
#ifndef ANALYSEGRAPHE_H
#define ANALYSEGRAPHE_H

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "graphe.h"

using GrapheTexte = Graphe<std::string>;

namespace detail {

inline void dfsArticulation(const GrapheTexte& g, const std::string& u, bool aUnParent,
                             const std::string& parent, std::map<std::string, bool>& visite,
                             std::map<std::string, int>& decouverte, std::map<std::string, int>& bas,
                             std::set<std::string>& articulations, int& temps) {
    int nbEnfants = 0;
    visite[u] = true;
    decouverte[u] = temps;
    bas[u] = temps;
    temps++;

    auto itU = g.sommets.find(u);
    for (const std::string& v : itU->second.voisins) {
        if (!visite[v]) {
            nbEnfants++;
            dfsArticulation(g, v, true, u, visite, decouverte, bas, articulations, temps);
            bas[u] = std::min(bas[u], bas[v]);
            if (aUnParent && bas[v] >= decouverte[u]) {
                articulations.insert(u);
            }
        } else if (!aUnParent || v != parent) {
            bas[u] = std::min(bas[u], decouverte[v]);
        }
    }

    if (!aUnParent && nbEnfants > 1) {
        articulations.insert(u);
    }
}

} // namespace detail

/// Points d'articulation du graphe (algorithme de Tarjan) : les sommets
/// dont le retrait deconnecterait le reste du graphe.
inline std::set<std::string> pointsArticulation(const GrapheTexte& g) {
    std::map<std::string, bool> visite;
    std::map<std::string, int> decouverte, bas;
    std::set<std::string> articulations;
    int temps = 0;

    for (const auto& paire : g.sommets) visite[paire.first] = false;

    for (const auto& paire : g.sommets) {
        if (!visite[paire.first]) {
            detail::dfsArticulation(g, paire.first, false, "", visite, decouverte, bas, articulations, temps);
        }
    }
    return articulations;
}

#endif

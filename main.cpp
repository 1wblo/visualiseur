/* main.cpp — Visualiseur de structures de donnees
 *
 * Insere la MEME sequence aleatoire, une valeur a la fois, dans trois
 * arbres differents (AVL, rouge-noir, BST simple non equilibre) et les
 * dessine cote a cote, pour rendre visible la difference entre leurs
 * strategies d'equilibrage. Une deuxieme vue montre un graphe aleatoire
 * avec ses points d'articulation (aucun lien avec les arbres : c'est
 * une structure separee, juste presentee dans le meme programme).
 *
 * Controles :
 *   Espace / clic gauche : insere la prochaine valeur dans les 3 arbres
 *   F                     : insere toutes les valeurs restantes d'un coup
 *   R                     : recommence avec une nouvelle sequence aleatoire
 *   G                     : bascule entre la vue "arbres" et la vue "graphe"
 *   Echap                 : quitte
 */
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "analysegraphe.h"
#include "arbreavl.h"
#include "arbrebst.h"
#include "arbrerougenoir.h"
#include "graphe.h"

namespace {

constexpr unsigned int LARGEUR_FENETRE = 1500;
constexpr unsigned int HAUTEUR_FENETRE = 880;
constexpr int NOMBRE_VALEURS = 20;
constexpr float RAYON_NOEUD = 15.f;

const sf::Color COULEUR_FOND(24, 26, 32);
const sf::Color COULEUR_AVL(70, 130, 220);
const sf::Color COULEUR_BST(150, 110, 200);
const sf::Color COULEUR_RN_ROUGE(210, 60, 60);
const sf::Color COULEUR_RN_NOIR(40, 40, 48);
const sf::Color COULEUR_ARETE(90, 95, 105);
const sf::Color COULEUR_TEXTE(230, 230, 235);
const sf::Color COULEUR_SOMMET(80, 150, 210);
const sf::Color COULEUR_ARTICULATION(230, 150, 50);

enum class Vue { Arbres, Graphe };

std::vector<int> nouvelleSequence(std::mt19937& rng) {
    std::vector<int> valeurs(NOMBRE_VALEURS);
    std::iota(valeurs.begin(), valeurs.end(), 1);
    std::shuffle(valeurs.begin(), valeurs.end(), rng);
    return valeurs;
}

// Dessine un arbre (sous forme deja exportee) dans la zone
// [x0, x0+largeur] x [y0, y0+hauteurZone], avec un espacement vertical
// commun `espacementNiveau` (pour que la hauteur se compare visuellement
// de facon juste entre les 3 colonnes).
void dessinerArbre(sf::RenderWindow& fenetre, sf::Font& police,
                    const DessinArbre<int>& dessin, float x0, float largeur, float y0,
                    float espacementNiveau, sf::Color couleurDefaut, bool utiliserRouge) {
    if(dessin.empty()) return;

    int n = (int)dessin.size();
    float marge = 35.f;
    float largeurUtile = std::max(largeur - 2 * marge, 1.f);

    auto position = [&](const NoeudDessin<int>& nd) -> sf::Vector2f {
        float fx = (n <= 1) ? 0.5f : (float)nd.rangInOrdre / (float)(n - 1);
        float x = x0 + marge + fx * largeurUtile;
        float y = y0 + nd.profondeur * espacementNiveau;
        return {x, y};
    };

    // Aretes d'abord (pour qu'elles passent sous les cercles).
    for(const auto& nd : dessin) {
        sf::Vector2f p1 = position(nd);
        if(nd.enfantGauche != -1) {
            sf::Vector2f p2 = position(dessin[nd.enfantGauche]);
            sf::Vertex ligne[] = {sf::Vertex{p1, COULEUR_ARETE}, sf::Vertex{p2, COULEUR_ARETE}};
            fenetre.draw(ligne, 2, sf::PrimitiveType::Lines);
        }
        if(nd.enfantDroit != -1) {
            sf::Vector2f p2 = position(dessin[nd.enfantDroit]);
            sf::Vertex ligne[] = {sf::Vertex{p1, COULEUR_ARETE}, sf::Vertex{p2, COULEUR_ARETE}};
            fenetre.draw(ligne, 2, sf::PrimitiveType::Lines);
        }
    }

    for(const auto& nd : dessin) {
        sf::Vector2f p = position(nd);

        sf::CircleShape cercle(RAYON_NOEUD);
        cercle.setOrigin({RAYON_NOEUD, RAYON_NOEUD});
        cercle.setPosition(p);
        sf::Color couleur = couleurDefaut;
        if(utiliserRouge) couleur = nd.rouge ? COULEUR_RN_ROUGE : COULEUR_RN_NOIR;
        cercle.setFillColor(couleur);
        cercle.setOutlineColor(sf::Color(15, 15, 18));
        cercle.setOutlineThickness(1.5f);
        fenetre.draw(cercle);

        sf::Text texte(police, std::to_string(nd.valeur), 12);
        texte.setFillColor(sf::Color::White);
        sf::FloatRect bornes = texte.getLocalBounds();
        texte.setOrigin({bornes.position.x + bornes.size.x / 2.f, bornes.position.y + bornes.size.y / 2.f});
        texte.setPosition(p);
        fenetre.draw(texte);
    }
}

void dessinerTexte(sf::RenderWindow& fenetre, sf::Font& police, const std::string& contenu, float x, float y,
                    unsigned int taille, sf::Color couleur) {
    sf::Text texte(police, contenu, taille);
    texte.setFillColor(couleur);
    texte.setPosition({x, y});
    fenetre.draw(texte);
}

// Layout circulaire simple (pas de notion de coordonnees dans Graphe<S>) :
// place les n sommets regulierement sur un cercle.
std::map<std::string, sf::Vector2f> layoutCirculaire(const GrapheTexte& g, sf::Vector2f centre, float rayon) {
    std::map<std::string, sf::Vector2f> positions;
    int n = (int)g.sommets.size();
    int i = 0;
    for(const auto& paire : g.sommets) {
        float angle = 2.f * 3.14159265f * (float)i / (float)std::max(n, 1);
        positions[paire.first] = {centre.x + rayon * std::cos(angle), centre.y + rayon * std::sin(angle)};
        i++;
    }
    return positions;
}

GrapheTexte genererGrapheAleatoire(std::mt19937& rng, int nbSommets) {
    GrapheTexte g;
    std::vector<std::string> noms;
    for(int i = 0; i < nbSommets; i++) {
        noms.push_back(std::to_string(i));
        g.ajouterSommet(noms.back());
    }
    // Chaine de base pour garantir un graphe connexe...
    for(int i = 0; i + 1 < nbSommets; i++) g.ajouterAreteNonOrientee(noms[i], noms[i + 1]);
    // ...puis quelques aretes supplementaires aleatoires.
    std::uniform_int_distribution<int> choix(0, nbSommets - 1);
    int extra = nbSommets / 2;
    for(int k = 0; k < extra; k++) {
        std::string a = noms[choix(rng)];
        std::string b = noms[choix(rng)];
        if(a != b) g.ajouterAreteNonOrientee(a, b);
    }
    return g;
}

} // namespace

int main() {
    sf::RenderWindow fenetre(sf::VideoMode({LARGEUR_FENETRE, HAUTEUR_FENETRE}),
                              "Visualiseur de structures - AVL / Rouge-Noir / BST");
    fenetre.setFramerateLimit(60);
    fenetre.setPosition({0, 0});

    // Police embarquee avec le projet (DejaVu Sans, licence libre) en
    // premier choix : ca evite de dependre d'un chemin systeme qui
    // n'existe pas forcement (les chemins Linux ci-dessous n'existent
    // pas sur macOS, par exemple). On ne retombe sur des polices
    // systeme que si le fichier embarque est introuvable pour une
    // raison ou une autre.
    sf::Font police;
    bool policeChargee = police.openFromFile("DejaVuSans.ttf");
    if(!policeChargee) {
        const char* cheminsSysteme[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",        // Linux (Debian/Ubuntu)
            "/usr/share/fonts/dejavu/DejaVuSans.ttf",                 // Linux (Fedora)
            "/System/Library/Fonts/Supplemental/Arial.ttf",           // macOS
            "/System/Library/Fonts/Helvetica.ttc",                    // macOS
        };
        for(const char* chemin : cheminsSysteme) {
            if(police.openFromFile(chemin)) { policeChargee = true; break; }
        }
    }

    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<int> sequence = nouvelleSequence(rng);
    std::size_t indiceSuivant = 0;

    ArbreAVL<int> avl;
    ArbreRougeNoir<int> rn;
    ArbreBST<int> bst;

    GrapheTexte graphe = genererGrapheAleatoire(rng, 12);
    Vue vue = Vue::Arbres;

    auto inserer = [&](int valeur) {
        avl.inserer(valeur);
        rn.inserer(valeur);
        bst.inserer(valeur);
    };

    auto recommencer = [&]() {
        avl.vider();
        rn.effacer();
        bst.effacer();
        sequence = nouvelleSequence(rng);
        indiceSuivant = 0;
    };

    while(fenetre.isOpen()) {
        while(const std::optional evt = fenetre.pollEvent()) {
            if(evt->is<sf::Event::Closed>()) fenetre.close();

            if(const auto* touche = evt->getIf<sf::Event::KeyPressed>()) {
                if(touche->code == sf::Keyboard::Key::Escape) fenetre.close();
                if(touche->code == sf::Keyboard::Key::G)
                    vue = (vue == Vue::Arbres) ? Vue::Graphe : Vue::Arbres;
                if(touche->code == sf::Keyboard::Key::R) recommencer();
                if((touche->code == sf::Keyboard::Key::Space || touche->code == sf::Keyboard::Key::Right) &&
                   indiceSuivant < sequence.size()) {
                    inserer(sequence[indiceSuivant]);
                    indiceSuivant++;
                }
                if(touche->code == sf::Keyboard::Key::F) {
                    while(indiceSuivant < sequence.size()) {
                        inserer(sequence[indiceSuivant]);
                        indiceSuivant++;
                    }
                }
            }

            if(const auto* clic = evt->getIf<sf::Event::MouseButtonPressed>()) {
                if(clic->button == sf::Mouse::Button::Left && indiceSuivant < sequence.size()) {
                    inserer(sequence[indiceSuivant]);
                    indiceSuivant++;
                }
            }
        }

        fenetre.clear(COULEUR_FOND);

        // --- Bandeau d'instructions, commun aux deux vues ---
        dessinerTexte(fenetre, police,
                      "Espace/clic = inserer suivant   F = tout inserer   R = nouvelle sequence   "
                      "G = basculer vue   Echap = quitter",
                      20, 12, 14, sf::Color(170, 175, 185));

        if(vue == Vue::Arbres) {
            std::ostringstream progression;
            progression << "Sequence : " << indiceSuivant << " / " << sequence.size() << " valeurs inserees";
            dessinerTexte(fenetre, police, progression.str(), 20, 36, 15, COULEUR_TEXTE);

            float largeurColonne = LARGEUR_FENETRE / 3.f;
            float y0 = 150.f;
            float zoneHauteur = HAUTEUR_FENETRE - y0 - 30.f;

            int hMax = std::max({avl.hauteur(), rn.hauteur(), bst.hauteur(), 1});
            float espacement = std::min(70.f, zoneHauteur / (float)(hMax + 1));

            struct Colonne {
                std::string titre;
                std::string stats;
                DessinArbre<int> dessin;
                sf::Color couleur;
                bool rouge;
            };

            std::ostringstream statsAvl, statsRn, statsBst;
            statsAvl << "hauteur=" << avl.hauteur() << "  rotations=" << avl.nombreRotations();
            statsRn << "hauteur=" << rn.hauteur() << "  rotations=" << rn.nombreRotations()
                    << "  recolorations=" << rn.nombreRecolorations();
            statsBst << "hauteur=" << bst.hauteur() << "  rotations=0 (non equilibre)";

            Colonne colonnes[3] = {
                {"AVL", statsAvl.str(), avl.exporterPourDessin(), COULEUR_AVL, false},
                {"Rouge-Noir", statsRn.str(), rn.exporterPourDessin(), COULEUR_RN_NOIR, true},
                {"BST simple", statsBst.str(), bst.exporterPourDessin(), COULEUR_BST, false},
            };

            for(int i = 0; i < 3; i++) {
                float x0 = i * largeurColonne;
                dessinerTexte(fenetre, police, colonnes[i].titre, x0 + 20, 70, 20, COULEUR_TEXTE);
                dessinerTexte(fenetre, police, colonnes[i].stats, x0 + 20, 100, 13, sf::Color(190, 195, 200));
                if(i > 0) {
                    sf::RectangleShape separateur({1.f, HAUTEUR_FENETRE - y0});
                    separateur.setPosition({x0, y0});
                    separateur.setFillColor(sf::Color(60, 62, 70));
                    fenetre.draw(separateur);
                }
                dessinerArbre(fenetre, police, colonnes[i].dessin, x0, largeurColonne, y0, espacement,
                              colonnes[i].couleur, colonnes[i].rouge);
            }
        } else {
            dessinerTexte(fenetre, police, "Graphe aleatoire : points d'articulation en orange", 20, 36, 15,
                          COULEUR_TEXTE);

            sf::Vector2f centre(LARGEUR_FENETRE / 2.f, HAUTEUR_FENETRE / 2.f + 20.f);
            auto positions = layoutCirculaire(graphe, centre, 320.f);
            auto critiques = pointsArticulation(graphe);

            // Aretes.
            for(const auto& paire : graphe.sommets) {
                sf::Vector2f p1 = positions[paire.first];
                for(const auto& voisin : paire.second.voisins) {
                    // Chaque arete non orientee apparait deux fois (A->B et B->A) :
                    // on ne la dessine qu'une fois en comparant les cles.
                    if(voisin < paire.first) continue;
                    sf::Vector2f p2 = positions[voisin];
                    sf::Vertex ligne[] = {sf::Vertex{p1, COULEUR_ARETE}, sf::Vertex{p2, COULEUR_ARETE}};
                    fenetre.draw(ligne, 2, sf::PrimitiveType::Lines);
                }
            }

            // Sommets.
            for(const auto& paire : graphe.sommets) {
                sf::Vector2f p = positions[paire.first];
                sf::CircleShape cercle(RAYON_NOEUD + 4.f);
                cercle.setOrigin({RAYON_NOEUD + 4.f, RAYON_NOEUD + 4.f});
                cercle.setPosition(p);
                cercle.setFillColor(critiques.count(paire.first) ? COULEUR_ARTICULATION : COULEUR_SOMMET);
                cercle.setOutlineColor(sf::Color(15, 15, 18));
                cercle.setOutlineThickness(1.5f);
                fenetre.draw(cercle);

                sf::Text texte(police, paire.first, 13);
                texte.setFillColor(sf::Color::White);
                sf::FloatRect bornes = texte.getLocalBounds();
                texte.setOrigin({bornes.position.x + bornes.size.x / 2.f, bornes.position.y + bornes.size.y / 2.f});
                texte.setPosition(p);
                fenetre.draw(texte);
            }

            std::ostringstream info;
            info << critiques.size() << " point(s) d'articulation sur " << graphe.sommets.size() << " sommets";
            dessinerTexte(fenetre, police, info.str(), 20, HAUTEUR_FENETRE - 36, 14, sf::Color(190, 195, 200));
        }

        if(!policeChargee) {
            dessinerTexte(fenetre, police, "(police systeme introuvable : le texte peut ne pas s'afficher)", 20,
                          HAUTEUR_FENETRE - 60, 12, sf::Color(220, 120, 90));
        }

        fenetre.display();
    }

    return 0;
}

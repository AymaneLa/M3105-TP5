#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
    if (instruction != nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
    int valeur = m_expression->executer(); // On exécute (évalue) l'expression
    ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
    return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
    int og, od, valeur;
    if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
    if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (this->m_operateur == "+") valeur = (og + od);
    else if (this->m_operateur == "-") valeur = (og - od);
    else if (this->m_operateur == "*") valeur = (og * od);
    else if (this->m_operateur == "==") valeur = (og == od);
    else if (this->m_operateur == "!=") valeur = (og != od);
    else if (this->m_operateur == "<") valeur = (og < od);
    else if (this->m_operateur == ">") valeur = (og > od);
    else if (this->m_operateur == "<=") valeur = (og <= od);
    else if (this->m_operateur == ">=") valeur = (og >= od);
    else if (this->m_operateur == "et") valeur = (og && od);
    else if (this->m_operateur == "ou") valeur = (og || od);
    else if (this->m_operateur == "non") valeur = (!og);
    else if (this->m_operateur == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
    if (m_condition->executer()) m_sequence->executer();
    return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstTantQue::NoeudInstTantQue(Noeud* cond, Noeud* seq)
: m_condition(cond), m_sequence(seq) {
}

int NoeudInstTantQue::executer() {
    while (m_condition -> executer()) {
        m_sequence -> executer();
    }
    return 0;
}

NoeudInstSiRiche::NoeudInstSiRiche(vector<Noeud*> conditions, vector<Noeud*> sequences)
: m_conditions(conditions), m_sequences(sequences) {
}

int NoeudInstSiRiche::executer() {
    for (unsigned int i = 0; i < m_conditions.size(); i++) {
        if (i == m_conditions.size() - 1) {
            m_sequences[i] -> executer();
        } else {
            if (m_conditions[i] -> executer()) {
                m_sequences[i] -> executer();
            }
        }
    }
    return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstRepeter::NoeudInstRepeter(Noeud* cond, Noeud* seq)
: m_condition(cond), m_sequence(seq) {
}

int NoeudInstRepeter::executer() {
    do {
        m_sequence -> executer();
    } while (m_condition->executer());
    return 0; // La valeur renvoyée ne représente rien !
}

NoeudInstPour::NoeudInstPour(Noeud* cond, Noeud* seq, Noeud* affect, Noeud* affect2)
: m_condition(cond), m_sequence(seq), m_affectation(affect), m_affectation2(affect2) {
}

int NoeudInstPour::executer() {
    for (m_affectation->executer(); m_condition->executer(); m_affectation2->executer()) {
        m_sequence->executer(); // on exécute chaque instruction de la séquence     
    }
    return 0;
}

NoeudInstEcrire::NoeudInstEcrire(vector<Noeud*> cond, vector<Noeud*> para)
: m_conditions(cond), m_parametres(para) {
}

int NoeudInstEcrire::executer() {
    int i = 0;
    while(m_conditions[i] -> executer()) {
        cout << m_conditions[i];
    }
    return 0;
}

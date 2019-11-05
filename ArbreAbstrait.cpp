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

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        m_instructions[i]->traduitEnCPP(cout, indentation); // on exécute chaque instruction de la séquence
        cout << endl; // La valeur renvoyée ne représente rien !
    }
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

void NoeudAffectation::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    m_variable->traduitEnCPP(cout, indentation);
    cout << " = ";
    m_expression->traduitEnCPP(cout, 0);
    cout << ";";
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

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    m_operandeGauche->traduitEnCPP(cout, 0);
    cout << " " << m_operateur.getChaine() << " ";
    m_operandeDroit->traduitEnCPP(cout, 0);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
    if (m_condition->executer()) {
        m_sequence->executer();

    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    cout << setw(4 * indentation) << " " << "if (";
    m_condition -> traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence -> traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << " " << "{" << endl;
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

void NoeudInstTantQue::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "while (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "}";
}

NoeudInstSiRiche::NoeudInstSiRiche(vector<Noeud*> conditions, vector<Noeud*> sequences)
: m_conditions(conditions), m_sequences(sequences) {
}

int NoeudInstSiRiche::executer() {
    bool exit = false;
    for (int i = 0; i < m_conditions.size() - 1 && !exit; i++) {
        if (m_conditions.at(i)->executer()) {
            m_sequences.at(i)->executer();
            exit = true;
        }
    }

    if (m_conditions.size() > 0 && !exit) {
        m_sequences.at(m_sequences.size() - 1)->executer();
    }

    return 0;
}

void NoeudInstSiRiche::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    unsigned int i = 1;
    cout << setw(4 * indentation) << "" << "if (";
    m_conditions.at(0)->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequences.at(0)->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "}";

    while (i < m_conditions.size() - 1) {
        cout << "else if (";
        m_conditions.at(i)->traduitEnCPP(cout, 0);
        cout << ") {" << endl;
        m_sequences.at(i)->traduitEnCPP(cout, indentation + 1);
        cout << setw(4 * indentation) << "" << "}";
        i = i + 1;

    }

    if (m_conditions.size() > 0) {
        cout << "else {" << endl;
        m_sequences.at(m_sequences.size() - 1)->traduitEnCPP(cout, indentation + 1);
        cout << setw(4 * indentation) << "" << "}";
    }
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

void NoeudInstRepeter::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "do {" << endl;
    m_sequence->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "} while (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ");";
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

void NoeudInstPour::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    if (m_affectation != nullptr) {
        m_affectation->traduitEnCPP(cout, indentation);
    } else if (m_affectation2 != nullptr) {
        m_affectation2->traduitEnCPP(cout, indentation);
    }
    cout << "\n" << setw(4 * indentation) << "" << "for (";
    m_affectation->traduitEnCPP(cout, 0);
    m_condition->traduitEnCPP(cout, 0);
    cout << ";";
    m_affectation2->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * indentation) << "" << "}";
}

NoeudInstEcrire::NoeudInstEcrire(vector<Noeud*> para)
: m_parametres(para) {
}

int NoeudInstEcrire::executer() {
    for (int i = 0; i < m_parametres.size(); i++) {
        if (typeid (*m_parametres[i]) == typeid (SymboleValue) && *((SymboleValue*) m_parametres[i]) == "<CHAINE>") {
            string s = ((SymboleValue*) m_parametres[i])->getChaine();
            cout << s.substr(1, s.size() - 2);
        } else {
            cout << m_parametres[i] -> executer();

        }
    }
    return 0;
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    unsigned int i = 0;
    while (i < m_parametres.size()) {
        cout << " cout << ";
        m_parametres.at(i)->traduitEnCPP(cout, 0);
        i++;
    }
    cout << setw(4 * indentation) << " << endl;";
}

NoeudInstLire::NoeudInstLire(vector<Noeud*> var)
: m_variables(var) {
}

int NoeudInstLire::executer() {
    for (int i = 0; i < m_variables.size(); i++) {
        int b;
        cin >> b;
        ((SymboleValue*) m_variables[i])-> setValeur(b);
    }
    return 0;
}

void NoeudInstLire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    unsigned int i = 0;
    while (i < m_variables.size()) {
        cout << "cin >> ";
        m_variables.at(i)->traduitEnCPP(cout, 0);
        i++;
    }
    cout << ";" << endl;
}

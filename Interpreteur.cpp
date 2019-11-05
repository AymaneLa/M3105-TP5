#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr), m_nbErreur(0) {

}

void Interpreteur::analyse() {
    m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    static char messageWhat[256];
    if (m_lecteur.getSymbole() != symboleAttendu) {
        sprintf(messageWhat,
                "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
                m_lecteur.getLigne(), m_lecteur.getColonne(),
                symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
        throw SyntaxeException(messageWhat);
    }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    tester(symboleAttendu);
    m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    cout << setw(4 * indentation) << "" << "int main() {" << endl;
    for (int i = 0; i < m_table.getTaille(); i++) {
        if (m_table[i] == "<VARIABLE>") {
            cout << setw(4 * (indentation + 1)) << "" << "int " << m_table[i].getChaine() << ";" << endl;
        }
    }
    getArbre() -> traduitEnCPP(cout, indentation + 1);
    cout << setw(4 * (indentation + 1)) << "" << "return 0;" << endl;
    cout << setw(4 * indentation) << "}" << endl;
}

Noeud* Interpreteur::programme() {
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    try {
        testerEtAvancer("procedure");
        testerEtAvancer("principale");
        testerEtAvancer("(");
        testerEtAvancer(")");
        Noeud* sequence = seqInst();
        testerEtAvancer("finproc");
        tester("<FINDEFICHIER>");
        return sequence;
    } catch (SyntaxeException const& e) {
        cout << e.what() << endl;
        while (m_lecteur.getSymbole() != "<FINDEFICHIER>") {
            m_lecteur.avancer();
        }
        m_nbErreur++;
    }
    return 0;
}

Noeud* Interpreteur::seqInst() {
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    do {
        sequence->ajoute(inst());
    } while (m_lecteur.getSymbole() == "<VARIABLE>" ||
            m_lecteur.getSymbole() == "si" ||
            m_lecteur.getSymbole() == "tantque" ||
            m_lecteur.getSymbole() == "repeter" ||
            m_lecteur.getSymbole() == "pour" ||
            m_lecteur.getSymbole() == "lire" ||
            m_lecteur.getSymbole() == "ecrire");
    // Tant que le symbole courant est un début possible d'instruction...
    // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
    return sequence;
}

Noeud* Interpreteur::inst() {
    // <inst> ::= <affectation>  ; | <instSiRiche> | <instTantQue> | <instRepeter> | <instPour> | <instEcrire> | <instLire>
    try {
        if (m_lecteur.getSymbole() == "<VARIABLE>") {
            Noeud *affect = affectation();
            testerEtAvancer(";");
            return affect;
        } else if (m_lecteur.getSymbole() == "si")
            return instSiRiche();
            // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
        else if (m_lecteur.getSymbole() == "tantque")
            return instTantQue();
        else if (m_lecteur.getSymbole() == "repeter")
            return instRepeter();
        else if (m_lecteur.getSymbole() == "pour")
            return instPour();
        else if (m_lecteur.getSymbole() == "ecrire")
            return instEcrire();
        else if (m_lecteur.getSymbole() == "lire")
            return instLire();
        else {
            erreur("Instruction incorrecte");
        }
    } catch (SyntaxeException const& e) {
        cout << e.what() << endl;
        while ((m_lecteur.getSymbole() != "si" && m_lecteur.getSymbole() != "tantque" && m_lecteur.getSymbole() != "repeter" &&
                m_lecteur.getSymbole() != "pour" && m_lecteur.getSymbole() != "ecrire" &&
                m_lecteur.getSymbole() != "lire") && m_lecteur.getSymbole() != "<FINDEFICHIER>") {
            m_lecteur.avancer();
        }
        m_nbErreur++;
    }
    return 0;
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression> 
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
    m_lecteur.avancer();
    testerEtAvancer("=");
    Noeud* exp = expression(); // On mémorise l'expression trouvée
    return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
    // <expression> ::= <expEt> {ou <expEt> }
    //  <opBinaire> ::= ou
    Noeud* expE = expEt();
    while (m_lecteur.getSymbole() == "ou") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* expEDroit = expEt(); // On mémorise l'opérande droit
        expE = new NoeudOperateurBinaire(operateur, expE, expEDroit); // Et on construuit un noeud opérateur binaire
    }
    return expE; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::expMult() {
    // <expression> ::= <facteur> { * | / <facteur>}
    //  <opBinaire> ::= *  | / 
    Noeud* fact = facteur();
    while (m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur(); // On mémorise l'opérande droit
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
    }
    return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::expAdd() {
    // <expAdd> ::= <expMult> {+|-<expMult> }
    //  <opBinaire> ::=  + | - 
    Noeud* expM = expMult();
    while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* expMDroit = expMult(); // On mémorise l'opérande droit
        expM = new NoeudOperateurBinaire(operateur, expM, expMDroit); // Et on construuit un noeud opérateur binaire
    }
    return expM; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::expComp() {
    // <expComp> ::= <expAdd> {==|!=|<|<=|>|>= <expAdd> }
    //  <opBinaire> ::=  == | != | < | <= | > | >= 
    Noeud* expA = expAdd();
    while (m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" || m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" || m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* expADroit = expAdd(); // On mémorise l'opérande droit
        expA = new NoeudOperateurBinaire(operateur, expA, expADroit); // Et on construuit un noeud opérateur binaire
    }
    return expA; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::expEt() {
    // <expEt> ::= <expComp> {et <expComp> }
    //  <opBinaire> ::=  et 
    Noeud* expC = expComp();
    while (m_lecteur.getSymbole() == "et") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* expCDroit = expComp(); // On mémorise l'opérande droit
        expC = new NoeudOperateurBinaire(operateur, expC, expCDroit); // Et on construuit un noeud opérateur binaire
    }
    return expC; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {
    // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expression();
        testerEtAvancer(")");
    } else
        erreur("Facteur incorrect");
    return fact;
}

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    testerEtAvancer("finsi");
    return new NoeudInstSi(condition, sequence); // Et on renvoie un noeud Instruction Si
}

Noeud* Interpreteur::instTantQue() {
    // <instTantQue> ::=tantque( <expression> ) <seqInst> fintantque
    testerEtAvancer("tantque");
    testerEtAvancer("(");
    Noeud* condition = expression();
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("fintantque");
    return new NoeudInstTantQue(condition, sequence);
}

Noeud* Interpreteur::instSiRiche() {
    // <instSiRiche> ::=si(<expression>) <seqInst> {sinonsi(<expression>) <seqInst> }[sinon <seqInst>]finsi
    vector<Noeud*> conditions;
    vector<Noeud*> sequences;
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression();
    conditions.push_back(condition);
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    sequences.push_back(sequence);
    while (m_lecteur.getSymbole() == "sinonsi") {
        m_lecteur.avancer();
        testerEtAvancer("(");
        Noeud* condition = expression();
        conditions.push_back(condition);
        testerEtAvancer(")");
        Noeud* sequence = seqInst();
        sequences.push_back(sequence);
    }

    if (m_lecteur.getSymbole() == "sinon") {
        Noeud* condition;
        conditions.push_back(condition);
        m_lecteur.avancer();
        Noeud* sequence = seqInst();
        sequences.push_back(sequence);
    }
    testerEtAvancer("finsi");
    return new NoeudInstSiRiche(conditions, sequences);
}

Noeud* Interpreteur::instRepeter() {
    testerEtAvancer("repeter");
    Noeud* sequence = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = expression();
    testerEtAvancer(")");
    return new NoeudInstRepeter(condition, sequence);
}

Noeud* Interpreteur::instPour() {
    Noeud* affect;
    Noeud* affect2;
    testerEtAvancer("pour");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        affect = affectation();
        testerEtAvancer(";");
    }
    Noeud* condition = expression();
    testerEtAvancer(";");
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        affect2 = affectation();
    }
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finpour");
    return new NoeudInstPour(condition, sequence, affect, affect2);
}

Noeud* Interpreteur::instEcrire() {
    vector<Noeud*> parametres;
    testerEtAvancer("ecrire");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole() == "<CHAINE>") {
        Noeud* p = m_table.chercheAjoute(m_lecteur.getSymbole());
        parametres.push_back(p);
        m_lecteur.avancer();
    } else {
        Noeud* exp = expression();
        parametres.push_back(exp);

    }

    while (m_lecteur.getSymbole() == ",") {
        m_lecteur.avancer();
        if (m_lecteur.getSymbole() == "<CHAINE>") {
            Noeud* p = m_table.chercheAjoute(m_lecteur.getSymbole());
            parametres.push_back(p);
            m_lecteur.avancer();
        } else {
            Noeud* exp = expression();
            parametres.push_back(exp);

        }
    }
    testerEtAvancer(")");
    return new NoeudInstEcrire(parametres);
}

Noeud* Interpreteur::instLire() {
    vector<Noeud*> variables;
    testerEtAvancer("lire");
    testerEtAvancer("(");

    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
    m_lecteur.avancer();
    variables.push_back(var);

    while (m_lecteur.getSymbole() == ",") {
        m_lecteur.avancer();
        tester("<VARIABLE>");
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
        m_lecteur.avancer();
        variables.push_back(var);
    }
    testerEtAvancer(")");
    return new NoeudInstLire(variables);
}
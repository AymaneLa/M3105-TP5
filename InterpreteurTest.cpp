/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   InterpreteurTest.cpp
 * Author: laraicha
 *
 * Created on Nov 4, 2019, 2:41:29 PM
 */

#include "InterpreteurTest.h"
#include "Interpreteur.h"


CPPUNIT_TEST_SUITE_REGISTRATION(InterpreteurTest);

InterpreteurTest::InterpreteurTest() {
}

InterpreteurTest::~InterpreteurTest() {
}

void InterpreteurTest::setUp() {
}

void InterpreteurTest::tearDown() {
}

void InterpreteurTest::testTantQue() {
    ifstream fichier("instTantQue.txt");
    Interpreteur interpreteur(fichier);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("TantQue fonctionnel", interpreteur.analyse());

    ifstream fichier2("instTantQueErreur.txt");
    Interpreteur interpreteur2(fichier2);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("TantQue non fonctionnel", interpreteur2.analyse(), interpreteur2.erreur("Erreur synthaxique"));
}




/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   InterpreteurTest.h
 * Author: laraicha
 *
 * Created on Nov 4, 2019, 2:41:28 PM
 */

#ifndef INTERPRETEURTEST_H
#define INTERPRETEURTEST_H

#include <cppunit/extensions/HelperMacros.h>

class InterpreteurTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(InterpreteurTest);

    CPPUNIT_TEST(testInterpreteur);
    CPPUNIT_TEST(testAnalyse);
    CPPUNIT_TEST(testTraduitEnCPP);

    CPPUNIT_TEST_SUITE_END();

public:
    InterpreteurTest();
    virtual ~InterpreteurTest();
    void setUp();
    void tearDown();
    void testTantQue();

private:
    void testInterpreteur();
    void testAnalyse();
    void testTraduitEnCPP();

};

#endif /* INTERPRETEURTEST_H */


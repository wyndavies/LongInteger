/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: wyn
 *
 * Created on 03 March 2017, 13:49
 */

#include <cstdlib>
#include <iostream>
#include "MyHardwareInfo.h"
#include "LongInteger.h"

using std::cout;
using std::endl;

/*
 * 
 */
int main(int argc, char** argv) {

    MyHardwareInfo mhi;
    
    cout << "Hello there" << endl;
    cout << "I can see " << mhi.GetLogicalCores() << " logical cores" << endl;
    cout << "and " << mhi.GetPhysicalCores() << " physical cores" << endl;
    
    LongInteger value1;
    LongInteger value2(10);
    LongInteger value3("10");
    LongInteger value4(value3);
    value1 = value2 * value3;
    cout << value1.toDecimal() << endl;
    value1 = value2 / value3;
    cout << value1.toDecimal() << endl;
    value1 = value2 + value3;
    cout << value1.toDecimal() << endl;
    value1 = value2 - value3;
    cout << value1.toDecimal() << endl;
    value1 = value2;
    value1.powerCalc(value3);
    cout << value1.toDecimal() << endl;

    value1 = 100000;
    cout << value1.toDecimal() << endl;
    string value = "100000000000000000000";
    value2 = value;
    cout << value2.toDecimal() << endl;
    value3 = value2 * value2;
    value3 *= value2;
    value3 *= value2;
    value3 *= value2;
    cout << value3.toDecimal() << endl;
    cout << value3.getSize() << endl;

    LongInteger::KARATSUBACUTOFF = 20;
    LongInteger::KARATSUBATHREADING = 50;
    for( int i = 0; i < 14; i++) {
        value3 *= value3;
//        cout << value3.toDecimal() << endl;
        cout << value3.getSize() << endl;
    }    
    return 0;
}

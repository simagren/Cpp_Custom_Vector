#pragma once
#include "TestLevel.h"
#include <string>
#include <limits.h>
#include <iostream>
#include <cassert>

#if DEL==1
namespace {
    //För DEL == 1 så är detta bara en char så testprogrammet fungerar

    struct Dhelper {
        char value;

        //För att kolla om det blev en const
        int Test() { return 1; }
        const int Test() const { return 2; }


        //~Dhelper() {
        //}
        Dhelper() {
            value = 0;
        }
        //Dhelper(bool) {
        //    assert(false);
        //}
        Dhelper(int v) {
            value = v;
        }
        Dhelper(const Dhelper& other) {
            value = other.value;
        }
        Dhelper(Dhelper&& other) noexcept {
            value = other.value;
        }

        Dhelper& operator=(char v) {
            value = v;
            return *this;
        }
        //Used
        Dhelper& operator=(const Dhelper& other) {
            value = other.value;
            return *this;
        }
        friend
            auto operator<=>(const Dhelper&, const Dhelper&) = default;

        explicit operator char() const { return value; }
        friend
            std::ostream& operator<<(std::ostream& cout, Dhelper d) {
                return cout << (char)d;
        }

    };
}

#elif DEL==2
namespace {
    //DEL's value control the behavior (is there a default constructor or not)
    //We assume that unconstructed memory has a content <=0.
    // throws otherwise
    // A Dhelper object has a char value and a status flag with values:
    enum {
        NON = 1, //Not initialized at all
        DD, // Has been destroyed
        MF, // Has been moved From
        DC, // Default constructed (onlypossible if DEL == 1)
        CV, // Constructed with value
        CC, // Copy Constructed
        MC, // Move Constructed
        CA, // Copy assigned
        MA, // Move assigned
    };

    struct Dhelper {
        static std::string usedConstr;
        static bool checkDhelper;
        char flag;
        char FLAG;
        char value;

        //För att kolla om det blev en const
        int Test() { return 1; }
        const int Test() const { return 2; }

        void IsConstr(bool checkDhelper) {
            assert(!checkDhelper || 1 < flag && flag < 7);
        }
        void IsNotConstr(bool checkDhelper)
        {
            assert(!checkDhelper || 1 == flag);
        }
        bool IsConstr() const {
            assert(FLAG >= MF);
            return true;
        }
        bool IsNotConstr() const {
            assert(FLAG <= MF);
            return true;
        }
        bool IsMoved() const {
            assert(FLAG == MF);
            return true;
        }

        ~Dhelper() {
            IsConstr();
            FLAG = DD;
        }
        Dhelper() {
            assert(FLAG <= DD); // Constructed twice!
            FLAG = CV;
            value = 0;
            usedConstr += "DC";
            IsNotConstr(checkDhelper);
            flag = 2;
        }
//#pragma warning(disable:26495)
        Dhelper(bool) {
            assert(false);
        }
        Dhelper(int v) {
            IsNotConstr();
            FLAG = CV;
            value = v;
            usedConstr += "CC";
            IsNotConstr(checkDhelper);
            flag = 3;
        }
        Dhelper(const Dhelper& other) {
            assert(FLAG <= DD); // Constructed twice!
            FLAG = CC;
            value = other.value;
            usedConstr += "CC";
            IsNotConstr(checkDhelper);
            flag = 4;
        }
        Dhelper(Dhelper&& other) noexcept {
            assert(IsNotConstr());
            assert(other.IsConstr());
            FLAG = MC;
            other.FLAG = MF;
            value = other.value;
        }

        Dhelper& operator=(char v) {
            assert(FLAG > DD); // not constructed!
            FLAG = CA;

            value = v;
            usedConstr += "CA";
            IsConstr(true);
            flag = 6;
            return *this;
        }

        Dhelper& operator=(const Dhelper& other) {
            assert(FLAG > DD); // not constructed!
            FLAG = CA;
            value = other.value;
            usedConstr += "CA";
            IsConstr(true);
            flag = 6;
            return *this;
        }

        friend
            auto operator<=>(const Dhelper&, const Dhelper&) = default;

        friend bool operator==(const Dhelper& lhs, char rhs) {
            assert(lhs.FLAG > DD); // Not constructed!
            return lhs.value == rhs;
        }
        friend bool operator!=(const Dhelper& lhs, const Dhelper& rhs) {
            assert(lhs.FLAG > DD && rhs.FLAG > DD); // Not constructed!
            auto x = lhs.value != rhs.value;
            return x;
        }

        explicit operator char() const { return value; }
        friend
            std::ostream& operator<<(std::ostream& cout, Dhelper d) {
            return cout << (char)d;
        }

    };

    std::string Dhelper::usedConstr{};
    bool Dhelper::checkDhelper{};

}

#endif
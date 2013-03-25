/*
 * main.cpp
 *
 *  Created on: 30.04.2011
 *      Author: igor
 */

#include "config.h"
#include "gettext.h"
#define _(str) gettext(str)
#include "mapi.h"
#include <iostream>
#include <locale>
#include <string>
#include <cassert>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <boost/thread/thread.hpp>

/*
 * Функция для тестирования - изменяет mapi большое число раз
 */
void
change(mapi<std::string, int>& m, volatile bool *work)
try
{
    std::ostringstream ost;
    for (int i = 0; i < 1000; ++i)
    {
        for (int j = 0; j < 1000; ++j)
        {
            ost << "test" << j;
            assert(m.insert(make_pair(ost.str(), j)).second);
            ost.str("");
        }
        for (int j = 0; j < 1000; ++j)
        {
            ost << "test" << j;
            assert(m.erase(ost.str()) == 1);
            ost.str("");
        }
    }
    *work = false;
}
catch (const std::exception& e)
{
    std::cerr << _("An exception occurred: ") << e.what() << std::endl;
    exit(EXIT_FAILURE);
}
catch (...)
{
    std::cerr << _("An unknown exception\n");
    exit(EXIT_FAILURE);
}
/*
 * Функция для тестирования - проводит в цикле проверку объекта
 */
void
check(mapi<std::string, int>& m, volatile bool *work, volatile int *count)
try
{
    while (*work)
    {
        assert(m.validate());
        ++*count;
    }
}
catch (const std::exception& e)
{
    std::cerr << _("An exception occurred: ") << e.what() << std::endl;
    exit(EXIT_FAILURE);
}
catch (...)
{
    std::cerr << _("An unknown exception\n");
    exit(EXIT_FAILURE);
}

/*
 * Тестирование класса mapi
 */
int
main(int argc, char *argv[])
try
{
    std::locale::global(std::locale(""));
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    std::cout << _("Test 1 Creating empty object: ");
    mapi<std::string, int> a;
    assert(a.empty());
    assert(a.validate());
    std::cout << _("OK\n");
    std::cout << _("Test 2 Creating object from a range of iterators: ");
    std::vector<std::pair<std::string, int> > vec;
    vec.push_back(std::make_pair("test1", 1));
    vec.push_back(std::make_pair("test2", 2));
    vec.push_back(std::make_pair("test2", 3)); // не должно вставится
    mapi<std::string, int> b(vec.begin(), vec.end());
    assert(b.size() == 2);
    assert(b.validate());
    assert(b.findv(1).size() == 1);
    assert(b.findv(2).size() == 1);
    assert(b.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 3 Creating object from mapi: ");
    mapi<std::string, int> c(b);
    assert(c.size() == 2);
    assert(c.validate());
    assert(c.findv(1).size() == 1);
    assert(c.findv(2).size() == 1);
    assert(c.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 4 Creating object from map: ");
    std::map<std::string, int> m(vec.begin(), vec.end());
    mapi<std::string, int> d(m);
    assert(d.size() == 2);
    assert(d.validate());
    assert(d.findv(1).size() == 1);
    assert(d.findv(2).size() == 1);
    assert(d.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 5 Assigning object mapi: ");
    d.clear();
    d = c;
    assert(d.size() == 2);
    assert(d.validate());
    assert(d.findv(1).size() == 1);
    assert(d.findv(2).size() == 1);
    assert(d.findv(3).size() == 0);
    std:: cout << _("OK\n");
    std::cout << _("Test 6 Assigning object map: ");
    d.clear();
    d = m;
    assert(d.size() == 2);
    assert(d.validate());
    assert(d.findv(1).size() == 1);
    assert(d.findv(2).size() == 1);
    assert(d.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 7 Creating const object: ");
    const mapi<std::string, int> e(vec.begin(), vec.end());
    assert(e.size() == 2);
    assert(e.validate());
    assert(e.findv(1).size() == 1);
    assert(e.findv(2).size() == 1);
    assert(e.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 8 Inserting value: ");
    assert(a.insert(std::make_pair("test1", 1)).second);
    assert(a.insert(std::make_pair("test2", 2)).second);
    assert(a.insert(std::make_pair("test2", 3)).second == false);
    assert(a.size() == 2);
    assert(a.validate());
    assert(a.findv(1).size() == 1);
    assert(a.findv(2).size() == 1);
    assert(a.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 9 Inserting value with hint position: ");
    a.clear();
    a.insert(a.begin(), std::make_pair("test1", 1));
    a.insert(a.begin(), std::make_pair("test2", 2));
    a.insert(a.begin(), std::make_pair("test2", 3)); // не должно вставится
    assert(a.size() == 2);
    assert(a.validate());
    assert(a.findv(1).size() == 1);
    assert(a.findv(2).size() == 1);
    assert(a.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 10 Inserting from a range of iterators: ");
    a.clear();
    a.insert(vec.begin(), vec.end());
    assert(a.size() == 2);
    assert(a.validate());
    assert(a.findv(1).size() == 1);
    assert(a.findv(2).size() == 1);
    assert(a.findv(3).size() == 0);
    std::cout << _("OK\n");
    std::cout << _("Test 11 Deleting by value: ");
    assert(a.erase("test1") == 1);
    assert(a.erase("test3") == 0);
    assert(a.size() == 1);
    assert(a.validate());
    assert(a.find("test1") == a.end());
    assert(a.find("test2") != a.end());
    std::cout << _("OK\n");
    std::cout << _("Test 12 Deleting by iterator: ");
    a.insert(std::make_pair("test1", 1));
    a.insert(std::make_pair("test3", 3));
    a.insert(std::make_pair("test4", 4));
    a.erase(a.find("test2"));
    a.erase(a.find("test3"));
    assert(a.size() == 2);
    assert(a.validate());
    assert(a.find("test1") != a.end());
    assert(a.find("test2") == a.end());
    assert(a.find("test3") == a.end());
    assert(a.find("test4") != a.end());
    std::cout << _("OK\n");
    std::cout << _("Test 13 Deleting by range of iterators: ");
    a.insert(std::make_pair("test2", 2));
    a.insert(std::make_pair("test3", 3));
    a.erase(a.find("test2"), a.find("test4"));
    assert(a.size() == 2);
    assert(a.validate());
    assert(a.find("test1") != a.end());
    assert(a.find("test2") == a.end());
    assert(a.find("test3") == a.end());
    assert(a.find("test4") != a.end());
    std::cout << _("OK\n");
    std::cout << _("Test 14 Find by value: ");
    std::vector<mapi<std::string, int>::iterator> vit = a.findv(1);
    assert(a.validate());
    assert(vit.size() == 1);
    assert(vit[0]->first == "test1");
    assert(vit[0]->second == 1);
    a["test1"] = 44;
    assert(a.validate());
    a["test9"] = 9;
    assert(a.validate());
    vit = a.findv(9);
    assert(vit.size() == 1);
    assert(vit[0]->first == "test9");
    assert(vit[0]->second == 9);
    assert(a.erase("test9") == 1);
    a["test11"];
    assert(a.validate());
    vit = a.findv(0);
    assert(vit.size() == 1);
    assert(vit[0]->first == "test11");
    assert(vit[0]->second == 0);
    assert(a.erase("test11") == 1);
    assert(a.validate());
    vit = a.findv(44);
    assert(vit.size() == 1);
    assert(vit[0]->first == "test1");
    assert(vit[0]->second == 44);
    mapi<std::string, int>::iterator i = a.find("test4");
    assert(i != a.end());
    i->second = 44;
    assert(a.validate());
    vit = a.findv(44);
    assert(vit.size() == 2);
    if( vit[0]->first == "test1" )
        assert(vit[1]->first == "test4");
    else
    {
        assert(vit[0]->first == "test4");
        assert(vit[1]->first == "test1");
    }
    assert(vit[0]->second == 44);
    assert(vit[1]->second == 44);
    a.insert(std::make_pair("test2", 2));
    a.insert(std::make_pair("test3", 3));
    assert(a.size() == 4);
    i = a.find("test3");
    (*i).second = 44;
    assert(a.validate());
    vit = a.findv(44);
    assert(vit.size() == 3);
    if( vit[0]->first == "test1" )
    {
        if( vit[1]->first == "test3" )
        {
            assert(vit[2]->first == "test4");
        }
        else
        {
            assert(vit[1]->first == "test4");
            assert(vit[2]->first == "test3");
        }
    }
    else if( vit[0]->first == "test3" )
    {
        if( vit[1]->first == "test4" )
        {
            assert(vit[2]->first == "test1");
        }
        else
        {
            assert(vit[1]->first == "test1");
            assert(vit[2]->first == "test4");
        }
    }
    else
    {
        if( vit[1]->first == "test1" )
        {
            assert(vit[2]->first == "test3");
        }
        else
        {
            assert(vit[1]->first == "test3");
            assert(vit[2]->first == "test1");
        }
    }
    assert(vit[0]->second == 44);
    assert(vit[1]->second == 44);
    assert(vit[2]->second == 44);
    vit = a.findv(99);
    assert(vit.empty());
    std::cout << _("OK\n");
    std::cout << _("Test 15 Check thread safety: ");
    a.clear();
    volatile bool work = true;
    volatile int count = 0;
    boost::thread thrd1(check, a, &work, &count);
    boost::thread thrd2(change, a, &work);
    thrd1.join();
    thrd2.join();
    assert(a.validate());
    assert(a.empty());
    std::cout << _("tested ") << count << _(" times OK\n");
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << _("An exception occurred: ") << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << _("An unknown exception\n");
    return EXIT_FAILURE;
}

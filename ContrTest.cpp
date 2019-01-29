#include <vector>
#include <iostream>

using namespace std;
class A
{
public:
	A(){cout << "A is Construct" << endl;}
	~A(){cout << "A is Destruct" << endl;}
};

class B
{
public:
	B(){cout << "B is Construct" << endl;}
	~B(){cout << "B is Destruct" << endl;}
};

class C : public A
{
public:
	C(){cout << "C is Construct" << endl;}
	~C(){cout << "C is Destruct" << endl;}
	void Test(void){cout << "C Test" << endl;};

	B member;
};

int main()
{
	C* c = new C;
	delete c;
	cout << "==================" <<endl;
	A* c1 = new A;
	((C*)c1)->Test();
	delete c1;
	return 0;
}
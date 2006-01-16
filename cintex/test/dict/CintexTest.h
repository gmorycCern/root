#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <cmath>

void free_function(void) { std::cout << "free function called" << std::endl; }
int another_free_function(int i ) { 
  std::cout << "another free function called" << std::endl;
  return i;
}
std::string another_free_function(const std::string& s ) { 
  std::cout << "another free complex function called" << std::endl;
  return s;
}
std::pair<int,int> operator + ( const std::pair<int,int>& p1, const std::pair<int,int>& p2 ) { 
  return std::make_pair(p1.first+p2.first,p1.second+p2.second);
}

class UnknownType;

typedef double Double_t;
typedef int  Int_t;
struct MyInt { int i; MyInt() : i(0){} };   
typedef MyInt MyInt_t;

namespace A {
namespace B {
namespace C {

void free_function(void) { std::cout << "free function called" << std::endl; }
int another_free_function(int i ) { 
  std::cout << "another free function called" << std::endl;
  return i;
}

struct Primitives {
  Primitives() : m_b(false), 
    m_c(99), m_s(99), m_i(99), m_l(99),
    m_uc(99), m_us(99), m_ui(99), m_ul(99),
    m_f(float(9.9)), m_d(9.9), m_str("99")
  {}

  bool m_b;
  char m_c;
  short m_s;
  int m_i;
  long m_l;
  unsigned char m_uc;
  unsigned short m_us;
  unsigned int m_ui;
  unsigned long m_ul;
  long long m_ll;
  long long m_ull;
  float m_f;
  double m_d;
  std::string m_str;

  bool b() { return m_b;}
  char c() { return m_c;}
  short s() { return m_s;}
  int i() { return m_i;}
  long l() { return m_l;}
  long long ll() { return m_ll; }
  unsigned char uc() { return m_uc;}
  unsigned short us() { return m_us;}
  unsigned int ui() { return m_ui;}
  unsigned long ul() { return m_ul;}
  unsigned long long ull() { return m_ull; }
  float f() { return m_f;}
  double d() { return m_d;}
  std::string str() { return m_str;}
  const char* ccstr() { return m_str.c_str();}
  char* cstr() { return const_cast<char*>(m_str.c_str());}

  void set_b( bool a)  { m_b = a;}
  void set_c( char a)  { m_c = a;}
  void set_s( short a) { m_s = a;}
  void set_i( const int a) { m_i = a;}
  void set_l( long a) { m_l = a;}
  void set_ll( long long a) { m_ll = a;}
  void set_uc( unsigned char a)  { m_uc = a;}
  void set_us( unsigned short a) { m_us = a;}
  void set_ui( unsigned int a) { m_ui = a;}
  void set_ul( unsigned long a) { m_ul = a;}
  void set_ull( unsigned long long a) { m_ull = a;}
  void set_f( float a) { m_f = a;}
  void set_d( double a) { m_d = a;}
  void set_str( const std::string& a) { m_str = a;}
  void set_cstr( const char* a) { m_str = std::string(a);}
  void set_all( bool b, char c, short s, int i, long l, float f, double d, const std::string& str) {
    m_b = b; m_c = c; m_s = s; m_i = i; m_l = l; m_f = f; m_d = d; m_str = str;
  }
};

enum Answer { no=0, yes, maybe };

class MyClass {
private:
  class InnerClass {
    public:
    InnerClass() {}
  };
  static int s_instances;
public:
  MyClass();
  MyClass(const MyClass& );
  virtual ~MyClass();
  MyClass& operator=(MyClass&);
  int doSomething(const std::string& );
  int magic() { return m_magic; }
  void setMagic(int v) { m_magic = v; }
  static int instances() { return s_instances; }
  Answer answer() { return m_answer; }
  void setAnswer(Answer a) { m_answer = a; } 
private:
  Answer m_answer;
  int m_magic;
};

int MyClass::s_instances = 0;

struct Abstract {
  virtual double vf() = 0;
};
struct Concrete : public Abstract {
  virtual double vf() { return 666.666; }
};

typedef int (*FuncPtr)(int);

class Calling {
public: 
  Calling() : m_ptr(&m_object) {}
  ~Calling() {}
  MyClass  retByValue() { return m_object; }
  MyClass* retByPointer() { return &m_object; }
  MyClass& retByReference() { return m_object; }
  MyClass*& retByRefPointer() { return m_ptr; }
  void* retByVoidPointer() { return &m_object; }
  UnknownType* retUnkownTypePointer() { return (UnknownType*)0x12345678; }
  UnknownType& retUnkownTypeReference() { return *(UnknownType*)0x12345678; }
  std::string retStrByValue() { return std::string("value");}
  std::string& retStrByRef() { static std::string s("reference");return s;}
  const std::string& retStrByConstRef() { static std::string s("const reference");return s;}    
  char* retCStr() { static char* s = const_cast<char*>("pointer"); return s;}
  const char* retConstCStr() { static const char* s="const pointer";return s;}    
  void setByValue( MyClass m ) { m_object = m; m.setMagic(999999); }
  void setByPointer( MyClass* m ) { 
    if (m) m_object = *m, m->setMagic(999999);
    else   m_object.setMagic(0);  }
  void setByReference( MyClass& m ) { m_object = m;  m.setMagic(999999); }
  void setByRefPointer( MyClass*& m ) { m_object = *m; m->setMagic(999999); }
  void setByRefPointerAbstract( Abstract*& m ) { static Concrete c; m = &c; }
  void setByVoidPointer( void* m) { m_object = *(MyClass*)m; ((MyClass*)m)->setMagic(999999);}
  long setByUnknownTypePointer(UnknownType* t) { return (long)t; }
  long setByUnknownTypeReference(UnknownType& t) { return (long)&t; }
  long setByUnknownConstTypePointer(const UnknownType* t) { return (long)t; }
  long setByUnknownConstTypeReference(const UnknownType& t) { return (long)&t; }
  int overloaded( int ) { return 1; }
  int overloaded( float ) { return 2; }
  int overloaded( int, float ) { return 3; }
  int overloaded( float, int ) { return 4; }
  int call( FuncPtr f, int v) { return f(v); }
private:
  MyClass m_object;
  MyClass* m_ptr;
};

MyClass::MyClass() : m_magic( 987654321 ){
  s_instances++;
}

MyClass::MyClass(const MyClass& c) {
  m_magic = c.m_magic;
  s_instances++;
}
MyClass::~MyClass() {
  s_instances--;
}
MyClass& MyClass::operator=(MyClass& c) {
  m_magic = c.m_magic;
  return *this;
}

int MyClass::doSomething(const std::string& something) {
  return something.size();
}

class Base1 {
public:
  int base1;
  virtual int v_getBase() {return base1; }
  virtual int v_get1() { return base1; }
  int getBase() { return base1; }
};
class Base2 {
public:
  int base2;
  virtual int v_getBase() { return base2; }
  virtual int v_get2() { return base2; }
  int getBase() { return base2; }
};
class Derived : public Base1, public Base2 {
 public:
  int a;
  double f;
  virtual int v_getBase() { return a; }
  int getBase() { return a; }
};

template <class T, class V> 
class Template {
public:
  struct Nested { 
    T first;
  };
  int doSomething(const std::string& something ) { return m_o.doSomething(something);}
  V&  getV() { return m_v; }
private:
  T m_o;
  V m_v;
};

class Virtual {
public:
  virtual double vf() = 0;
};

class IntermediateA : public virtual MyClass, public virtual Virtual {
public:
  IntermediateA() : m_intA(0x1111) {}
  virtual double vf()  { return 999.999; }
private:
  int m_intA;
};
class IntermediateB : public virtual MyClass  {
public:
  IntermediateB() : m_intB(0x2222) {}
private:
  int m_intB;
};

class Diamond : public virtual IntermediateA, public virtual IntermediateB {
public:
  Diamond() {strcpy( m_name, "Diamond"); }
  ~Diamond() { }
private:
  char m_name[8];
};

class DiamondFactory {
public:
  Virtual* getVirtual() { 
    return &m_diamond; 
  }
  Diamond* getDiamond() { 
    return &m_diamond; 
  }
private:
  Diamond m_diamond;
};


class Number  {
public:
  Number() : m_int(0) {}
  Number(int i) : m_int(i) {}
  ~Number() {}
  int number() { return m_int; }
  Number operator+(const Number& n) const { return Number(m_int+n.m_int);} 
  Number operator-(const Number& n) const { return Number(m_int-n.m_int);} 
  Number operator*(const Number& n) const { return Number(m_int*n.m_int);} 
  Number operator/(const Number& n) const { return Number(m_int/n.m_int);} 
  Number operator%(const Number& n) const { return Number(m_int%n.m_int);} 
  Number& operator+=(const Number& n) { m_int += n.m_int; return *this;} 
  Number& operator-=(const Number& n) { m_int -= n.m_int; return *this;} 
  Number& operator*=(const Number& n) { m_int *= n.m_int; return *this;} 
  Number& operator/=(const Number& n) { m_int /= n.m_int; return *this;}
  Number& operator%=(const Number& n) { m_int %= n.m_int; return *this;}
  bool operator<(const Number& n) const { return m_int < n.m_int;}
  bool operator>(const Number& n) const { return m_int > n.m_int;}
  bool operator<=(const Number& n) const { return m_int <= n.m_int;}
  bool operator>=(const Number& n) const { return m_int >= n.m_int;}
  bool operator!=(const Number& n) const { return m_int != n.m_int;}
  bool operator==(const Number& n) const { return m_int == n.m_int;}
  Number operator&(const Number& n) const { return Number(m_int&n.m_int);} 
  Number operator|(const Number& n) const { return Number(m_int|n.m_int);} 
  Number operator^(const Number& n) const { return Number(m_int^n.m_int);} 
  Number& operator&=(const Number& n) { m_int &= n.m_int; return *this;} 
  Number& operator|=(const Number& n) { m_int |= n.m_int; return *this;} 
  Number& operator^=(const Number& n) { m_int ^= n.m_int; return *this;} 
  Number operator<<(int i) const { return Number(m_int<<i);} 
  Number operator>>(int i) const { return Number(m_int>>i);} 
private:
  int m_int;
};

struct DataMembers {
  DataMembers() : i(0), f(0.0), p_myclass(new MyClass) {}
  ~DataMembers() { delete p_myclass; }
  int   i;
  float f;
  double d;
  std::string s;
  MyClass  myclass;
  MyClass* p_myclass;
};

struct ExtDataMembers : virtual public DataMembers {
  ExtDataMembers(): DataMembers(), xi(99) {}
  int xi;
};

struct DefaultArguments {
  DefaultArguments(int i=1,double f=0.0) : m_i(i),m_f(f) { }
  float function( const std::string& s, double v = -1.0, int i = 999 ) {
    return (float)(v + s.size() + i);
  }
  char* f_char( char* c = "string value") { return c; }
  std::string f_string( std::string c = std::string("string value") ) { return c; }
  DefaultArguments f_defarg( DefaultArguments d = DefaultArguments(9, 9.9), bool = false ) { return d; }
  int i() { return m_i; }
  double f() { return m_f; }
  int m_i;
  double m_f;
};

struct TypedefArguments {
  TypedefArguments(Int_t i, Double_t f) : m_i(i),m_f(f) { }
  double function( const MyInt_t& s, Double_t v , Int_t i ) {
    return (double)s.i + v + i;
  }
  int i() { return m_i; }
  double f() { return m_f; }
  int m_i;
  double m_f;
};

}}}

template <class T, class V> class GlobalTemplate {
public:
  int doSomething(const std::string& something ) { return m_o.doSomething(something);}
  V&  getV() { return m_v; }
private:
  T m_o;
  V m_v;
};


class Key {
private:
  long m_key;
public:
  Key(long l) : m_key(l){}
  Key() : m_key(0){}
  Key(const Key& k) : m_key(k.m_key) {}
  bool operator < (const Key& k) const {return m_key < k.m_key;}
  bool operator == (const Key& k) const {return m_key == k.m_key;}
  long value() const { return m_key; }
};

//--explicit template instanciations should work but not in the current
//  version of gccxml. Forcing instanciation of static

// template class A::B::C::Template<A::B::C::MyClass,float>; 
// template struct std::pair<A::B::C::MyClass,float>;
// template class std::vector<float>;
// template class std::vector<int>;
struct _Instantiations {
  std::vector<float> Vf;
  std::vector<int> Vi;
  std::vector<unsigned int> Vui;
  std::vector<long> Vl;
  std::list<int> Li;
  std::list<int>::iterator ILi;
  std::list<A::B::C::MyClass*> Lm;
  std::list<A::B::C::MyClass*>::iterator ILm;
  std::map<int,std::string> Mis;
  std::map<int,std::string>::value_type Mvt;
  std::map<int,std::string>::iterator IMis;
  std::map<Key,std::string> Mks;
  std::map<Key,std::string>::value_type Mkvt;
  std::map<Key,std::string>::iterator IMks;
  A::B::C::Template<A::B::C::MyClass,float>  m_temp1;
  A::B::C::Template<A::B::C::MyClass,float>::Nested  m_temp3;
  GlobalTemplate<A::B::C::MyClass,float>  m_temp2;
};

//--Persistency tests
//
struct Data {
  int    i;
  long   l;
  char   c;
  short  s;
  float  f;
  double d;
  bool   b;
  long long ll;
  unsigned long long ull;
  int     len;
  double  arra[10];
  double* array; //[len]
  long   transient1; //Set transient by the selection file with transient keyword
  long   transient2; //! Set transient by the comment
  long   transient3; //Set transient by adding comment in selection file
  Data() : i(0), l(0), c(0), s(0), f(0.0), d(0.0), b(false), ll(0), ull(0), len(0), array(0),
           transient1(1), transient2(2),transient3(3) {
    //std::cout << "Data constructor called" << std::endl; 
  }
  Data(const Data& o) : i(o.i), l(o.l), c(o.c), s(o.s), f(o.f), d(o.d), b(o.b), ll(o.ll), ull(o.ull) {
    if( o.len ) { 
      array = new double[o.len];
      len = o.len;
      for (int j=0; j<o.len; j++) array[j] = o.array[j];
    } else {
      len = 0;
      array = 0;
    }
    //std::cout << "Data copy constructor called" << std::endl; 
  }
  ~Data() {
    delete array;
    //std::cout << "Data destructor called" << std::endl; 
  }
  void setPattern(int o = 0) {
    c   = 9 + o;
    s   = 999 + o;
    i   = 9999 + o;
    l   = 99999 + o;
    ll  = 999999 + o;
    ull = 9999999 + o;
    b = o % 2 == 0;
    f = 9.9F + o;
    d = 9.99 + o;
    array = new double[i];
  }  
  bool checkPattern(int o = 0) {
    return (c == 9 + o)  &&
           (s == 999 + o) &&
           (i == 9999 + o) &&
           (l == 99999 + o) &&
           (ll == 999999 + o) &&
           (ull == 9999999 + unsigned(o)) &&
           (b == (o % 2 == 0)) &&
           (std::fabs(f - (9.9F + o)) < 0.0001F ) &&
           (std::fabs(d - (9.99 + o)) < 0.000001 );
  }
};

struct OData {
  int   i2;
  float f2;
  char  c2;
  short s2;
  OData() : i2(0), f2(0), c2(0), s2(0) {}
};

struct MyData : public Data {
  int extra;
};

typedef Data Data_t;
typedef std::vector<Data*> Data_v;

struct MyString : public std::string {
  virtual ~MyString() {}
};

struct Aggregate : public Data {
  int  extra;
  Data  dval1;
  Data_t  dval2;
  Data_t* dptr;
  Data_v  dvec;
  int  narray;
  Data farray[3];
  Data* darray;
  //MyString str;
  Aggregate() : extra(0), dptr(0), narray(0), darray(0) {}
  ~Aggregate() { delete dptr; delete [] darray; }
  void setFArrayPattern() {
    for ( unsigned int i = 0; i < sizeof(farray)/sizeof(Data); i++) farray[i].setPattern(i);
  }
  bool checkFArrayPattern() {
    for ( unsigned int i = 0; i < sizeof(farray)/sizeof(Data); i++) 
      if ( ! farray[i].checkPattern(i) ) return false;
    return true;
  }
  void setDArrayPattern(int n) {
    narray = n;
    darray = new Data[n];
    for ( int i = 0; i < n; i++) darray[i].setPattern(i);
  }
  bool checkDArrayPattern() {
    for ( int i = 0; i < narray; i++) 
      if ( ! darray[i].checkPattern(i) ) return false;
    return true;
  }
};

typedef OData NewData;

struct Final : public NewData, public Aggregate, public std::string {
  Data dval;
};

namespace a {
  struct b {
    A::B::C::MyClass m;
  }; 
}
namespace c {
  struct d : public a::b {};
}
namespace simple {
  struct test : public c::d {
    int i;
    double b;
    a::b ab;    
  };
}

typedef Data MyOtherData;


class __void__;
class SpecialConstructor {
  public:
  SpecialConstructor(__void__&){}
  SpecialConstructor(int,float) {}
  ~SpecialConstructor() {}
};

//------GaudiPython Interfaces problem
struct Ibase {
  virtual void f() = 0;
};
struct Iderived : virtual public Ibase {
  virtual void g() = 0;
};
struct Rfoo : public Iderived {
  virtual void f() {}
  virtual void g() {}
};
Iderived* I_get() { return new Rfoo;  }
void I_set(Ibase*) { }

//------GaudiPython SimpleProperty<string> problem
template <class T> class Verifier {
public:
  Verifier() {}
  ~Verifier() {}
};

template <class T, class V=Verifier<T> > class SimpleProperty {
public: 
  SimpleProperty() {}
  SimpleProperty(const SimpleProperty& p) { m_value = p.m_value; }
  SimpleProperty( const T& v ) { m_value = v; } 
  ~SimpleProperty() {}
private:
  T m_value;   
};

//------GaudiPython vector<const Property*>
class Pbase {
public:
  Pbase() {}
  virtual int get() { return 0; }
};

class P : public Pbase {
public:
  P(int i) : m_i(i) {}
  virtual int get() { return m_i; }
private:
  int m_i;
}; 

struct Obj {
  Obj() : val(99000099) {}
  int val;
};


#include <iostream>
using namespace std;

void byValue(Obj a) { cout << a.val << endl; }
void byPointer(Obj* a) { cout << a->val << endl; }
void byConstPointer(const Obj* a) { cout << a->val << endl; }
void byReference(Obj& a) { cout << a.val << endl; }
void byConstReference(const Obj& a) { cout << a.val << endl; }
void byReferencePointer(Obj*& a) { cout << a->val << endl; }
void byConstReferencePointer(const Obj*& a) { cout << a->val << endl; }
void byConstReferenceConstPointer(const Obj *const& a) { cout << a->val << endl; }

namespace {
  struct Instances {
    SimpleProperty<std::string> s;
    std::vector<const Pbase*> v;
  };
}

//-----Enumeration test--------------------------------------------------------------
namespace A {
  enum E1 {
    one = 1,
    two = 2,
    hundred = 100
  };
}

enum E2 {
  E2_one = 1,
  E2_two = 2,
  E2_hundred = 100
};

//----Variable test------------------------------------------------------------------
int gMyInt = 123;

namespace A {
  A::B::C::MyClass* gMyPointer = 0;
}


#include <stdexcept>

class ExceptionGenerator {
  public:
    ExceptionGenerator( bool b ) { if (b) throw std::logic_error("My Exception in constructor"); }
    ~ExceptionGenerator() {}
    void doThrow( bool b ) {if (b) throw std::logic_error("My Exception in method"); }
};

#include <list>
struct __ins {
 std::list<ExceptionGenerator*> a;
};

namespace N {

  class WithProtectedTypes {
    protected:
    class ProtectedInner { int i; };
  };
  
  class WithPrivateTypes : public WithProtectedTypes {
    class Inner { int i; };
    class InnerWithInner {
      public:
      class PInner {};
      private:
      class Inner { int i;};
      Inner* i;
    };
    typedef Inner Inner_t;
    struct Inner_s { int i; };
    union  Inner_u { int i; float f; };
    enum   Inner_e { one, two };
    template <class T> class InnterT {int y;};

    Inner   i1;
    Inner*  i1p;
    Inner** i1pp;
    Inner_t   it1;
    Inner_t*  it1p;
    Inner_t** it1pp;
    std::vector<Inner> vi1;  
    std::vector<Inner*> vi1p;  
    std::vector<const Inner*> vci1p;
    std::vector<WithPrivateTypes*> vt;  
    std::vector<InnerWithInner*> vv;
    InnerWithInner::PInner ii;
    ProtectedInner  iii;
    ProtectedInner* iiip;
    Inner_s  s1;
    Inner_s* s1p;
    Inner_u  u1;
    Inner_u* u1p;
    Inner_e  e1;
    Inner_e* e1p;
    std::vector<InnterT<Inner*> *> vvvv;
  };
}
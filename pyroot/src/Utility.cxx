// @(#)root/pyroot:$Name:  $:$Id: Utility.cxx,v 1.11 2004/11/02 10:13:06 rdm Exp $
// Author: Wim Lavrijsen, Apr 2004

// Bindings
#include "PyROOT.h"
#include "Utility.h"
#include "ObjectHolder.h"

// ROOT
#include "TClassEdit.h"

// CINT
#include "Api.h"


//- data ------------------------------------------------------------------------
char* PyROOT::Utility::theObject_ = const_cast< char* >( "_theObject" );

PyObject* PyROOT::Utility::theObjectString_ =
   PyString_FromString( PyROOT::Utility::theObject_ );


//- public functions ------------------------------------------------------------
void PyROOT::Utility::addToClass(
      const char* label, PyCFunction cfunc, PyObject* cls, int flags ) {
   PyMethodDef* pdef = new PyMethodDef;
   pdef->ml_name  = const_cast< char* >( label );
   pdef->ml_meth  = cfunc;
   pdef->ml_flags = flags;
   pdef->ml_doc   = NULL;

   PyObject* func = PyCFunction_New( pdef, NULL );
   PyObject* method = PyMethod_New( func, NULL, cls );
   PyObject_SetAttrString( cls, pdef->ml_name, method );
   Py_DECREF( func );
   Py_DECREF( method );
}


PyROOT::ObjectHolder* PyROOT::Utility::getObjectHolder( PyObject* self ) {
   if ( ! self )
      return 0;

   PyObject* cobj = PyObject_GetAttr( self, theObjectString_ );
   if ( cobj != 0 ) {
      ObjectHolder* holder =
         reinterpret_cast< PyROOT::ObjectHolder* >( PyCObject_AsVoidPtr( cobj ) );
      Py_DECREF( cobj );
      return holder;
   }
   else
      PyErr_Clear();

   return 0;
}


void* PyROOT::Utility::getObjectFromHolderFromArgs( PyObject* argsTuple ) {
   PyObject* self = PyTuple_GET_ITEM( argsTuple, 0 );
   Py_INCREF( self );

   PyROOT::ObjectHolder* holder = getObjectHolder( self );
   Py_DECREF( self );

   if ( holder != 0 )
      return holder->getObject();
   return 0;
}


PyROOT::Utility::EDataType PyROOT::Utility::effectiveType( const std::string& typeName ) {
   EDataType effType = kOther;

   G__TypeInfo ti( typeName.c_str() );
   if ( ti.Property() & G__BIT_ISENUM )
      return EDataType( (int) kEnum );

   std::string shortName = TClassEdit::ShortType( ti.TrueName(), 1 );

   const int isp = isPointer( typeName );
   const int mask = isp == 1 ? kPtrMask : 0;

   if ( shortName == "bool" )
      effType = EDataType( (int) kBool | mask );
   else if ( shortName == "char" )
      effType = EDataType( (int) kChar | mask );
   else if ( shortName == "short" )
      effType = EDataType( (int) kShort | mask );
   else if ( shortName == "int" )
      effType = EDataType( (int) kInt | mask );
   else if ( shortName == "unsigned int" )
      effType = EDataType( (int) kUInt | mask );
   else if ( shortName == "long" )
      effType = EDataType( (int) kLong | mask );
   else if ( shortName == "unsigned long" )
      effType = EDataType( (int) kULong | mask );
   else if ( shortName == "long long" )
      effType = EDataType( (int) kLongLong | mask );
   else if ( shortName == "float" )
      effType = EDataType( (int) kFloat | mask );
   else if ( shortName == "double" )
      effType = EDataType( (int) kDouble | mask );
   else if ( shortName == "void" )
      effType = EDataType( (int) kVoid | mask );
   else if ( shortName == "string" && isp == 0 )
      effType = kSTLString;
   else
      effType = kOther;

   return effType;
}


int PyROOT::Utility::isPointer( const std::string& tn ) {
   int isp = 0;
   for ( std::string::const_reverse_iterator it = tn.rbegin(); it != tn.rend(); ++it ) {
      if ( *it == '*' ) {
         isp = 1;
         break;
      }
      else if ( *it == '&' ) {
         isp = 2;
         break;
      }
      else if ( isalnum( *it ) )
         break;
   }

   return isp;
}

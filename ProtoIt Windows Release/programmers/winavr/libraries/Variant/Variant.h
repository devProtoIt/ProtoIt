// GNU General Public License Usage
// --------------------------------
// This file may be used under the terms of the GNU General Public
// License version 3.0 as published by the Free Software Foundation
// and appearing in the file 'GNU-GPL.txt' included in the packaging
// of this file.  Please review the following information to ensure
// the GNU General Public License version 3.0 requirements will be
// met: http://www.gnu.org/copyleft/gpl.html.
//
// (C) 2014 Dirk Kruithof

#ifndef __VARIANT__
#define __VARIANT__

#include <Arduino.h>

class Variant
{
public:

  Variant();
  Variant( const Variant& value);
  Variant( const int value);
  Variant( const long value);
  Variant( const uint8_t value);
  Variant( const double value);
  Variant( const char* value);
  Variant( const String& value);

  ~Variant();

  Variant& operator= ( const int value);
  Variant& operator= ( const long value);
  Variant& operator= ( const uint8_t value);
  Variant& operator= ( const double value);
  Variant& operator= ( const char* value);
  Variant& operator= ( const String& value);

  operator int();
  operator long();
  operator uint8_t();
  operator double();
  
  String toString( int decimals = -1);
  int length();
  void replace( String id, Variant value);

  const Variant operator+ ( const Variant& value) const;
  const Variant operator- ( const Variant& value) const;
  const Variant operator* ( const Variant& value) const;
  const Variant operator/ ( const Variant& value) const;
  const Variant operator& ( const Variant& value) const;
  const Variant operator| ( const Variant& value) const;
  const Variant operator^ ( const Variant& value) const;
  const Variant operator~ () const;

  const Variant operator+= ( const Variant& value);
  const Variant operator-= ( const Variant& value);
  const Variant operator*= ( const Variant& value);
  const Variant operator/= ( const Variant& value);
  const Variant operator&= ( const Variant& value);
  const Variant operator|= ( const Variant& value);
  const Variant operator^= ( const Variant& value);

  const Variant operator++ (int);
  const Variant operator-- (int);

  const bool operator== ( const Variant& value) const;
  const bool operator!= ( const Variant& value) const;
  const bool operator> ( const Variant& value) const;
  const bool operator< ( const Variant& value) const;
  const bool operator>= ( const Variant& value) const;
  const bool operator<= ( const Variant& value) const;

protected:
  
  void setLong( const long i);
  void setReal( const double r);
  void setString( const char* s);
  void setString( const String& s);

  long		iVar;  // integer representation
  double	rVar;  // real representation
  String	sVar;  // string representation
};

typedef struct VariantElement {
	Variant			var;
	VariantElement*	prev;
	VariantElement*	next;
} VariantElement;

class VariantList
{
public:

  VariantList();
  ~VariantList();

  int append( Variant var);
  int insert( int i, Variant var);
  void remove( int i);
  Variant at( int i);
  int size();
  int count();

  protected:

  VariantElement*	begin;
  VariantElement*	end;
  int				sz;
};

Variant calculate( Variant calculation);

#endif

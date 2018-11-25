// GNU General Public License Usage
// --------------------------------
// This file may be used under the terms of the GNU General Public
// License version 3.0 as published by the Free Software Foundation
// and appearing in the file 'GNU-GPL.txt' included in the packaging
// of this file.  Please review the following information to ensure
// the GNU General Public License version 3.0 requirements will be
// met: http://www.gnu.org/copyleft/gpl.html.
//
// (C) 2018 Dirk Kruithof

#ifndef __VARIANT__
#define __VARIANT__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>	// c++ library, declares std::string
#include <string.h>	// c library, declares strncpy
#include <wiringPi.h>

#define uint8_t unsigned char

class String
{
public:

  String();
  String( const String& str);
  String( const int value);
  String( const long value);
  String( const uint8_t value);
  String( const double value);
  String( const std::string& str);
  String( const char* str);

  const String operator+ ( const String& str) const;
  const String operator+= ( const String& str);

  const bool operator== ( const String& str) const;
  const bool operator!= ( const String& str) const;
  const bool operator> ( const String& str) const;
  const bool operator< ( const String& str) const;
  const bool operator>= ( const String& str) const;
  const bool operator<= ( const String& str) const;

  const char operator[] ( const int ix) const;
  const char operator[] ( const uint8_t ix) const;

  int length();
  int indexOf( char chr);
  int indexOf( String& str);
  String substring( int offset, int count = -1);
  void replace( String& str1, String str2);
  String trim();
  void toCharArray( char* str, int size);

protected:
  std::string s;
};

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

Variant calculate( Variant value);

#endif

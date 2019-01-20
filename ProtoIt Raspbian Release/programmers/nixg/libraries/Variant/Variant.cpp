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

#include "Variant.h"

char* dtostrf( double val, signed char width, unsigned char prec, char* s)
{
  char tmp[31];
  sprintf( tmp, "%.*f", prec, val);
  for ( int i = 0; i < 30 && i < width; i++ )
    s[i] = tmp[i];
  return s;
}

String::String()
{
  s = "";
}

String::String( const String& str)
{
  s = str.s;
}

String::String( const int value)
{
  char str[10];
  sprintf( str, "%d", value);
  s = str;
}

String::String( const long value)
{
  char str[20];
  sprintf( str, "%d", value);
  s = str;
}

String::String( const uint8_t value)
{
  char str[10];
  sprintf( str, "%d", value);
  s = str;
}

String::String( const double value)
{
  char str[10];
  sprintf( str, "%f", value);
  s = str;
}

String::String( const std::string& str)
{
  s = str;
}

String::String( const char* str)
{
  s = str;
}

String::String( const char chr)
{
  s = chr;
}

const String String::operator+ ( const String& str) const
{
  return s + str.s;
}

const String String::operator+= ( const String& str)
{
  s += str.s;
  return s;
}

const bool String::operator== ( const String& str) const
{
  return s == str.s;
}

const bool String::operator!= ( const String& str) const
{
  return s != str.s;
}

const bool String::operator> ( const String& str) const
{
  return s > str.s;
}

const bool String::operator< ( const String& str) const
{
  return s < str.s;
}

const bool String::operator>= ( const String& str) const
{
  return s >= str.s;
}

const bool String::operator<= ( const String& str) const
{
  return s <= str.s;
}


const char String::operator[] ( const int ix) const
{
  return s[ix];
}

const char String::operator[] ( const uint8_t ix) const
{
  return s[ix];
}


int String::length()
{
  return s.length();
}

int String::indexOf( char chr)
{
  int i;
  for ( i = 0; i < s.length(); i++ )
    if ( s[i] == chr ) break;
  if ( i >= s.length() ) return -1;
  return i;
}

int String::indexOf( String& str)
{
  int i1, i2;
  for ( i1 = 0; i1 < s.length(); i1++ ) {
    for ( i2 = 0; i2 < str.length() && i1+i2 < s.length(); i2++ )
      if ( s[i1+i2] != str[i2] ) break;
    if ( i2 >= str.length() ) break;
  }
  if ( i1 >= s.length() ) return -1;
  return i1;
}

String String::substring( int offset, int count)
{
  if ( count < 0 )
    count = s.length() - offset;
  std::string str;
  for ( int i = offset; i < s.length() && i < offset + count; i++ )
    str += s[i];
  return String( str);
}

void String::replace( String& str1, String str2)
{
  String str( s);
  int il = str.indexOf( str1);
  if ( il < 0 ) return;
  int ir = il + str1.s.length();
  String left = str.substring( 0, il);
  String right = str.substring( ir, s.length() - ir);
  s = left.s + str2.s + right.s;
}

String String::trim()
{
  int i1, i2;
  for ( i1 = 0; i1 < s.length(); i1++ )
    if ( s[i1] > ' ' ) break;
  for ( i2 = s.length() - 1; i2 >= 0; i2-- )
    if ( s[i2] > ' ' ) break;
  String str = String( s).substring( i1, i2 - i1 + 1);
  s = str.s;
  return str;
}

void String::toCharArray( char* str, int size)
{
  for ( int i = 0; i < size; i++ )
    str[i] = s[i];
}

Variant::Variant()
{
  setString( "");
}

Variant::Variant( const Variant& value)
{
  iVar = value.iVar;
  rVar = value.rVar;
  sVar = value.sVar;
}

Variant::Variant( const int value)
{
  setLong( value);
}

Variant::Variant( const long value)
{
  setLong( value);
}

Variant::Variant( const uint8_t value)
{
  setLong( value);
}

Variant::Variant( const double value)
{
  setReal( value);
}

Variant::Variant( const char* value)
{
  setString( value);
}

Variant::Variant( const String& value)
{
  setString( value);
}

Variant::~Variant()
{
}

Variant& Variant::operator= ( const int value)
{
  setLong( value);
  return *this;
}

Variant& Variant::operator= ( const long value)
{
  setLong( value);
  return *this;
}

Variant& Variant::operator= ( const uint8_t value)
{
  setLong( value);
  return *this;
}

Variant& Variant::operator= ( const double value)
{
  setReal( value);
  return *this;
}

Variant& Variant::operator= ( const char* value)
{
  setString( value);
  return *this;
}

Variant& Variant::operator= ( const String& value)
{
  setString( value);
  return *this;
}

Variant::operator int()
{
  return (int) iVar;
}

Variant::operator long()
{
  return iVar;
}

Variant::operator uint8_t()
{
  return iVar;
}

Variant::operator double()
{
  return rVar;
}

String Variant::toString( int decimals)
{
	if ( decimals < 0 ) return sVar;
	int len, ix = sVar.indexOf( '.');
	if ( ix < 0 ) return sVar;
	if ( decimals ) decimals++;
	return sVar.substring( 0, ix + decimals);
}

int Variant::length()
{
	return sVar.length();
}

void Variant::replace( String id, Variant value)
{
	sVar.replace( id, value.toString());
	setString( sVar);
}

const Variant Variant::operator+ ( const Variant& value) const
{
  return Variant( rVar + value.rVar);
}

const Variant Variant::operator- ( const Variant& value) const
{
  return Variant( rVar - value.rVar);
}

const Variant Variant::operator* ( const Variant& value) const
{
  return Variant( rVar * value.rVar);
}

const Variant Variant::operator/ ( const Variant& value) const
{
  return Variant( rVar / value.rVar);
}

const Variant Variant::operator& ( const Variant& value) const
{
  return Variant( iVar & value.iVar);
}

const Variant Variant::operator| ( const Variant& value) const
{
  return Variant( iVar | value.iVar);
}

const Variant Variant::operator^ ( const Variant& value) const
{
  return Variant( iVar ^ value.iVar);
}

const Variant Variant::operator~ () const
{
  return Variant( ~iVar);
}

const Variant Variant::operator+= ( const Variant& value)
{
  setReal( rVar + value.rVar);
  return *this;
}

const Variant Variant::operator-= ( const Variant& value)
{
  setReal( rVar - value.rVar);
  return *this;
}

const Variant Variant::operator*= ( const Variant& value)
{
  setReal( rVar * value.rVar);
  return *this;
}

const Variant Variant::operator/= ( const Variant& value)
{
  setReal( rVar / value.rVar);
  return *this;
}

const Variant Variant::operator&= ( const Variant& value)
{
  setLong( iVar & value.iVar);
  return *this;
}

const Variant Variant::operator|= ( const Variant& value)
{
  setLong( iVar | value.iVar);
  return *this;
}

const Variant Variant::operator^= ( const Variant& value)
{
  setLong( iVar ^ value.iVar);
  return *this;
}

const Variant Variant::operator++ (int)
{
  setReal( rVar + 1);
  return *this;
}

const Variant Variant::operator-- (int)
{
  setReal( rVar - 1);
  return *this;
}

const bool Variant::operator== ( const Variant& value) const
{
  return (sVar == value.sVar);
}

const bool Variant::operator!= ( const Variant& value) const
{
  return (sVar != value.sVar);
}

const bool Variant::operator> ( const Variant& value) const
{
  return (rVar > value.rVar);
}

const bool Variant::operator< ( const Variant& value) const
{
  return (rVar < value.rVar);
}

const bool Variant::operator>= ( const Variant& value) const
{
  return (rVar >= value.rVar);
}

const bool Variant::operator<= ( const Variant& value) const
{
  return (rVar <= value.rVar);
}

void Variant::setLong( const long i)
{
  char s[sVar.length() + 1];
  rVar = i;
  iVar = i;
  sVar = String( i);
}

void Variant::Variant::setReal( const double r)
{
  char s[51];
  rVar = r;
  iVar = (r >= 0 ? r + 0.5 : r - 0.5);
  dtostrf( r, 50, 10, s);
  sVar = s;
  sVar.trim();
}

void Variant::setString( const char* s)
{
  char str[51];
  strncpy( str, s, 50);
  sVar = s;
  rVar = strtod( str, NULL);
  iVar = (rVar >= 0 ? rVar + 0.5 : rVar - 0.5);
}

void Variant::setString( const String& s)
{
  char str[51];
  String s1 = s;
  s1.toCharArray( str, 50);
  str[50] = '\0';
  sVar = s;
  rVar = atof( str);
  iVar = (rVar >= 0 ? rVar + 0.5 : rVar - 0.5);
}

bool isoperator( char ch)
{
	char s[7] = "&|*/+-";
	for ( int i = 0; i < 6; i++ )
		if ( s[i] == ch ) return true;
	return false;
}

String parseformat( String str)
{
	int i;
	char c;
	
	// remove all spaces
	str.trim();
	for ( i = str.length() - 1; i > 0; i-- )
		if ( str[i] == ' ' )
			str = str.substring( 0, i) + str.substring( i + 1);

	// replace minus operator by underscore and remove double operators
	c = '\0';
	for ( i = str.length() - 1; i > 0; i-- ) {
		if ( isoperator( str[i]) ) {
			if ( c ) {
				if ( c == '_' ) {
					if ( str[i] == '-' )
						str = str.substring( 0, i) + String( "_-") + str.substring( i + 2);
					else
						str = str.substring( 0, i + 1) + String( '-') + str.substring( i + 2);
				}
				else
					str = str.substring( 0, i) + str.substring( i + 1);
			}
			else {
				if ( str[i] == '-' && i > 0 && str[i - 1] != '(' ) {
					c = '_';
					str = str.substring( 0, i) + String( '_') + str.substring( i + 1);
				}
				else
					c = str[i];
			}
		}
		else
			c = '\0';
	}

	return str;
}

String parseOperators( String func, String str)
{
	// NB! operator- is represented by an underscore
	// and can be distinguished from the -sign therefore

	Variant vfirst, vlast;
	int first, oper, last;

	oper = last = first = 0;
	while ( oper < str.length() ) {
		if ( str[oper] == '&' || str[oper] == '|' || str[oper] == '*' || str[oper] == '/' || str[oper] == '+' || str[oper] == '_' ) {
			if ( str[oper] == func[0] || str[oper] == func[1] ) {
				for ( last = oper + 1; last < str.length();last++ )
					if ( str[last] == '&' || str[last] == '|' || str[last] == '*' || str[last] == '/' || str[last] == '+' || str[last] == '_' )
						break;
				vfirst = str.substring( first, oper);
				vlast  = str.substring( oper + 1, last);
				switch ( str[oper] ) {
					case '&' : vfirst = vfirst & vlast; break;
					case '|' : vfirst = vfirst | vlast; break;
					case '*' : vfirst = vfirst * vlast; break;
					case '/' : vfirst = vfirst / vlast; break;
					case '+' : vfirst = vfirst + vlast; break;
					case '_' : vfirst = vfirst - vlast; break;
				}
				str = str.substring( 0, first) + vfirst.toString() + str.substring( last);
				oper = last = first;
			}
			else
				first = oper + 1;
		}
		oper++;
	}
	return str;
}

double dtor = 3.1415926535897932384626433832795/180;

String parseBrackets( String str)
{
	int i, j, c;
	Variant v;
	String begin, val, end;

	// when there are brackets
	while ( (i = str.indexOf( '(')) > -1 ) {
		// find the closing bracket
		c = 1;
		for ( j = i + 1; j < str.length() && c > 0; j++ ) {
			if ( str[j] == '(' ) c++;
			if ( str[j] == ')' ) c--;
		}
		// value of 'j' is the position behind the closing bracket - thus can be str.length() also
		if ( j > str.length() )
			return ""; // closing brackets are missing

		begin = str.substring( 0, i);
		val = str.substring( i + 1, j - 1);
		end = str.substring( j);

		// val can have brackets too, so recurse
		val = parseBrackets( val);

		// if it is a function, calculate the outcome
		v = val;
		if ( i > 3 ) {
			if ( begin.substring( i - 4, i) == "asin" ) { v = asin( double( Variant( val))) / dtor; begin = begin.substring( 0, i - 4); }
			else
			if ( begin.substring( i - 4, i) == "acos" ) { v = acos( double( Variant( val))) / dtor; begin = begin.substring( 0, i - 4); }
			else
			if ( begin.substring( i - 4, i) == "atan" ) { v = atan( double( Variant( val))) / dtor; begin = begin.substring( 0, i - 4); }
			else
			if ( begin.substring( i - 4, i) == "sqrt" ) { v = sqrt( double( Variant( val))); begin = begin.substring( 0, i - 4); }
		}
		if ( i > 2 ) {
			if ( begin.substring( i - 3, i) == "sin" ) { v = sin( dtor * double( Variant( val))); begin = begin.substring( 0, i - 3); }
			else
			if ( begin.substring( i - 3, i) == "cos" ) { v = cos( dtor * double( Variant( val))); begin = begin.substring( 0, i - 3); }
			else
			if ( begin.substring( i - 3, i) == "tan" ) { v = tan( dtor * double( Variant( val))); begin = begin.substring( 0, i - 3); }
			else
			if ( begin.substring( i - 3, i) == "sqr" ) { v = Variant( val) * Variant( val); begin = begin.substring( 0, i - 3); }
		}

		// the resulting stringtoCharArray
		str = begin + v.toString() + end;
	}
	
	// str doesn't contain any brackets anymore
	str = parseOperators( "&|", str);
	str = parseOperators( "/*", str);
	str = parseOperators( "+_", str);

	return str;
}

Variant calculate( Variant calculation)
{
	String str = calculation.toString();
	str = parseformat( str);
	str = parseBrackets( str);
	return Variant( str);
}

VariantList::VariantList()
{
	begin = NULL;
	end = NULL;
	sz = 0;
}

VariantList::~VariantList()
{
	VariantElement *prv, *rem = end;
	while ( rem ) {
		prv = rem->prev;
		delete rem;
		rem = prv;
	}
}

int VariantList::append( Variant var)
{
	return insert( sz, var);
}

int VariantList::insert( int i, Variant var)
{
	VariantElement *ins, *nxt = NULL, *prv = begin;
	bool atend = (i == sz);
	if ( i < 0 || i > sz ) return -1;
	if ( i ) {
		i--; // split list before element at i
		while ( i > 0 ) { prv = prv->next; i--; }
	}
	else {
		prv = NULL;
		nxt = begin; // 'split' list at begin
	}
		
	ins = new VariantElement;
	if ( prv )
		nxt = prv->next;
	else
		begin = ins;
	ins->prev = prv;
	ins->next = nxt;
	ins->var = var;
	if ( prv ) prv->next = ins;
	if ( nxt ) nxt->prev = ins;
	if ( atend ) end = ins;
	sz++;

	return sz;
}

void VariantList::remove( int i)
{
	VariantElement *prv, *nxt, *rem = begin;
	if ( i < 0 || i >= sz ) return;
	while ( i > 0 ) { rem = rem->next; i--; }
	prv = rem->prev;
	nxt = rem->next;
	delete rem;
	sz--;
	if ( prv )
		prv->next = nxt;
	else
		begin = nxt;
	if ( nxt )
		nxt->prev = prv;
	else
		end = prv;
}

Variant VariantList::at( int i)
{
	VariantElement *cur = begin;
	if ( i < 0 || i >= sz ) return Variant( "");
	while ( i > 0 ) { cur = cur->next; i--; }
	return cur->var;
}

int VariantList::size()
{
	return sz;
}

int VariantList::count()
{
	return sz;
}

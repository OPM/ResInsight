//##################################################################################################
//
//  Lib: LibIo
//  
//  --------------------------------------------------------------------------------------------
//  Please see references below for copyright, usage rights and license for this file.
//##################################################################################################

// Doxygen conditional section to hide contents of the cvf_tinyXML namespace
/// \cond CVF_NEVER_INCLUDE

// Includes needed by TinyXML:
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>



// This namespace contains the entire TinyXML library. All .cpp and .h files from TinyXML are just 
namespace cvf_tinyXML
{

// tinyxml.h

/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

// START - Mod by Ceetron:
// Added here as suggested in readme.txt  - FV, 20-FEB-2007
#define TIXML_USE_STL
// END - Mod


#ifndef TINYXML_INCLUDED
#define TINYXML_INCLUDED

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

// Help out windows:
// #if defined( _DEBUG ) && !defined( DEBUG )
// #define DEBUG
// #endif

#ifdef TIXML_USE_STL
	#define TIXML_STRING		std::string
#else
	#include "tinystr.h"
	#define TIXML_STRING		TiXmlString
#endif

// Deprecated library function hell. Compilers want to use the
// new safe versions. This probably doesn't fully address the problem,
// but it gets closer. There are too many compilers for me to fully
// test. If you get compilation troubles, undefine TIXML_SAFE
#define TIXML_SAFE

#ifdef TIXML_SAFE
	#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
		// Microsoft visual studio, version 2005 and higher.
		#define TIXML_SNPRINTF _snprintf_s
		#define TIXML_SSCANF   sscanf_s
	#elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
		// Microsoft visual studio, version 6 and higher.
		//#pragma message( "Using _sn* functions." )
		#define TIXML_SNPRINTF _snprintf
		#define TIXML_SSCANF   sscanf
	#elif defined(__GNUC__) && (__GNUC__ >= 3 )
		// GCC version 3 and higher.s
		//#warning( "Using sn* functions." )
		#define TIXML_SNPRINTF snprintf
		#define TIXML_SSCANF   sscanf
	#else
		#define TIXML_SNPRINTF snprintf
		#define TIXML_SSCANF   sscanf
	#endif
#endif	

class TiXmlDocument;
class TiXmlElement;
class TiXmlComment;
class TiXmlUnknown;
class TiXmlAttribute;
class TiXmlText;
class TiXmlDeclaration;
class TiXmlParsingData;

const int TIXML_MAJOR_VERSION = 2;
const int TIXML_MINOR_VERSION = 6;
const int TIXML_PATCH_VERSION = 1;

/*	Internal structure for tracking location of items 
	in the XML file.
*/
struct TiXmlCursor
{
	TiXmlCursor()		{ Clear(); }
	void Clear()		{ row = col = -1; }

	int row;	// 0 based.
	int col;	// 0 based.
};


/**
	Implements the interface to the "Visitor pattern" (see the Accept() method.)
	If you call the Accept() method, it requires being passed a TiXmlVisitor
	class to handle callbacks. For nodes that contain other nodes (Document, Element)
	you will get called with a VisitEnter/VisitExit pair. Nodes that are always leaves
	are simply called with Visit().

	If you return 'true' from a Visit method, recursive parsing will continue. If you return
	false, <b>no children of this node or its sibilings</b> will be Visited.

	All flavors of Visit methods have a default implementation that returns 'true' (continue 
	visiting). You need to only override methods that are interesting to you.

	Generally Accept() is called on the TiXmlDocument, although all nodes suppert Visiting.

	You should never change the document from a callback.

	@sa TiXmlNode::Accept()
*/
class TiXmlVisitor
{
public:
	virtual ~TiXmlVisitor() {}

	/// Visit a document.
	virtual bool VisitEnter( const TiXmlDocument& /*doc*/ )			{ return true; }
	/// Visit a document.
	virtual bool VisitExit( const TiXmlDocument& /*doc*/ )			{ return true; }

	/// Visit an element.
	virtual bool VisitEnter( const TiXmlElement& /*element*/, const TiXmlAttribute* /*firstAttribute*/ )	{ return true; }
	/// Visit an element.
	virtual bool VisitExit( const TiXmlElement& /*element*/ )		{ return true; }

	/// Visit a declaration
	virtual bool Visit( const TiXmlDeclaration& /*declaration*/ )	{ return true; }
	/// Visit a text node
	virtual bool Visit( const TiXmlText& /*text*/ )					{ return true; }
	/// Visit a comment node
	virtual bool Visit( const TiXmlComment& /*comment*/ )			{ return true; }
	/// Visit an unknow node
	virtual bool Visit( const TiXmlUnknown& /*unknown*/ )			{ return true; }
};

// Only used by Attribute::Query functions
enum 
{ 
	TIXML_SUCCESS,
	TIXML_NO_ATTRIBUTE,
	TIXML_WRONG_TYPE
};


// Used by the parsing routines.
enum TiXmlEncoding
{
	TIXML_ENCODING_UNKNOWN,
	TIXML_ENCODING_UTF8,
	TIXML_ENCODING_LEGACY
};

const TiXmlEncoding TIXML_DEFAULT_ENCODING = TIXML_ENCODING_UNKNOWN;

/** TiXmlBase is a base class for every class in TinyXml.
	It does little except to establish that TinyXml classes
	can be printed and provide some utility functions.

	In XML, the document and elements can contain
	other elements and other types of nodes.

	@verbatim
	A Document can contain:	Element	(container or leaf)
							Comment (leaf)
							Unknown (leaf)
							Declaration( leaf )

	An Element can contain:	Element (container or leaf)
							Text	(leaf)
							Attributes (not on tree)
							Comment (leaf)
							Unknown (leaf)

	A Decleration contains: Attributes (not on tree)
	@endverbatim
*/
class TiXmlBase
{
	friend class TiXmlNode;
	friend class TiXmlElement;
	friend class TiXmlDocument;

public:
	TiXmlBase()	:	userData(0)		{}
	virtual ~TiXmlBase()			{}

	/**	All TinyXml classes can print themselves to a filestream
		or the string class (TiXmlString in non-STL mode, std::string
		in STL mode.) Either or both cfile and str can be null.
		
		This is a formatted print, and will insert 
		tabs and newlines.
		
		(For an unformatted stream, use the << operator.)
	*/
	virtual void Print( FILE* cfile, int depth ) const = 0;

	/**	The world does not agree on whether white space should be kept or
		not. In order to make everyone happy, these global, static functions
		are provided to set whether or not TinyXml will condense all white space
		into a single space or not. The default is to condense. Note changing this
		value is not thread safe.
	*/
	static void SetCondenseWhiteSpace( bool condense )		{ condenseWhiteSpace = condense; }

	/// Return the current white space setting.
	static bool IsWhiteSpaceCondensed()						{ return condenseWhiteSpace; }

	/** Return the position, in the original source file, of this node or attribute.
		The row and column are 1-based. (That is the first row and first column is
		1,1). If the returns values are 0 or less, then the parser does not have
		a row and column value.

		Generally, the row and column value will be set when the TiXmlDocument::Load(),
		TiXmlDocument::LoadFile(), or any TiXmlNode::Parse() is called. It will NOT be set
		when the DOM was created from operator>>.

		The values reflect the initial load. Once the DOM is modified programmatically
		(by adding or changing nodes and attributes) the new values will NOT update to
		reflect changes in the document.

		There is a minor performance cost to computing the row and column. Computation
		can be disabled if TiXmlDocument::SetTabSize() is called with 0 as the value.

		@sa TiXmlDocument::SetTabSize()
	*/
	int Row() const			{ return location.row + 1; }
	int Column() const		{ return location.col + 1; }	///< See Row()

	void  SetUserData( void* user )			{ userData = user; }	///< Set a pointer to arbitrary user data.
	void* GetUserData()						{ return userData; }	///< Get a pointer to arbitrary user data.
	const void* GetUserData() const 		{ return userData; }	///< Get a pointer to arbitrary user data.

	// Table that returs, for a given lead byte, the total number of bytes
	// in the UTF-8 sequence.
	static const int utf8ByteTable[256];

	virtual const char* Parse(	const char* p, 
								TiXmlParsingData* data, 
								TiXmlEncoding encoding /*= TIXML_ENCODING_UNKNOWN */ ) = 0;

	/** Expands entities in a string. Note this should not contian the tag's '<', '>', etc, 
		or they will be transformed into entities!
	*/
	static void EncodeString( const TIXML_STRING& str, TIXML_STRING* out );

	enum
	{
		TIXML_NO_ERROR = 0,
		TIXML_ERROR,
		TIXML_ERROR_OPENING_FILE,
		TIXML_ERROR_PARSING_ELEMENT,
		TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
		TIXML_ERROR_READING_ELEMENT_VALUE,
		TIXML_ERROR_READING_ATTRIBUTES,
		TIXML_ERROR_PARSING_EMPTY,
		TIXML_ERROR_READING_END_TAG,
		TIXML_ERROR_PARSING_UNKNOWN,
		TIXML_ERROR_PARSING_COMMENT,
		TIXML_ERROR_PARSING_DECLARATION,
		TIXML_ERROR_DOCUMENT_EMPTY,
		TIXML_ERROR_EMBEDDED_NULL,
		TIXML_ERROR_PARSING_CDATA,
		TIXML_ERROR_DOCUMENT_TOP_ONLY,

		TIXML_ERROR_STRING_COUNT
	};

protected:

	static const char* SkipWhiteSpace( const char*, TiXmlEncoding encoding );

	inline static bool IsWhiteSpace( char c )		
	{ 
		return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
	}
	inline static bool IsWhiteSpace( int c )
	{
		if ( c < 256 )
			return IsWhiteSpace( (char) c );
		return false;	// Again, only truly correct for English/Latin...but usually works.
	}

	#ifdef TIXML_USE_STL
	static bool	StreamWhiteSpace( std::istream * in, TIXML_STRING * tag );
	static bool StreamTo( std::istream * in, int character, TIXML_STRING * tag );
	#endif

	/*	Reads an XML name into the string provided. Returns
		a pointer just past the last character of the name,
		or 0 if the function has an error.
	*/
	static const char* ReadName( const char* p, TIXML_STRING* name, TiXmlEncoding encoding );

	/*	Reads text. Returns a pointer past the given end tag.
		Wickedly complex options, but it keeps the (sensitive) code in one place.
	*/
	static const char* ReadText(	const char* in,				// where to start
									TIXML_STRING* text,			// the string read
									bool ignoreWhiteSpace,		// whether to keep the white space
									const char* endTag,			// what ends this text
									bool ignoreCase,			// whether to ignore case in the end tag
									TiXmlEncoding encoding );	// the current encoding

	// If an entity has been found, transform it into a character.
	static const char* GetEntity( const char* in, char* value, int* length, TiXmlEncoding encoding );

	// Get a character, while interpreting entities.
	// The length can be from 0 to 4 bytes.
	inline static const char* GetChar( const char* p, char* _value, int* length, TiXmlEncoding encoding )
	{
		assert( p );
		if ( encoding == TIXML_ENCODING_UTF8 )
		{
			*length = utf8ByteTable[ *((const unsigned char*)p) ];
			assert( *length >= 0 && *length < 5 );
		}
		else
		{
			*length = 1;
		}

		if ( *length == 1 )
		{
			if ( *p == '&' )
				return GetEntity( p, _value, length, encoding );
			*_value = *p;
			return p+1;
		}
		else if ( *length )
		{
			//strncpy( _value, p, *length );	// lots of compilers don't like this function (unsafe),
												// and the null terminator isn't needed
			for( int i=0; p[i] && i<*length; ++i ) {
				_value[i] = p[i];
			}
			return p + (*length);
		}
		else
		{
			// Not valid text.
			return 0;
		}
	}

	// Return true if the next characters in the stream are any of the endTag sequences.
	// Ignore case only works for english, and should only be relied on when comparing
	// to English words: StringEqual( p, "version", true ) is fine.
	static bool StringEqual(	const char* p,
								const char* endTag,
								bool ignoreCase,
								TiXmlEncoding encoding );

	static const char* errorString[ TIXML_ERROR_STRING_COUNT ];

	TiXmlCursor location;

    /// Field containing a generic user pointer
	void*			userData;
	
	// None of these methods are reliable for any language except English.
	// Good for approximation, not great for accuracy.
	static int IsAlpha( unsigned char anyByte, TiXmlEncoding encoding );
	static int IsAlphaNum( unsigned char anyByte, TiXmlEncoding encoding );
	inline static int ToLower( int v, TiXmlEncoding encoding )
	{
		if ( encoding == TIXML_ENCODING_UTF8 )
		{
			if ( v < 128 ) return tolower( v );
			return v;
		}
		else
		{
			return tolower( v );
		}
	}
	static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );

private:
	TiXmlBase( const TiXmlBase& );				// not implemented.
	void operator=( const TiXmlBase& base );	// not allowed.

	struct Entity
	{
		const char*     str;
		unsigned int	strLength;
		char		    chr;
	};
	enum
	{
		NUM_ENTITY = 5,
		MAX_ENTITY_LENGTH = 6

	};
	static Entity entity[ NUM_ENTITY ];
	static bool condenseWhiteSpace;
};


/** The parent class for everything in the Document Object Model.
	(Except for attributes).
	Nodes have siblings, a parent, and children. A node can be
	in a document, or stand on its own. The type of a TiXmlNode
	can be queried, and it can be cast to its more defined type.
*/
class TiXmlNode : public TiXmlBase
{
	friend class TiXmlDocument;
	friend class TiXmlElement;

public:
	#ifdef TIXML_USE_STL	

	    /** An input stream operator, for every class. Tolerant of newlines and
		    formatting, but doesn't expect them.
	    */
	    friend std::istream& operator >> (std::istream& in, TiXmlNode& base);

	    /** An output stream operator, for every class. Note that this outputs
		    without any newlines or formatting, as opposed to Print(), which
		    includes tabs and new lines.

		    The operator<< and operator>> are not completely symmetric. Writing
		    a node to a stream is very well defined. You'll get a nice stream
		    of output, without any extra whitespace or newlines.
		    
		    But reading is not as well defined. (As it always is.) If you create
		    a TiXmlElement (for example) and read that from an input stream,
		    the text needs to define an element or junk will result. This is
		    true of all input streams, but it's worth keeping in mind.

		    A TiXmlDocument will read nodes until it reads a root element, and
			all the children of that root element.
	    */	
	    friend std::ostream& operator<< (std::ostream& out, const TiXmlNode& base);

		/// Appends the XML node or attribute to a std::string.
		friend std::string& operator<< (std::string& out, const TiXmlNode& base );

	#endif

	/** The types of XML nodes supported by TinyXml. (All the
			unsupported types are picked up by UNKNOWN.)
	*/
	enum NodeType
	{
		TINYXML_DOCUMENT,
		TINYXML_ELEMENT,
		TINYXML_COMMENT,
		TINYXML_UNKNOWN,
		TINYXML_TEXT,
		TINYXML_DECLARATION,
		TINYXML_TYPECOUNT
	};

	virtual ~TiXmlNode();

	/** The meaning of 'value' changes for the specific type of
		TiXmlNode.
		@verbatim
		Document:	filename of the xml file
		Element:	name of the element
		Comment:	the comment text
		Unknown:	the tag contents
		Text:		the text string
		@endverbatim

		The subclasses will wrap this function.
	*/
	const char *Value() const { return value.c_str (); }

    #ifdef TIXML_USE_STL
	/** Return Value() as a std::string. If you only use STL,
	    this is more efficient than calling Value().
		Only available in STL mode.
	*/
	const std::string& ValueStr() const { return value; }
	#endif

	const TIXML_STRING& ValueTStr() const { return value; }

	/** Changes the value of the node. Defined as:
		@verbatim
		Document:	filename of the xml file
		Element:	name of the element
		Comment:	the comment text
		Unknown:	the tag contents
		Text:		the text string
		@endverbatim
	*/
	void SetValue(const char * _value) { value = _value;}

    #ifdef TIXML_USE_STL
	/// STL std::string form.
	void SetValue( const std::string& _value )	{ value = _value; }
	#endif

	/// Delete all the children of this node. Does not affect 'this'.
	void Clear();

	/// One step up the DOM.
	TiXmlNode* Parent()							{ return parent; }
	const TiXmlNode* Parent() const				{ return parent; }

	const TiXmlNode* FirstChild()	const		{ return firstChild; }	///< The first child of this node. Will be null if there are no children.
	TiXmlNode* FirstChild()						{ return firstChild; }
	const TiXmlNode* FirstChild( const char * value ) const;			///< The first child of this node with the matching 'value'. Will be null if none found.
	/// The first child of this node with the matching 'value'. Will be null if none found.
	TiXmlNode* FirstChild( const char * _value ) {
		// Call through to the const version - safe since nothing is changed. Exiting syntax: cast this to a const (always safe)
		// call the method, cast the return back to non-const.
		return const_cast< TiXmlNode* > ((const_cast< const TiXmlNode* >(this))->FirstChild( _value ));
	}
	const TiXmlNode* LastChild() const	{ return lastChild; }		/// The last child of this node. Will be null if there are no children.
	TiXmlNode* LastChild()	{ return lastChild; }
	
	const TiXmlNode* LastChild( const char * value ) const;			/// The last child of this node matching 'value'. Will be null if there are no children.
	TiXmlNode* LastChild( const char * _value ) {
		return const_cast< TiXmlNode* > ((const_cast< const TiXmlNode* >(this))->LastChild( _value ));
	}

    #ifdef TIXML_USE_STL
	const TiXmlNode* FirstChild( const std::string& _value ) const	{	return FirstChild (_value.c_str ());	}	///< STL std::string form.
	TiXmlNode* FirstChild( const std::string& _value )				{	return FirstChild (_value.c_str ());	}	///< STL std::string form.
	const TiXmlNode* LastChild( const std::string& _value ) const	{	return LastChild (_value.c_str ());	}	///< STL std::string form.
	TiXmlNode* LastChild( const std::string& _value )				{	return LastChild (_value.c_str ());	}	///< STL std::string form.
	#endif

	/** An alternate way to walk the children of a node.
		One way to iterate over nodes is:
		@verbatim
			for( child = parent->FirstChild(); child; child = child->NextSibling() )
		@endverbatim

		IterateChildren does the same thing with the syntax:
		@verbatim
			child = 0;
			while( child = parent->IterateChildren( child ) )
		@endverbatim

		IterateChildren takes the previous child as input and finds
		the next one. If the previous child is null, it returns the
		first. IterateChildren will return null when done.
	*/
	const TiXmlNode* IterateChildren( const TiXmlNode* previous ) const;
	TiXmlNode* IterateChildren( const TiXmlNode* previous ) {
		return const_cast< TiXmlNode* >( (const_cast< const TiXmlNode* >(this))->IterateChildren( previous ) );
	}

	/// This flavor of IterateChildren searches for children with a particular 'value'
	const TiXmlNode* IterateChildren( const char * value, const TiXmlNode* previous ) const;
	TiXmlNode* IterateChildren( const char * _value, const TiXmlNode* previous ) {
		return const_cast< TiXmlNode* >( (const_cast< const TiXmlNode* >(this))->IterateChildren( _value, previous ) );
	}

    #ifdef TIXML_USE_STL
	const TiXmlNode* IterateChildren( const std::string& _value, const TiXmlNode* previous ) const	{	return IterateChildren (_value.c_str (), previous);	}	///< STL std::string form.
	TiXmlNode* IterateChildren( const std::string& _value, const TiXmlNode* previous ) {	return IterateChildren (_value.c_str (), previous);	}	///< STL std::string form.
	#endif

	/** Add a new node related to this. Adds a child past the LastChild.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	TiXmlNode* InsertEndChild( const TiXmlNode& addThis );


	/** Add a new node related to this. Adds a child past the LastChild.

		NOTE: the node to be added is passed by pointer, and will be
		henceforth owned (and deleted) by tinyXml. This method is efficient
		and avoids an extra copy, but should be used with care as it
		uses a different memory model than the other insert functions.

		@sa InsertEndChild
	*/
	TiXmlNode* LinkEndChild( TiXmlNode* addThis );

	/** Add a new node related to this. Adds a child before the specified child.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	TiXmlNode* InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis );

	/** Add a new node related to this. Adds a child after the specified child.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	TiXmlNode* InsertAfterChild(  TiXmlNode* afterThis, const TiXmlNode& addThis );

	/** Replace a child of this node.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	TiXmlNode* ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis );

	/// Delete a child of this node.
	bool RemoveChild( TiXmlNode* removeThis );

	/// Navigate to a sibling node.
	const TiXmlNode* PreviousSibling() const			{ return prev; }
	TiXmlNode* PreviousSibling()						{ return prev; }

	/// Navigate to a sibling node.
	const TiXmlNode* PreviousSibling( const char * ) const;
	TiXmlNode* PreviousSibling( const char *_prev ) {
		return const_cast< TiXmlNode* >( (const_cast< const TiXmlNode* >(this))->PreviousSibling( _prev ) );
	}

    #ifdef TIXML_USE_STL
	const TiXmlNode* PreviousSibling( const std::string& _value ) const	{	return PreviousSibling (_value.c_str ());	}	///< STL std::string form.
	TiXmlNode* PreviousSibling( const std::string& _value ) 			{	return PreviousSibling (_value.c_str ());	}	///< STL std::string form.
	const TiXmlNode* NextSibling( const std::string& _value) const		{	return NextSibling (_value.c_str ());	}	///< STL std::string form.
	TiXmlNode* NextSibling( const std::string& _value) 					{	return NextSibling (_value.c_str ());	}	///< STL std::string form.
	#endif

	/// Navigate to a sibling node.
	const TiXmlNode* NextSibling() const				{ return next; }
	TiXmlNode* NextSibling()							{ return next; }

	/// Navigate to a sibling node with the given 'value'.
	const TiXmlNode* NextSibling( const char * ) const;
	TiXmlNode* NextSibling( const char* _next ) {
		return const_cast< TiXmlNode* >( (const_cast< const TiXmlNode* >(this))->NextSibling( _next ) );
	}

	/** Convenience function to get through elements.
		Calls NextSibling and ToElement. Will skip all non-Element
		nodes. Returns 0 if there is not another element.
	*/
	const TiXmlElement* NextSiblingElement() const;
	TiXmlElement* NextSiblingElement() {
		return const_cast< TiXmlElement* >( (const_cast< const TiXmlNode* >(this))->NextSiblingElement() );
	}

	/** Convenience function to get through elements.
		Calls NextSibling and ToElement. Will skip all non-Element
		nodes. Returns 0 if there is not another element.
	*/
	const TiXmlElement* NextSiblingElement( const char * ) const;
	TiXmlElement* NextSiblingElement( const char *_next ) {
		return const_cast< TiXmlElement* >( (const_cast< const TiXmlNode* >(this))->NextSiblingElement( _next ) );
	}

    #ifdef TIXML_USE_STL
	const TiXmlElement* NextSiblingElement( const std::string& _value) const	{	return NextSiblingElement (_value.c_str ());	}	///< STL std::string form.
	TiXmlElement* NextSiblingElement( const std::string& _value)				{	return NextSiblingElement (_value.c_str ());	}	///< STL std::string form.
	#endif

	/// Convenience function to get through elements.
	const TiXmlElement* FirstChildElement()	const;
	TiXmlElement* FirstChildElement() {
		return const_cast< TiXmlElement* >( (const_cast< const TiXmlNode* >(this))->FirstChildElement() );
	}

	/// Convenience function to get through elements.
	const TiXmlElement* FirstChildElement( const char * _value ) const;
	TiXmlElement* FirstChildElement( const char * _value ) {
		return const_cast< TiXmlElement* >( (const_cast< const TiXmlNode* >(this))->FirstChildElement( _value ) );
	}

    #ifdef TIXML_USE_STL
	const TiXmlElement* FirstChildElement( const std::string& _value ) const	{	return FirstChildElement (_value.c_str ());	}	///< STL std::string form.
	TiXmlElement* FirstChildElement( const std::string& _value )				{	return FirstChildElement (_value.c_str ());	}	///< STL std::string form.
	#endif

	/** Query the type (as an enumerated value, above) of this node.
		The possible types are: DOCUMENT, ELEMENT, COMMENT,
								UNKNOWN, TEXT, and DECLARATION.
	*/
	int Type() const	{ return type; }

	/** Return a pointer to the Document this node lives in.
		Returns null if not in a document.
	*/
	const TiXmlDocument* GetDocument() const;
	TiXmlDocument* GetDocument() {
		return const_cast< TiXmlDocument* >( (const_cast< const TiXmlNode* >(this))->GetDocument() );
	}

	/// Returns true if this node has no children.
	bool NoChildren() const						{ return !firstChild; }

	virtual const TiXmlDocument*    ToDocument()    const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const TiXmlElement*     ToElement()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const TiXmlComment*     ToComment()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const TiXmlUnknown*     ToUnknown()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const TiXmlText*        ToText()        const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const TiXmlDeclaration* ToDeclaration() const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

	virtual TiXmlDocument*          ToDocument()    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual TiXmlElement*           ToElement()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual TiXmlComment*           ToComment()     { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual TiXmlUnknown*           ToUnknown()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual TiXmlText*	            ToText()        { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual TiXmlDeclaration*       ToDeclaration() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

	/** Create an exact duplicate of this node and return it. The memory must be deleted
		by the caller. 
	*/
	virtual TiXmlNode* Clone() const = 0;

	/** Accept a hierchical visit the nodes in the TinyXML DOM. Every node in the 
		XML tree will be conditionally visited and the host will be called back
		via the TiXmlVisitor interface.

		This is essentially a SAX interface for TinyXML. (Note however it doesn't re-parse
		the XML for the callbacks, so the performance of TinyXML is unchanged by using this
		interface versus any other.)

		The interface has been based on ideas from:

		- http://www.saxproject.org/
		- http://c2.com/cgi/wiki?HierarchicalVisitorPattern 

		Which are both good references for "visiting".

		An example of using Accept():
		@verbatim
		TiXmlPrinter printer;
		tinyxmlDoc.Accept( &printer );
		const char* xmlcstr = printer.CStr();
		@endverbatim
	*/
	virtual bool Accept( TiXmlVisitor* visitor ) const = 0;

protected:
	TiXmlNode( NodeType _type );

	// Copy to the allocated object. Shared functionality between Clone, Copy constructor,
	// and the assignment operator.
	void CopyTo( TiXmlNode* target ) const;

	#ifdef TIXML_USE_STL
	    // The real work of the input operator.
	virtual void StreamIn( std::istream* in, TIXML_STRING* tag ) = 0;
	#endif

	// Figure out what is at *p, and parse it. Returns null if it is not an xml node.
	TiXmlNode* Identify( const char* start, TiXmlEncoding encoding );

	TiXmlNode*		parent;
	NodeType		type;

	TiXmlNode*		firstChild;
	TiXmlNode*		lastChild;

	TIXML_STRING	value;

	TiXmlNode*		prev;
	TiXmlNode*		next;

private:
	TiXmlNode( const TiXmlNode& );				// not implemented.
	void operator=( const TiXmlNode& base );	// not allowed.
};


/** An attribute is a name-value pair. Elements have an arbitrary
	number of attributes, each with a unique name.

	@note The attributes are not TiXmlNodes, since they are not
		  part of the tinyXML document object model. There are other
		  suggested ways to look at this problem.
*/
class TiXmlAttribute : public TiXmlBase
{
	friend class TiXmlAttributeSet;

public:
	/// Construct an empty attribute.
	TiXmlAttribute() : TiXmlBase()
	{
		document = 0;
		prev = next = 0;
	}

	#ifdef TIXML_USE_STL
	/// std::string constructor.
	TiXmlAttribute( const std::string& _name, const std::string& _value )
	{
		name = _name;
		value = _value;
		document = 0;
		prev = next = 0;
	}
	#endif

	/// Construct an attribute with a name and value.
	TiXmlAttribute( const char * _name, const char * _value )
	{
		name = _name;
		value = _value;
		document = 0;
		prev = next = 0;
	}

	const char*		Name()  const		{ return name.c_str(); }		///< Return the name of this attribute.
	const char*		Value() const		{ return value.c_str(); }		///< Return the value of this attribute.
	#ifdef TIXML_USE_STL
	const std::string& ValueStr() const	{ return value; }				///< Return the value of this attribute.
	#endif
	int				IntValue() const;									///< Return the value of this attribute, converted to an integer.
	double			DoubleValue() const;								///< Return the value of this attribute, converted to a double.

	// Get the tinyxml string representation
	const TIXML_STRING& NameTStr() const { return name; }

	/** QueryIntValue examines the value string. It is an alternative to the
		IntValue() method with richer error checking.
		If the value is an integer, it is stored in 'value' and 
		the call returns TIXML_SUCCESS. If it is not
		an integer, it returns TIXML_WRONG_TYPE.

		A specialized but useful call. Note that for success it returns 0,
		which is the opposite of almost all other TinyXml calls.
	*/
	int QueryIntValue( int* _value ) const;
	/// QueryDoubleValue examines the value string. See QueryIntValue().
	int QueryDoubleValue( double* _value ) const;

	void SetName( const char* _name )	{ name = _name; }				///< Set the name of this attribute.
	void SetValue( const char* _value )	{ value = _value; }				///< Set the value.

	void SetIntValue( int _value );										///< Set the value from an integer.
	void SetDoubleValue( double _value );								///< Set the value from a double.

    #ifdef TIXML_USE_STL
	/// STL std::string form.
	void SetName( const std::string& _name )	{ name = _name; }	
	/// STL std::string form.	
	void SetValue( const std::string& _value )	{ value = _value; }
	#endif

	/// Get the next sibling attribute in the DOM. Returns null at end.
	const TiXmlAttribute* Next() const;
	TiXmlAttribute* Next() {
		return const_cast< TiXmlAttribute* >( (const_cast< const TiXmlAttribute* >(this))->Next() ); 
	}

	/// Get the previous sibling attribute in the DOM. Returns null at beginning.
	const TiXmlAttribute* Previous() const;
	TiXmlAttribute* Previous() {
		return const_cast< TiXmlAttribute* >( (const_cast< const TiXmlAttribute* >(this))->Previous() ); 
	}

	bool operator==( const TiXmlAttribute& rhs ) const { return rhs.name == name; }
	bool operator<( const TiXmlAttribute& rhs )	 const { return name < rhs.name; }
	bool operator>( const TiXmlAttribute& rhs )  const { return name > rhs.name; }

	/*	Attribute parsing starts: first letter of the name
						 returns: the next char after the value end quote
	*/
	virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );

	// Prints this Attribute to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const {
		Print( cfile, depth, 0 );
	}
	void Print( FILE* cfile, int depth, TIXML_STRING* str ) const;

	// [internal use]
	// Set the document pointer so the attribute can report errors.
	void SetDocument( TiXmlDocument* doc )	{ document = doc; }

private:
	TiXmlAttribute( const TiXmlAttribute& );				// not implemented.
	void operator=( const TiXmlAttribute& base );	// not allowed.

	TiXmlDocument*	document;	// A pointer back to a document, for error reporting.
	TIXML_STRING name;
	TIXML_STRING value;
	TiXmlAttribute*	prev;
	TiXmlAttribute*	next;
};


/*	A class used to manage a group of attributes.
	It is only used internally, both by the ELEMENT and the DECLARATION.
	
	The set can be changed transparent to the Element and Declaration
	classes that use it, but NOT transparent to the Attribute
	which has to implement a next() and previous() method. Which makes
	it a bit problematic and prevents the use of STL.

	This version is implemented with circular lists because:
		- I like circular lists
		- it demonstrates some independence from the (typical) doubly linked list.
*/
class TiXmlAttributeSet
{
public:
	TiXmlAttributeSet();
	~TiXmlAttributeSet();

	void Add( TiXmlAttribute* attribute );
	void Remove( TiXmlAttribute* attribute );

	const TiXmlAttribute* First()	const	{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
	TiXmlAttribute* First()					{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
	const TiXmlAttribute* Last() const		{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
	TiXmlAttribute* Last()					{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }

	TiXmlAttribute*	Find( const char* _name ) const;
	TiXmlAttribute* FindOrCreate( const char* _name );

#	ifdef TIXML_USE_STL
	TiXmlAttribute*	Find( const std::string& _name ) const;
	TiXmlAttribute* FindOrCreate( const std::string& _name );
#	endif


private:
	//*ME:	Because of hidden/disabled copy-construktor in TiXmlAttribute (sentinel-element),
	//*ME:	this class must be also use a hidden/disabled copy-constructor !!!
	TiXmlAttributeSet( const TiXmlAttributeSet& );	// not allowed
	void operator=( const TiXmlAttributeSet& );	// not allowed (as TiXmlAttribute)

	TiXmlAttribute sentinel;
};


/** The element is a container class. It has a value, the element name,
	and can contain other elements, text, comments, and unknowns.
	Elements also contain an arbitrary number of attributes.
*/
class TiXmlElement : public TiXmlNode
{
public:
	/// Construct an element.
	TiXmlElement (const char * in_value);

	#ifdef TIXML_USE_STL
	/// std::string constructor.
	TiXmlElement( const std::string& _value );
	#endif

	TiXmlElement( const TiXmlElement& );

	void operator=( const TiXmlElement& base );

	virtual ~TiXmlElement();

	/** Given an attribute name, Attribute() returns the value
		for the attribute of that name, or null if none exists.
	*/
	const char* Attribute( const char* name ) const;

	/** Given an attribute name, Attribute() returns the value
		for the attribute of that name, or null if none exists.
		If the attribute exists and can be converted to an integer,
		the integer value will be put in the return 'i', if 'i'
		is non-null.
	*/
	const char* Attribute( const char* name, int* i ) const;

	/** Given an attribute name, Attribute() returns the value
		for the attribute of that name, or null if none exists.
		If the attribute exists and can be converted to an double,
		the double value will be put in the return 'd', if 'd'
		is non-null.
	*/
	const char* Attribute( const char* name, double* d ) const;

	/** QueryIntAttribute examines the attribute - it is an alternative to the
		Attribute() method with richer error checking.
		If the attribute is an integer, it is stored in 'value' and 
		the call returns TIXML_SUCCESS. If it is not
		an integer, it returns TIXML_WRONG_TYPE. If the attribute
		does not exist, then TIXML_NO_ATTRIBUTE is returned.
	*/	
	int QueryIntAttribute( const char* name, int* _value ) const;
	/// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
	int QueryDoubleAttribute( const char* name, double* _value ) const;
	/// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
	int QueryFloatAttribute( const char* name, float* _value ) const {
		double d;
		int result = QueryDoubleAttribute( name, &d );
		if ( result == TIXML_SUCCESS ) {
			*_value = (float)d;
		}
		return result;
	}

    #ifdef TIXML_USE_STL
	/// QueryStringAttribute examines the attribute - see QueryIntAttribute().
	int QueryStringAttribute( const char* name, std::string* _value ) const {
		const char* cstr = Attribute( name );
		if ( cstr ) {
			*_value = std::string( cstr );
			return TIXML_SUCCESS;
		}
		return TIXML_NO_ATTRIBUTE;
	}

	/** Template form of the attribute query which will try to read the
		attribute into the specified type. Very easy, very powerful, but
		be careful to make sure to call this with the correct type.
		
		NOTE: This method doesn't work correctly for 'string' types that contain spaces.

		@return TIXML_SUCCESS, TIXML_WRONG_TYPE, or TIXML_NO_ATTRIBUTE
	*/
	template< typename T > int QueryValueAttribute( const std::string& name, T* outValue ) const
	{
		const TiXmlAttribute* node = attributeSet.Find( name );
		if ( !node )
			return TIXML_NO_ATTRIBUTE;

		std::stringstream sstream( node->ValueStr() );
		sstream >> *outValue;
		if ( !sstream.fail() )
			return TIXML_SUCCESS;
		return TIXML_WRONG_TYPE;
	}

	int QueryValueAttribute( const std::string& name, std::string* outValue ) const
	{
		const TiXmlAttribute* node = attributeSet.Find( name );
		if ( !node )
			return TIXML_NO_ATTRIBUTE;
		*outValue = node->ValueStr();
		return TIXML_SUCCESS;
	}
	#endif

	/** Sets an attribute of name to a given value. The attribute
		will be created if it does not exist, or changed if it does.
	*/
	void SetAttribute( const char* name, const char * _value );

    #ifdef TIXML_USE_STL
	const std::string* Attribute( const std::string& name ) const;
	const std::string* Attribute( const std::string& name, int* i ) const;
	const std::string* Attribute( const std::string& name, double* d ) const;
	int QueryIntAttribute( const std::string& name, int* _value ) const;
	int QueryDoubleAttribute( const std::string& name, double* _value ) const;

	/// STL std::string form.
	void SetAttribute( const std::string& name, const std::string& _value );
	///< STL std::string form.
	void SetAttribute( const std::string& name, int _value );
	///< STL std::string form.
	void SetDoubleAttribute( const std::string& name, double value );
	#endif

	/** Sets an attribute of name to a given value. The attribute
		will be created if it does not exist, or changed if it does.
	*/
	void SetAttribute( const char * name, int value );

	/** Sets an attribute of name to a given value. The attribute
		will be created if it does not exist, or changed if it does.
	*/
	void SetDoubleAttribute( const char * name, double value );

	/** Deletes an attribute with the given name.
	*/
	void RemoveAttribute( const char * name );
    #ifdef TIXML_USE_STL
	void RemoveAttribute( const std::string& name )	{	RemoveAttribute (name.c_str ());	}	///< STL std::string form.
	#endif

	const TiXmlAttribute* FirstAttribute() const	{ return attributeSet.First(); }		///< Access the first attribute in this element.
	TiXmlAttribute* FirstAttribute() 				{ return attributeSet.First(); }
	const TiXmlAttribute* LastAttribute()	const 	{ return attributeSet.Last(); }		///< Access the last attribute in this element.
	TiXmlAttribute* LastAttribute()					{ return attributeSet.Last(); }

	/** Convenience function for easy access to the text inside an element. Although easy
		and concise, GetText() is limited compared to getting the TiXmlText child
		and accessing it directly.
	
		If the first child of 'this' is a TiXmlText, the GetText()
		returns the character string of the Text node, else null is returned.

		This is a convenient method for getting the text of simple contained text:
		@verbatim
		<foo>This is text</foo>
		const char* str = fooElement->GetText();
		@endverbatim

		'str' will be a pointer to "This is text". 
		
		Note that this function can be misleading. If the element foo was created from
		this XML:
		@verbatim
		<foo><b>This is text</b></foo> 
		@endverbatim

		then the value of str would be null. The first child node isn't a text node, it is
		another element. From this XML:
		@verbatim
		<foo>This is <b>text</b></foo> 
		@endverbatim
		GetText() will return "This is ".

		WARNING: GetText() accesses a child node - don't become confused with the 
				 similarly named TiXmlHandle::Text() and TiXmlNode::ToText() which are 
				 safe type casts on the referenced node.
	*/
	const char* GetText() const;

	/// Creates a new Element and returns it - the returned element is a copy.
	virtual TiXmlNode* Clone() const;
	// Print the Element to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	/*	Attribtue parsing starts: next char past '<'
						 returns: next char past '>'
	*/
	virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );

	virtual const TiXmlElement*     ToElement()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual TiXmlElement*           ToElement()	          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( TiXmlVisitor* visitor ) const;

protected:

	void CopyTo( TiXmlElement* target ) const;
	void ClearThis();	// like clear, but initializes 'this' object as well

	// Used to be public [internal use]
	#ifdef TIXML_USE_STL
	virtual void StreamIn( std::istream * in, TIXML_STRING * tag );
	#endif
	/*	[internal use]
		Reads the "value" of the element -- another element, or text.
		This should terminate with the current end tag.
	*/
	const char* ReadValue( const char* in, TiXmlParsingData* prevData, TiXmlEncoding encoding );

private:
	TiXmlAttributeSet attributeSet;
};


/**	An XML comment.
*/
class TiXmlComment : public TiXmlNode
{
public:
	/// Constructs an empty comment.
	TiXmlComment() : TiXmlNode( TiXmlNode::TINYXML_COMMENT ) {}
	/// Construct a comment from text.
	TiXmlComment( const char* _value ) : TiXmlNode( TiXmlNode::TINYXML_COMMENT ) {
		SetValue( _value );
	}
	TiXmlComment( const TiXmlComment& );
	void operator=( const TiXmlComment& base );

	virtual ~TiXmlComment()	{}

	/// Returns a copy of this Comment.
	virtual TiXmlNode* Clone() const;
	// Write this Comment to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	/*	Attribtue parsing starts: at the ! of the !--
						 returns: next char past '>'
	*/
	virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );

	virtual const TiXmlComment*  ToComment() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual TiXmlComment*  ToComment() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( TiXmlVisitor* visitor ) const;

protected:
	void CopyTo( TiXmlComment* target ) const;

	// used to be public
	#ifdef TIXML_USE_STL
	virtual void StreamIn( std::istream * in, TIXML_STRING * tag );
	#endif
//	virtual void StreamOut( TIXML_OSTREAM * out ) const;

private:

};


/** XML text. A text node can have 2 ways to output the next. "normal" output 
	and CDATA. It will default to the mode it was parsed from the XML file and
	you generally want to leave it alone, but you can change the output mode with 
	SetCDATA() and query it with CDATA().
*/
class TiXmlText : public TiXmlNode
{
	friend class TiXmlElement;
public:
	/** Constructor for text element. By default, it is treated as 
		normal, encoded text. If you want it be output as a CDATA text
		element, set the parameter _cdata to 'true'
	*/
	TiXmlText (const char * initValue ) : TiXmlNode (TiXmlNode::TINYXML_TEXT)
	{
		SetValue( initValue );
		cdata = false;
	}
	virtual ~TiXmlText() {}

	#ifdef TIXML_USE_STL
	/// Constructor.
	TiXmlText( const std::string& initValue ) : TiXmlNode (TiXmlNode::TINYXML_TEXT)
	{
		SetValue( initValue );
		cdata = false;
	}
	#endif

	TiXmlText( const TiXmlText& copy ) : TiXmlNode( TiXmlNode::TINYXML_TEXT )	{ copy.CopyTo( this ); }
	void operator=( const TiXmlText& base )							 	{ base.CopyTo( this ); }

	// Write this text object to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	/// Queries whether this represents text using a CDATA section.
	bool CDATA() const				{ return cdata; }
	/// Turns on or off a CDATA representation of text.
	void SetCDATA( bool _cdata )	{ cdata = _cdata; }

	virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );

	virtual const TiXmlText* ToText() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual TiXmlText*       ToText()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( TiXmlVisitor* content ) const;

protected :
	///  [internal use] Creates a new Element and returns it.
	virtual TiXmlNode* Clone() const;
	void CopyTo( TiXmlText* target ) const;

	bool Blank() const;	// returns true if all white space and new lines
	// [internal use]
	#ifdef TIXML_USE_STL
	virtual void StreamIn( std::istream * in, TIXML_STRING * tag );
	#endif

private:
	bool cdata;			// true if this should be input and output as a CDATA style text element
};


/** In correct XML the declaration is the first entry in the file.
	@verbatim
		<?xml version="1.0" standalone="yes"?>
	@endverbatim

	TinyXml will happily read or write files without a declaration,
	however. There are 3 possible attributes to the declaration:
	version, encoding, and standalone.

	Note: In this version of the code, the attributes are
	handled as special cases, not generic attributes, simply
	because there can only be at most 3 and they are always the same.
*/
class TiXmlDeclaration : public TiXmlNode
{
public:
	/// Construct an empty declaration.
	TiXmlDeclaration()   : TiXmlNode( TiXmlNode::TINYXML_DECLARATION ) {}

#ifdef TIXML_USE_STL
	/// Constructor.
	TiXmlDeclaration(	const std::string& _version,
						const std::string& _encoding,
						const std::string& _standalone );
#endif

	/// Construct.
	TiXmlDeclaration(	const char* _version,
						const char* _encoding,
						const char* _standalone );

	TiXmlDeclaration( const TiXmlDeclaration& copy );
	void operator=( const TiXmlDeclaration& copy );

	virtual ~TiXmlDeclaration()	{}

	/// Version. Will return an empty string if none was found.
	const char *Version() const			{ return version.c_str (); }
	/// Encoding. Will return an empty string if none was found.
	const char *Encoding() const		{ return encoding.c_str (); }
	/// Is this a standalone document?
	const char *Standalone() const		{ return standalone.c_str (); }

	/// Creates a copy of this Declaration and returns it.
	virtual TiXmlNode* Clone() const;
	// Print this declaration to a FILE stream.
	virtual void Print( FILE* cfile, int depth, TIXML_STRING* str ) const;
	virtual void Print( FILE* cfile, int depth ) const {
		Print( cfile, depth, 0 );
	}

	virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );

	virtual const TiXmlDeclaration* ToDeclaration() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual TiXmlDeclaration*       ToDeclaration()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( TiXmlVisitor* visitor ) const;

protected:
	void CopyTo( TiXmlDeclaration* target ) const;
	// used to be public
	#ifdef TIXML_USE_STL
	virtual void StreamIn( std::istream * in, TIXML_STRING * tag );
	#endif

private:

	TIXML_STRING version;
	TIXML_STRING encoding;
	TIXML_STRING standalone;
};


/** Any tag that tinyXml doesn't recognize is saved as an
	unknown. It is a tag of text, but should not be modified.
	It will be written back to the XML, unchanged, when the file
	is saved.

	DTD tags get thrown into TiXmlUnknowns.
*/
class TiXmlUnknown : public TiXmlNode
{
public:
	TiXmlUnknown() : TiXmlNode( TiXmlNode::TINYXML_UNKNOWN )	{}
	virtual ~TiXmlUnknown() {}

	TiXmlUnknown( const TiXmlUnknown& copy ) : TiXmlNode( TiXmlNode::TINYXML_UNKNOWN )		{ copy.CopyTo( this ); }
	void operator=( const TiXmlUnknown& copy )										{ copy.CopyTo( this ); }

	/// Creates a copy of this Unknown and returns it.
	virtual TiXmlNode* Clone() const;
	// Print this Unknown to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );

	virtual const TiXmlUnknown*     ToUnknown()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual TiXmlUnknown*           ToUnknown()	    { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( TiXmlVisitor* content ) const;

protected:
	void CopyTo( TiXmlUnknown* target ) const;

	#ifdef TIXML_USE_STL
	virtual void StreamIn( std::istream * in, TIXML_STRING * tag );
	#endif

private:

};


/** Always the top level node. A document binds together all the
	XML pieces. It can be saved, loaded, and printed to the screen.
	The 'value' of a document node is the xml file name.
*/
class TiXmlDocument : public TiXmlNode
{
public:
	/// Create an empty document, that has no name.
	TiXmlDocument();
	/// Create a document with a name. The name of the document is also the filename of the xml.
	TiXmlDocument( const char * documentName );

	#ifdef TIXML_USE_STL
	/// Constructor.
	TiXmlDocument( const std::string& documentName );
	#endif

	TiXmlDocument( const TiXmlDocument& copy );
	void operator=( const TiXmlDocument& copy );

	virtual ~TiXmlDocument() {}

	/** Load a file using the current document value.
		Returns true if successful. Will delete any existing
		document data before loading.
	*/
	bool LoadFile( TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
	/// Save a file using the current document value. Returns true if successful.
	bool SaveFile() const;
	/// Load a file using the given filename. Returns true if successful.
	bool LoadFile( const char * filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
	/// Save a file using the given filename. Returns true if successful.
	bool SaveFile( const char * filename ) const;
	/** Load a file using the given FILE*. Returns true if successful. Note that this method
		doesn't stream - the entire object pointed at by the FILE*
		will be interpreted as an XML file. TinyXML doesn't stream in XML from the current
		file location. Streaming may be added in the future.
	*/
	bool LoadFile( FILE*, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
	/// Save a file using the given FILE*. Returns true if successful.
	bool SaveFile( FILE* ) const;

	#ifdef TIXML_USE_STL
	bool LoadFile( const std::string& filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING )			///< STL std::string version.
	{
		return LoadFile( filename.c_str(), encoding );
	}
	bool SaveFile( const std::string& filename ) const		///< STL std::string version.
	{
		return SaveFile( filename.c_str() );
	}
	#endif

	/** Parse the given null terminated block of xml data. Passing in an encoding to this
		method (either TIXML_ENCODING_LEGACY or TIXML_ENCODING_UTF8 will force TinyXml
		to use that encoding, regardless of what TinyXml might otherwise try to detect.
	*/
	virtual const char* Parse( const char* p, TiXmlParsingData* data = 0, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );

	/** Get the root element -- the only top level element -- of the document.
		In well formed XML, there should only be one. TinyXml is tolerant of
		multiple elements at the document level.
	*/
	const TiXmlElement* RootElement() const		{ return FirstChildElement(); }
	TiXmlElement* RootElement()					{ return FirstChildElement(); }

	/** If an error occurs, Error will be set to true. Also,
		- The ErrorId() will contain the integer identifier of the error (not generally useful)
		- The ErrorDesc() method will return the name of the error. (very useful)
		- The ErrorRow() and ErrorCol() will return the location of the error (if known)
	*/	
	bool Error() const						{ return error; }

	/// Contains a textual (english) description of the error if one occurs.
	const char * ErrorDesc() const	{ return errorDesc.c_str (); }

	/** Generally, you probably want the error string ( ErrorDesc() ). But if you
		prefer the ErrorId, this function will fetch it.
	*/
	int ErrorId()	const				{ return errorId; }

	/** Returns the location (if known) of the error. The first column is column 1, 
		and the first row is row 1. A value of 0 means the row and column wasn't applicable
		(memory errors, for example, have no row/column) or the parser lost the error. (An
		error in the error reporting, in that case.)

		@sa SetTabSize, Row, Column
	*/
	int ErrorRow() const	{ return errorLocation.row+1; }
	int ErrorCol() const	{ return errorLocation.col+1; }	///< The column where the error occured. See ErrorRow()

	/** SetTabSize() allows the error reporting functions (ErrorRow() and ErrorCol())
		to report the correct values for row and column. It does not change the output
		or input in any way.
		
		By calling this method, with a tab size
		greater than 0, the row and column of each node and attribute is stored
		when the file is loaded. Very useful for tracking the DOM back in to
		the source file.

		The tab size is required for calculating the location of nodes. If not
		set, the default of 4 is used. The tabsize is set per document. Setting
		the tabsize to 0 disables row/column tracking.

		Note that row and column tracking is not supported when using operator>>.

		The tab size needs to be enabled before the parse or load. Correct usage:
		@verbatim
		TiXmlDocument doc;
		doc.SetTabSize( 8 );
		doc.Load( "myfile.xml" );
		@endverbatim

		@sa Row, Column
	*/
	void SetTabSize( int _tabsize )		{ tabsize = _tabsize; }

	int TabSize() const	{ return tabsize; }

	/** If you have handled the error, it can be reset with this call. The error
		state is automatically cleared if you Parse a new XML block.
	*/
	void ClearError()						{	error = false; 
												errorId = 0; 
												errorDesc = ""; 
												errorLocation.row = errorLocation.col = 0; 
												//errorLocation.last = 0; 
											}

	/** Write the document to standard out using formatted printing ("pretty print"). */
	void Print() const						{ Print( stdout, 0 ); }

	/* Write the document to a string using formatted printing ("pretty print"). This
		will allocate a character array (new char[]) and return it as a pointer. The
		calling code pust call delete[] on the return char* to avoid a memory leak.
	*/
	//char* PrintToMemory() const; 

	/// Print this Document to a FILE stream.
	virtual void Print( FILE* cfile, int depth = 0 ) const;
	// [internal use]
	void SetError( int err, const char* errorLocation, TiXmlParsingData* prevData, TiXmlEncoding encoding );

	virtual const TiXmlDocument*    ToDocument()    const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual TiXmlDocument*          ToDocument()          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( TiXmlVisitor* content ) const;

protected :
	// [internal use]
	virtual TiXmlNode* Clone() const;
	#ifdef TIXML_USE_STL
	virtual void StreamIn( std::istream * in, TIXML_STRING * tag );
	#endif

private:
	void CopyTo( TiXmlDocument* target ) const;

	bool error;
	int  errorId;
	TIXML_STRING errorDesc;
	int tabsize;
	TiXmlCursor errorLocation;
	bool useMicrosoftBOM;		// the UTF-8 BOM were found when read. Note this, and try to write.
};


/**
	A TiXmlHandle is a class that wraps a node pointer with null checks; this is
	an incredibly useful thing. Note that TiXmlHandle is not part of the TinyXml
	DOM structure. It is a separate utility class.

	Take an example:
	@verbatim
	<Document>
		<Element attributeA = "valueA">
			<Child attributeB = "value1" />
			<Child attributeB = "value2" />
		</Element>
	<Document>
	@endverbatim

	Assuming you want the value of "attributeB" in the 2nd "Child" element, it's very 
	easy to write a *lot* of code that looks like:

	@verbatim
	TiXmlElement* root = document.FirstChildElement( "Document" );
	if ( root )
	{
		TiXmlElement* element = root->FirstChildElement( "Element" );
		if ( element )
		{
			TiXmlElement* child = element->FirstChildElement( "Child" );
			if ( child )
			{
				TiXmlElement* child2 = child->NextSiblingElement( "Child" );
				if ( child2 )
				{
					// Finally do something useful.
	@endverbatim

	And that doesn't even cover "else" cases. TiXmlHandle addresses the verbosity
	of such code. A TiXmlHandle checks for null	pointers so it is perfectly safe 
	and correct to use:

	@verbatim
	TiXmlHandle docHandle( &document );
	TiXmlElement* child2 = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", 1 ).ToElement();
	if ( child2 )
	{
		// do something useful
	@endverbatim

	Which is MUCH more concise and useful.

	It is also safe to copy handles - internally they are nothing more than node pointers.
	@verbatim
	TiXmlHandle handleCopy = handle;
	@endverbatim

	What they should not be used for is iteration:

	@verbatim
	int i=0; 
	while ( true )
	{
		TiXmlElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", i ).ToElement();
		if ( !child )
			break;
		// do something
		++i;
	}
	@endverbatim

	It seems reasonable, but it is in fact two embedded while loops. The Child method is 
	a linear walk to find the element, so this code would iterate much more than it needs 
	to. Instead, prefer:

	@verbatim
	TiXmlElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).FirstChild( "Child" ).ToElement();

	for( child; child; child=child->NextSiblingElement() )
	{
		// do something
	}
	@endverbatim
*/
class TiXmlHandle
{
public:
	/// Create a handle from any node (at any depth of the tree.) This can be a null pointer.
	TiXmlHandle( TiXmlNode* _node )					{ this->node = _node; }
	/// Copy constructor
	TiXmlHandle( const TiXmlHandle& ref )			{ this->node = ref.node; }
	TiXmlHandle operator=( const TiXmlHandle& ref ) { this->node = ref.node; return *this; }

	/// Return a handle to the first child node.
	TiXmlHandle FirstChild() const;
	/// Return a handle to the first child node with the given name.
	TiXmlHandle FirstChild( const char * value ) const;
	/// Return a handle to the first child element.
	TiXmlHandle FirstChildElement() const;
	/// Return a handle to the first child element with the given name.
	TiXmlHandle FirstChildElement( const char * value ) const;

	/** Return a handle to the "index" child with the given name. 
		The first child is 0, the second 1, etc.
	*/
	TiXmlHandle Child( const char* value, int index ) const;
	/** Return a handle to the "index" child. 
		The first child is 0, the second 1, etc.
	*/
	TiXmlHandle Child( int index ) const;
	/** Return a handle to the "index" child element with the given name. 
		The first child element is 0, the second 1, etc. Note that only TiXmlElements
		are indexed: other types are not counted.
	*/
	TiXmlHandle ChildElement( const char* value, int index ) const;
	/** Return a handle to the "index" child element. 
		The first child element is 0, the second 1, etc. Note that only TiXmlElements
		are indexed: other types are not counted.
	*/
	TiXmlHandle ChildElement( int index ) const;

	#ifdef TIXML_USE_STL
	TiXmlHandle FirstChild( const std::string& _value ) const				{ return FirstChild( _value.c_str() ); }
	TiXmlHandle FirstChildElement( const std::string& _value ) const		{ return FirstChildElement( _value.c_str() ); }

	TiXmlHandle Child( const std::string& _value, int index ) const			{ return Child( _value.c_str(), index ); }
	TiXmlHandle ChildElement( const std::string& _value, int index ) const	{ return ChildElement( _value.c_str(), index ); }
	#endif

	/** Return the handle as a TiXmlNode. This may return null.
	*/
	TiXmlNode* ToNode() const			{ return node; } 
	/** Return the handle as a TiXmlElement. This may return null.
	*/
	TiXmlElement* ToElement() const		{ return ( ( node && node->ToElement() ) ? node->ToElement() : 0 ); }
	/**	Return the handle as a TiXmlText. This may return null.
	*/
	TiXmlText* ToText() const			{ return ( ( node && node->ToText() ) ? node->ToText() : 0 ); }
	/** Return the handle as a TiXmlUnknown. This may return null.
	*/
	TiXmlUnknown* ToUnknown() const		{ return ( ( node && node->ToUnknown() ) ? node->ToUnknown() : 0 ); }

	/** @deprecated use ToNode. 
		Return the handle as a TiXmlNode. This may return null.
	*/
	TiXmlNode* Node() const			{ return ToNode(); } 
	/** @deprecated use ToElement. 
		Return the handle as a TiXmlElement. This may return null.
	*/
	TiXmlElement* Element() const	{ return ToElement(); }
	/**	@deprecated use ToText()
		Return the handle as a TiXmlText. This may return null.
	*/
	TiXmlText* Text() const			{ return ToText(); }
	/** @deprecated use ToUnknown()
		Return the handle as a TiXmlUnknown. This may return null.
	*/
	TiXmlUnknown* Unknown() const	{ return ToUnknown(); }

private:
	TiXmlNode* node;
};


/** Print to memory functionality. The TiXmlPrinter is useful when you need to:

	-# Print to memory (especially in non-STL mode)
	-# Control formatting (line endings, etc.)

	When constructed, the TiXmlPrinter is in its default "pretty printing" mode.
	Before calling Accept() you can call methods to control the printing
	of the XML document. After TiXmlNode::Accept() is called, the printed document can
	be accessed via the CStr(), Str(), and Size() methods.

	TiXmlPrinter uses the Visitor API.
	@verbatim
	TiXmlPrinter printer;
	printer.SetIndent( "\t" );

	doc.Accept( &printer );
	fprintf( stdout, "%s", printer.CStr() );
	@endverbatim
*/
class TiXmlPrinter : public TiXmlVisitor
{
public:
	TiXmlPrinter() : depth( 0 ), simpleTextPrint( false ),
					 buffer(), indent( "    " ), lineBreak( "\n" ) {}

	virtual bool VisitEnter( const TiXmlDocument& doc );
	virtual bool VisitExit( const TiXmlDocument& doc );

	virtual bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute );
	virtual bool VisitExit( const TiXmlElement& element );

	virtual bool Visit( const TiXmlDeclaration& declaration );
	virtual bool Visit( const TiXmlText& text );
	virtual bool Visit( const TiXmlComment& comment );
	virtual bool Visit( const TiXmlUnknown& unknown );

	/** Set the indent characters for printing. By default 4 spaces
		but tab (\t) is also useful, or null/empty string for no indentation.
	*/
	void SetIndent( const char* _indent )			{ indent = _indent ? _indent : "" ; }
	/// Query the indention string.
	const char* Indent()							{ return indent.c_str(); }
	/** Set the line breaking string. By default set to newline (\n). 
		Some operating systems prefer other characters, or can be
		set to the null/empty string for no indenation.
	*/
	void SetLineBreak( const char* _lineBreak )		{ lineBreak = _lineBreak ? _lineBreak : ""; }
	/// Query the current line breaking string.
	const char* LineBreak()							{ return lineBreak.c_str(); }

	/** Switch over to "stream printing" which is the most dense formatting without 
		linebreaks. Common when the XML is needed for network transmission.
	*/
	void SetStreamPrinting()						{ indent = "";
													  lineBreak = "";
													}	
	/// Return the result.
	const char* CStr()								{ return buffer.c_str(); }
	/// Return the length of the result string.
	size_t Size()									{ return buffer.size(); }

	#ifdef TIXML_USE_STL
	/// Return the result.
	const std::string& Str()						{ return buffer; }
	#endif

private:
	void DoIndent()	{
		for( int i=0; i<depth; ++i )
			buffer += indent;
	}
	void DoLineBreak() {
		buffer += lineBreak;
	}

	int depth;
	bool simpleTextPrint;
	TIXML_STRING buffer;
	TIXML_STRING indent;
	TIXML_STRING lineBreak;
};


#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
 

// tinyxml.cpp:

/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/*
#include <ctype.h>

#ifdef TIXML_USE_STL
#include <sstream>
#include <iostream>
#endif

#include "tinyxml.h"
*/

FILE* TiXmlFOpen( const char* filename, const char* mode );

bool TiXmlBase::condenseWhiteSpace = true;

// Microsoft compiler security
FILE* TiXmlFOpen( const char* filename, const char* mode )
{
	#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
		FILE* fp = 0;
		errno_t err = fopen_s( &fp, filename, mode );
		if ( !err && fp )
			return fp;
		return 0;
	#else
		return fopen( filename, mode );
	#endif
}

void TiXmlBase::EncodeString( const TIXML_STRING& str, TIXML_STRING* outString )
{
	int i=0;

	while( i<(int)str.length() )
	{
		unsigned char c = (unsigned char) str[i];

		if (    c == '&' 
		     && i < ( (int)str.length() - 2 )
			 && str[i+1] == '#'
			 && str[i+2] == 'x' )
		{
			// Hexadecimal character reference.
			// Pass through unchanged.
			// &#xA9;	-- copyright symbol, for example.
			//
			// The -1 is a bug fix from Rob Laveaux. It keeps
			// an overflow from happening if there is no ';'.
			// There are actually 2 ways to exit this loop -
			// while fails (error case) and break (semicolon found).
			// However, there is no mechanism (currently) for
			// this function to return an error.
			while ( i<(int)str.length()-1 )
			{
				outString->append( str.c_str() + i, 1 );
				++i;
				if ( str[i] == ';' )
					break;
			}
		}
		else if ( c == '&' )
		{
			outString->append( entity[0].str, entity[0].strLength );
			++i;
		}
		else if ( c == '<' )
		{
			outString->append( entity[1].str, entity[1].strLength );
			++i;
		}
		else if ( c == '>' )
		{
			outString->append( entity[2].str, entity[2].strLength );
			++i;
		}
		else if ( c == '\"' )
		{
			outString->append( entity[3].str, entity[3].strLength );
			++i;
		}
		else if ( c == '\'' )
		{
			outString->append( entity[4].str, entity[4].strLength );
			++i;
		}
		else if ( c < 32 )
		{
			// Easy pass at non-alpha/numeric/symbol
			// Below 32 is symbolic.
			char buf[ 32 ];
			
			#if defined(TIXML_SNPRINTF)		
				TIXML_SNPRINTF( buf, sizeof(buf), "&#x%02X;", (unsigned) ( c & 0xff ) );
			#else
				sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
			#endif		

			//*ME:	warning C4267: convert 'size_t' to 'int'
			//*ME:	Int-Cast to make compiler happy ...
			outString->append( buf, (int)strlen( buf ) );
			++i;
		}
		else
		{
			//char realc = (char) c;
			//outString->append( &realc, 1 );
			*outString += (char) c;	// somewhat more efficient function call.
			++i;
		}
	}
}


TiXmlNode::TiXmlNode( NodeType _type ) : TiXmlBase()
{
	parent = 0;
	type = _type;
	firstChild = 0;
	lastChild = 0;
	prev = 0;
	next = 0;
}


TiXmlNode::~TiXmlNode()
{
	TiXmlNode* node = firstChild;
	TiXmlNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}	
}


void TiXmlNode::CopyTo( TiXmlNode* target ) const
{
	target->SetValue (value.c_str() );
	target->userData = userData; 
	target->location = location;
}


void TiXmlNode::Clear()
{
	TiXmlNode* node = firstChild;
	TiXmlNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}	

	firstChild = 0;
	lastChild = 0;
}


TiXmlNode* TiXmlNode::LinkEndChild( TiXmlNode* node )
{
	assert( node->parent == 0 || node->parent == this );
	assert( node->GetDocument() == 0 || node->GetDocument() == this->GetDocument() );

	if ( node->Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		delete node;
		if ( GetDocument() ) GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	node->parent = this;

	node->prev = lastChild;
	node->next = 0;

	if ( lastChild )
		lastChild->next = node;
	else
		firstChild = node;			// it was an empty list.

	lastChild = node;
	return node;
}


TiXmlNode* TiXmlNode::InsertEndChild( const TiXmlNode& addThis )
{
	if ( addThis.Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		if ( GetDocument() ) GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}
	TiXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;

	return LinkEndChild( node );
}


TiXmlNode* TiXmlNode::InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis )
{	
	if ( !beforeThis || beforeThis->parent != this ) {
		return 0;
	}
	if ( addThis.Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		if ( GetDocument() ) GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	TiXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->next = beforeThis;
	node->prev = beforeThis->prev;
	if ( beforeThis->prev )
	{
		beforeThis->prev->next = node;
	}
	else
	{
		assert( firstChild == beforeThis );
		firstChild = node;
	}
	beforeThis->prev = node;
	return node;
}


TiXmlNode* TiXmlNode::InsertAfterChild( TiXmlNode* afterThis, const TiXmlNode& addThis )
{
	if ( !afterThis || afterThis->parent != this ) {
		return 0;
	}
	if ( addThis.Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		if ( GetDocument() ) GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	TiXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->prev = afterThis;
	node->next = afterThis->next;
	if ( afterThis->next )
	{
		afterThis->next->prev = node;
	}
	else
	{
		assert( lastChild == afterThis );
		lastChild = node;
	}
	afterThis->next = node;
	return node;
}


TiXmlNode* TiXmlNode::ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis )
{
	if ( !replaceThis )
		return 0;

	if ( replaceThis->parent != this )
		return 0;

	if ( withThis.ToDocument() ) {
		// A document can never be a child.	Thanks to Noam.
		TiXmlDocument* document = GetDocument();
		if ( document ) 
			document->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	TiXmlNode* node = withThis.Clone();
	if ( !node )
		return 0;

	node->next = replaceThis->next;
	node->prev = replaceThis->prev;

	if ( replaceThis->next )
		replaceThis->next->prev = node;
	else
		lastChild = node;

	if ( replaceThis->prev )
		replaceThis->prev->next = node;
	else
		firstChild = node;

	delete replaceThis;
	node->parent = this;
	return node;
}


bool TiXmlNode::RemoveChild( TiXmlNode* removeThis )
{
	if ( !removeThis ) {
		return false;
	}

	if ( removeThis->parent != this )
	{	
		assert( 0 );
		return false;
	}

	if ( removeThis->next )
		removeThis->next->prev = removeThis->prev;
	else
		lastChild = removeThis->prev;

	if ( removeThis->prev )
		removeThis->prev->next = removeThis->next;
	else
		firstChild = removeThis->next;

	delete removeThis;
	return true;
}

const TiXmlNode* TiXmlNode::FirstChild( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = firstChild; node; node = node->next )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


const TiXmlNode* TiXmlNode::LastChild( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = lastChild; node; node = node->prev )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


const TiXmlNode* TiXmlNode::IterateChildren( const TiXmlNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild();
	}
	else
	{
		assert( previous->parent == this );
		return previous->NextSibling();
	}
}


const TiXmlNode* TiXmlNode::IterateChildren( const char * val, const TiXmlNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild( val );
	}
	else
	{
		assert( previous->parent == this );
		return previous->NextSibling( val );
	}
}


const TiXmlNode* TiXmlNode::NextSibling( const char * _value ) const 
{
	const TiXmlNode* node;
	for ( node = next; node; node = node->next )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


const TiXmlNode* TiXmlNode::PreviousSibling( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = prev; node; node = node->prev )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


void TiXmlElement::RemoveAttribute( const char * name )
{
    #ifdef TIXML_USE_STL
	TIXML_STRING str( name );
	TiXmlAttribute* node = attributeSet.Find( str );
	#else
	TiXmlAttribute* node = attributeSet.Find( name );
	#endif
	if ( node )
	{
		attributeSet.Remove( node );
		delete node;
	}
}

const TiXmlElement* TiXmlNode::FirstChildElement() const
{
	const TiXmlNode* node;

	for (	node = FirstChild();
			node;
			node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlElement* TiXmlNode::FirstChildElement( const char * _value ) const
{
	const TiXmlNode* node;

	for (	node = FirstChild( _value );
			node;
			node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlElement* TiXmlNode::NextSiblingElement() const
{
	const TiXmlNode* node;

	for (	node = NextSibling();
			node;
			node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlElement* TiXmlNode::NextSiblingElement( const char * _value ) const
{
	const TiXmlNode* node;

	for (	node = NextSibling( _value );
			node;
			node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlDocument* TiXmlNode::GetDocument() const
{
	const TiXmlNode* node;

	for( node = this; node; node = node->parent )
	{
		if ( node->ToDocument() )
			return node->ToDocument();
	}
	return 0;
}


TiXmlElement::TiXmlElement (const char * _value)
	: TiXmlNode( TiXmlNode::TINYXML_ELEMENT )
{
	firstChild = lastChild = 0;
	value = _value;
}


#ifdef TIXML_USE_STL
TiXmlElement::TiXmlElement( const std::string& _value ) 
	: TiXmlNode( TiXmlNode::TINYXML_ELEMENT )
{
	firstChild = lastChild = 0;
	value = _value;
}
#endif


TiXmlElement::TiXmlElement( const TiXmlElement& copy)
	: TiXmlNode( TiXmlNode::TINYXML_ELEMENT )
{
	firstChild = lastChild = 0;
	copy.CopyTo( this );	
}


void TiXmlElement::operator=( const TiXmlElement& base )
{
	ClearThis();
	base.CopyTo( this );
}


TiXmlElement::~TiXmlElement()
{
	ClearThis();
}


void TiXmlElement::ClearThis()
{
	Clear();
	while( attributeSet.First() )
	{
		TiXmlAttribute* node = attributeSet.First();
		attributeSet.Remove( node );
		delete node;
	}
}


const char* TiXmlElement::Attribute( const char* name ) const
{
	const TiXmlAttribute* node = attributeSet.Find( name );
	if ( node )
		return node->Value();
	return 0;
}


#ifdef TIXML_USE_STL
const std::string* TiXmlElement::Attribute( const std::string& name ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( attrib )
		return &attrib->ValueStr();
	return 0;
}
#endif


const char* TiXmlElement::Attribute( const char* name, int* i ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const char* result = 0;

	if ( attrib ) {
		result = attrib->Value();
		if ( i ) {
			attrib->QueryIntValue( i );
		}
	}
	return result;
}


#ifdef TIXML_USE_STL
const std::string* TiXmlElement::Attribute( const std::string& name, int* i ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const std::string* result = 0;

	if ( attrib ) {
		result = &attrib->ValueStr();
		if ( i ) {
			attrib->QueryIntValue( i );
		}
	}
	return result;
}
#endif


const char* TiXmlElement::Attribute( const char* name, double* d ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const char* result = 0;

	if ( attrib ) {
		result = attrib->Value();
		if ( d ) {
			attrib->QueryDoubleValue( d );
		}
	}
	return result;
}


#ifdef TIXML_USE_STL
const std::string* TiXmlElement::Attribute( const std::string& name, double* d ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const std::string* result = 0;

	if ( attrib ) {
		result = &attrib->ValueStr();
		if ( d ) {
			attrib->QueryDoubleValue( d );
		}
	}
	return result;
}
#endif


int TiXmlElement::QueryIntAttribute( const char* name, int* ival ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryIntValue( ival );
}


#ifdef TIXML_USE_STL
int TiXmlElement::QueryIntAttribute( const std::string& name, int* ival ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryIntValue( ival );
}
#endif


int TiXmlElement::QueryDoubleAttribute( const char* name, double* dval ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryDoubleValue( dval );
}


#ifdef TIXML_USE_STL
int TiXmlElement::QueryDoubleAttribute( const std::string& name, double* dval ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryDoubleValue( dval );
}
#endif


void TiXmlElement::SetAttribute( const char * name, int val )
{	
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetIntValue( val );
	}
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute( const std::string& name, int val )
{	
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetIntValue( val );
	}
}
#endif


void TiXmlElement::SetDoubleAttribute( const char * name, double val )
{	
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetDoubleValue( val );
	}
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetDoubleAttribute( const std::string& name, double val )
{	
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetDoubleValue( val );
	}
}
#endif 


void TiXmlElement::SetAttribute( const char * cname, const char * cvalue )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( cname );
	if ( attrib ) {
		attrib->SetValue( cvalue );
	}
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute( const std::string& _name, const std::string& _value )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( _name );
	if ( attrib ) {
		attrib->SetValue( _value );
	}
}
#endif


void TiXmlElement::Print( FILE* cfile, int depth ) const
{
	int i;
	assert( cfile );
	for ( i=0; i<depth; i++ ) {
		fprintf( cfile, "    " );
	}

	fprintf( cfile, "<%s", value.c_str() );

	const TiXmlAttribute* attrib;
	for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
	{
		fprintf( cfile, " " );
		attrib->Print( cfile, depth );
	}

	// There are 3 different formatting approaches:
	// 1) An element without children is printed as a <foo /> node
	// 2) An element with only a text child is printed as <foo> text </foo>
	// 3) An element with children is printed on multiple lines.
	TiXmlNode* node;
	if ( !firstChild )
	{
		fprintf( cfile, " />" );
	}
	else if ( firstChild == lastChild && firstChild->ToText() )
	{
		fprintf( cfile, ">" );
		firstChild->Print( cfile, depth + 1 );
		fprintf( cfile, "</%s>", value.c_str() );
	}
	else
	{
		fprintf( cfile, ">" );

		for ( node = firstChild; node; node=node->NextSibling() )
		{
			if ( !node->ToText() )
			{
				fprintf( cfile, "\n" );
			}
			node->Print( cfile, depth+1 );
		}
		fprintf( cfile, "\n" );
		for( i=0; i<depth; ++i ) {
			fprintf( cfile, "    " );
		}
		fprintf( cfile, "</%s>", value.c_str() );
	}
}


void TiXmlElement::CopyTo( TiXmlElement* target ) const
{
	// superclass:
	TiXmlNode::CopyTo( target );

	// Element class: 
	// Clone the attributes, then clone the children.
	const TiXmlAttribute* attribute = 0;
	for(	attribute = attributeSet.First();
	attribute;
	attribute = attribute->Next() )
	{
		target->SetAttribute( attribute->Name(), attribute->Value() );
	}

	TiXmlNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		target->LinkEndChild( node->Clone() );
	}
}

bool TiXmlElement::Accept( TiXmlVisitor* visitor ) const
{
	if ( visitor->VisitEnter( *this, attributeSet.First() ) ) 
	{
		for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
		{
			if ( !node->Accept( visitor ) )
				break;
		}
	}
	return visitor->VisitExit( *this );
}


TiXmlNode* TiXmlElement::Clone() const
{
	TiXmlElement* clone = new TiXmlElement( Value() );
	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


const char* TiXmlElement::GetText() const
{
	const TiXmlNode* child = this->FirstChild();
	if ( child ) {
		const TiXmlText* childText = child->ToText();
		if ( childText ) {
			return childText->Value();
		}
	}
	return 0;
}


TiXmlDocument::TiXmlDocument() : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	ClearError();
}

TiXmlDocument::TiXmlDocument( const char * documentName ) : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	value = documentName;
	ClearError();
}


#ifdef TIXML_USE_STL
TiXmlDocument::TiXmlDocument( const std::string& documentName ) : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
    value = documentName;
	ClearError();
}
#endif


TiXmlDocument::TiXmlDocument( const TiXmlDocument& copy ) : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	copy.CopyTo( this );
}


void TiXmlDocument::operator=( const TiXmlDocument& copy )
{
	Clear();
	copy.CopyTo( this );
}


bool TiXmlDocument::LoadFile( TiXmlEncoding encoding )
{
	return LoadFile( Value(), encoding );
}


bool TiXmlDocument::SaveFile() const
{
	return SaveFile( Value() );
}

bool TiXmlDocument::LoadFile( const char* _filename, TiXmlEncoding encoding )
{
	TIXML_STRING filename( _filename );
	value = filename;

	// reading in binary mode so that tinyxml can normalize the EOL
	FILE* file = TiXmlFOpen( value.c_str (), "rb" );	

	if ( file )
	{
		bool result = LoadFile( file, encoding );
		fclose( file );
		return result;
	}
	else
	{
		SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}
}

bool TiXmlDocument::LoadFile( FILE* file, TiXmlEncoding encoding )
{
	if ( !file ) 
	{
		SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Delete the existing data:
	Clear();
	location.Clear();

	// Get the file size, so we can pre-allocate the string. HUGE speed impact.
	long length = 0;
	fseek( file, 0, SEEK_END );
	length = ftell( file );
	fseek( file, 0, SEEK_SET );

	// Strange case, but good to handle up front.
	if ( length <= 0 )
	{
		SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Subtle bug here. TinyXml did use fgets. But from the XML spec:
	// 2.11 End-of-Line Handling
	// <snip>
	// <quote>
	// ...the XML processor MUST behave as if it normalized all line breaks in external 
	// parsed entities (including the document entity) on input, before parsing, by translating 
	// both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
	// a single #xA character.
	// </quote>
	//
	// It is not clear fgets does that, and certainly isn't clear it works cross platform. 
	// Generally, you expect fgets to translate from the convention of the OS to the c/unix
	// convention, and not work generally.

	/*
	while( fgets( buf, sizeof(buf), file ) )
	{
		data += buf;
	}
	*/

	char* buf = new char[ length+1 ];
	buf[0] = 0;

	if ( fread( buf, length, 1, file ) != 1 ) {
		delete [] buf;
		SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Process the buffer in place to normalize new lines. (See comment above.)
	// Copies from the 'p' to 'q' pointer, where p can advance faster if
	// a newline-carriage return is hit.
	//
	// Wikipedia:
	// Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or 
	// CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
	//		* LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
    //		* CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
    //		* CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

	const char* p = buf;	// the read head
	char* q = buf;			// the write head
	const char CR = 0x0d;
	const char LF = 0x0a;

	buf[length] = 0;
	while( *p ) {
		assert( p < (buf+length) );
		assert( q <= (buf+length) );
		assert( q <= p );

		if ( *p == CR ) {
			*q++ = LF;
			p++;
			if ( *p == LF ) {		// check for CR+LF (and skip LF)
				p++;
			}
		}
		else {
			*q++ = *p++;
		}
	}
	assert( q <= (buf+length) );
	*q = 0;

	Parse( buf, 0, encoding );

	delete [] buf;
	return !Error();
}


bool TiXmlDocument::SaveFile( const char * filename ) const
{
	// The old c stuff lives on...
	FILE* fp = TiXmlFOpen( filename, "w" );
	if ( fp )
	{
		bool result = SaveFile( fp );
		fclose( fp );
		return result;
	}
	return false;
}


bool TiXmlDocument::SaveFile( FILE* fp ) const
{
	if ( useMicrosoftBOM ) 
	{
		const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
		const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
		const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

		fputc( TIXML_UTF_LEAD_0, fp );
		fputc( TIXML_UTF_LEAD_1, fp );
		fputc( TIXML_UTF_LEAD_2, fp );
	}
	Print( fp, 0 );
	return (ferror(fp) == 0);
}


void TiXmlDocument::CopyTo( TiXmlDocument* target ) const
{
	TiXmlNode::CopyTo( target );

	target->error = error;
	target->errorId = errorId;
	target->errorDesc = errorDesc;
	target->tabsize = tabsize;
	target->errorLocation = errorLocation;
	target->useMicrosoftBOM = useMicrosoftBOM;

	TiXmlNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		target->LinkEndChild( node->Clone() );
	}	
}


TiXmlNode* TiXmlDocument::Clone() const
{
	TiXmlDocument* clone = new TiXmlDocument();
	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


void TiXmlDocument::Print( FILE* cfile, int depth ) const
{
	assert( cfile );
	for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
	{
		node->Print( cfile, depth );
		fprintf( cfile, "\n" );
	}
}


bool TiXmlDocument::Accept( TiXmlVisitor* visitor ) const
{
	if ( visitor->VisitEnter( *this ) )
	{
		for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
		{
			if ( !node->Accept( visitor ) )
				break;
		}
	}
	return visitor->VisitExit( *this );
}


const TiXmlAttribute* TiXmlAttribute::Next() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( next->value.empty() && next->name.empty() )
		return 0;
	return next;
}

/*
TiXmlAttribute* TiXmlAttribute::Next()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( next->value.empty() && next->name.empty() )
		return 0;
	return next;
}
*/

const TiXmlAttribute* TiXmlAttribute::Previous() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( prev->value.empty() && prev->name.empty() )
		return 0;
	return prev;
}

/*
TiXmlAttribute* TiXmlAttribute::Previous()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( prev->value.empty() && prev->name.empty() )
		return 0;
	return prev;
}
*/

void TiXmlAttribute::Print( FILE* cfile, int /*depth*/, TIXML_STRING* str ) const
{
	TIXML_STRING n, v;

	EncodeString( name, &n );
	EncodeString( value, &v );

	if (value.find ('\"') == TIXML_STRING::npos) {
		if ( cfile ) {
		fprintf (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
		}
		if ( str ) {
			(*str) += n; (*str) += "=\""; (*str) += v; (*str) += "\"";
		}
	}
	else {
		if ( cfile ) {
		fprintf (cfile, "%s='%s'", n.c_str(), v.c_str() );
		}
		if ( str ) {
			(*str) += n; (*str) += "='"; (*str) += v; (*str) += "'";
		}
	}
}


int TiXmlAttribute::QueryIntValue( int* ival ) const
{
	if ( TIXML_SSCANF( value.c_str(), "%d", ival ) == 1 )
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}

int TiXmlAttribute::QueryDoubleValue( double* dval ) const
{
	if ( TIXML_SSCANF( value.c_str(), "%lf", dval ) == 1 )
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}

void TiXmlAttribute::SetIntValue( int _value )
{
	char buf [64];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
	#else
		sprintf (buf, "%d", _value);
	#endif
	SetValue (buf);
}

void TiXmlAttribute::SetDoubleValue( double _value )
{
	char buf [256];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF( buf, sizeof(buf), "%g", _value);
	#else
		sprintf (buf, "%g", _value);
	#endif
	SetValue (buf);
}

int TiXmlAttribute::IntValue() const
{
	return atoi (value.c_str ());
}

double  TiXmlAttribute::DoubleValue() const
{
	return atof (value.c_str ());
}


TiXmlComment::TiXmlComment( const TiXmlComment& copy ) : TiXmlNode( TiXmlNode::TINYXML_COMMENT )
{
	copy.CopyTo( this );
}


void TiXmlComment::operator=( const TiXmlComment& base )
{
	Clear();
	base.CopyTo( this );
}


void TiXmlComment::Print( FILE* cfile, int depth ) const
{
	assert( cfile );
	for ( int i=0; i<depth; i++ )
	{
		fprintf( cfile,  "    " );
	}
	fprintf( cfile, "<!--%s-->", value.c_str() );
}


void TiXmlComment::CopyTo( TiXmlComment* target ) const
{
	TiXmlNode::CopyTo( target );
}


bool TiXmlComment::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlComment::Clone() const
{
	TiXmlComment* clone = new TiXmlComment();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


void TiXmlText::Print( FILE* cfile, int depth ) const
{
	assert( cfile );
	if ( cdata )
	{
		int i;
		fprintf( cfile, "\n" );
		for ( i=0; i<depth; i++ ) {
			fprintf( cfile, "    " );
		}
		fprintf( cfile, "<![CDATA[%s]]>\n", value.c_str() );	// unformatted output
	}
	else
	{
		TIXML_STRING buffer;
		EncodeString( value, &buffer );
		fprintf( cfile, "%s", buffer.c_str() );
	}
}


void TiXmlText::CopyTo( TiXmlText* target ) const
{
	TiXmlNode::CopyTo( target );
	target->cdata = cdata;
}


bool TiXmlText::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlText::Clone() const
{	
	TiXmlText* clone = 0;
	clone = new TiXmlText( "" );

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


TiXmlDeclaration::TiXmlDeclaration( const char * _version,
									const char * _encoding,
									const char * _standalone )
	: TiXmlNode( TiXmlNode::TINYXML_DECLARATION )
{
	version = _version;
	encoding = _encoding;
	standalone = _standalone;
}


#ifdef TIXML_USE_STL
TiXmlDeclaration::TiXmlDeclaration(	const std::string& _version,
									const std::string& _encoding,
									const std::string& _standalone )
	: TiXmlNode( TiXmlNode::TINYXML_DECLARATION )
{
	version = _version;
	encoding = _encoding;
	standalone = _standalone;
}
#endif


TiXmlDeclaration::TiXmlDeclaration( const TiXmlDeclaration& copy )
	: TiXmlNode( TiXmlNode::TINYXML_DECLARATION )
{
	copy.CopyTo( this );	
}


void TiXmlDeclaration::operator=( const TiXmlDeclaration& copy )
{
	Clear();
	copy.CopyTo( this );
}


void TiXmlDeclaration::Print( FILE* cfile, int /*depth*/, TIXML_STRING* str ) const
{
	if ( cfile ) fprintf( cfile, "<?xml " );
	if ( str )	 (*str) += "<?xml ";

	if ( !version.empty() ) {
		if ( cfile ) fprintf (cfile, "version=\"%s\" ", version.c_str ());
		if ( str ) { (*str) += "version=\""; (*str) += version; (*str) += "\" "; }
	}
	if ( !encoding.empty() ) {
		if ( cfile ) fprintf (cfile, "encoding=\"%s\" ", encoding.c_str ());
		if ( str ) { (*str) += "encoding=\""; (*str) += encoding; (*str) += "\" "; }
	}
	if ( !standalone.empty() ) {
		if ( cfile ) fprintf (cfile, "standalone=\"%s\" ", standalone.c_str ());
		if ( str ) { (*str) += "standalone=\""; (*str) += standalone; (*str) += "\" "; }
	}
	if ( cfile ) fprintf( cfile, "?>" );
	if ( str )	 (*str) += "?>";
}


void TiXmlDeclaration::CopyTo( TiXmlDeclaration* target ) const
{
	TiXmlNode::CopyTo( target );

	target->version = version;
	target->encoding = encoding;
	target->standalone = standalone;
}


bool TiXmlDeclaration::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlDeclaration::Clone() const
{	
	TiXmlDeclaration* clone = new TiXmlDeclaration();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


void TiXmlUnknown::Print( FILE* cfile, int depth ) const
{
	for ( int i=0; i<depth; i++ )
		fprintf( cfile, "    " );
	fprintf( cfile, "<%s>", value.c_str() );
}


void TiXmlUnknown::CopyTo( TiXmlUnknown* target ) const
{
	TiXmlNode::CopyTo( target );
}


bool TiXmlUnknown::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlUnknown::Clone() const
{
	TiXmlUnknown* clone = new TiXmlUnknown();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


TiXmlAttributeSet::TiXmlAttributeSet()
{
	sentinel.next = &sentinel;
	sentinel.prev = &sentinel;
}


TiXmlAttributeSet::~TiXmlAttributeSet()
{
	assert( sentinel.next == &sentinel );
	assert( sentinel.prev == &sentinel );
}


void TiXmlAttributeSet::Add( TiXmlAttribute* addMe )
{
    #ifdef TIXML_USE_STL
	assert( !Find( TIXML_STRING( addMe->Name() ) ) );	// Shouldn't be multiply adding to the set.
	#else
	assert( !Find( addMe->Name() ) );	// Shouldn't be multiply adding to the set.
	#endif

	addMe->next = &sentinel;
	addMe->prev = sentinel.prev;

	sentinel.prev->next = addMe;
	sentinel.prev      = addMe;
}

void TiXmlAttributeSet::Remove( TiXmlAttribute* removeMe )
{
	TiXmlAttribute* node;

	for( node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node == removeMe )
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
			node->next = 0;
			node->prev = 0;
			return;
		}
	}
	assert( 0 );		// we tried to remove a non-linked attribute.
}


#ifdef TIXML_USE_STL
TiXmlAttribute* TiXmlAttributeSet::Find( const std::string& name ) const
{
	for( TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node->name == name )
			return node;
	}
	return 0;
}

TiXmlAttribute* TiXmlAttributeSet::FindOrCreate( const std::string& _name )
{
	TiXmlAttribute* attrib = Find( _name );
	if ( !attrib ) {
		attrib = new TiXmlAttribute();
		Add( attrib );
		attrib->SetName( _name );
	}
	return attrib;
}
#endif


TiXmlAttribute* TiXmlAttributeSet::Find( const char* name ) const
{
	for( TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( strcmp( node->name.c_str(), name ) == 0 )
			return node;
	}
	return 0;
}


TiXmlAttribute* TiXmlAttributeSet::FindOrCreate( const char* _name )
{
	TiXmlAttribute* attrib = Find( _name );
	if ( !attrib ) {
		attrib = new TiXmlAttribute();
		Add( attrib );
		attrib->SetName( _name );
	}
	return attrib;
}


#ifdef TIXML_USE_STL	
std::istream& operator>> (std::istream & in, TiXmlNode & base)
{
	TIXML_STRING tag;
	tag.reserve( 8 * 1000 );
	base.StreamIn( &in, &tag );

	base.Parse( tag.c_str(), 0, TIXML_DEFAULT_ENCODING );
	return in;
}
#endif


#ifdef TIXML_USE_STL	
std::ostream& operator<< (std::ostream & out, const TiXmlNode & base)
{
	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	base.Accept( &printer );
	out << printer.Str();

	return out;
}


std::string& operator<< (std::string& out, const TiXmlNode& base )
{
	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	base.Accept( &printer );
	out.append( printer.Str() );

	return out;
}
#endif


TiXmlHandle TiXmlHandle::FirstChild() const
{
	if ( node )
	{
		TiXmlNode* child = node->FirstChild();
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChild( const char * value ) const
{
	if ( node )
	{
		TiXmlNode* child = node->FirstChild( value );
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChildElement() const
{
	if ( node )
	{
		TiXmlElement* child = node->FirstChildElement();
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChildElement( const char * value ) const
{
	if ( node )
	{
		TiXmlElement* child = node->FirstChildElement( value );
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::Child( int count ) const
{
	if ( node )
	{
		int i;
		TiXmlNode* child = node->FirstChild();
		for (	i=0;
				child && i<count;
				child = child->NextSibling(), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::Child( const char* value, int count ) const
{
	if ( node )
	{
		int i;
		TiXmlNode* child = node->FirstChild( value );
		for (	i=0;
				child && i<count;
				child = child->NextSibling( value ), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::ChildElement( int count ) const
{
	if ( node )
	{
		int i;
		TiXmlElement* child = node->FirstChildElement();
		for (	i=0;
				child && i<count;
				child = child->NextSiblingElement(), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::ChildElement( const char* value, int count ) const
{
	if ( node )
	{
		int i;
		TiXmlElement* child = node->FirstChildElement( value );
		for (	i=0;
				child && i<count;
				child = child->NextSiblingElement( value ), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


bool TiXmlPrinter::VisitEnter( const TiXmlDocument& )
{
	return true;
}

bool TiXmlPrinter::VisitExit( const TiXmlDocument& )
{
	return true;
}

bool TiXmlPrinter::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute )
{
	DoIndent();
	buffer += "<";
	buffer += element.Value();

	for( const TiXmlAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next() )
	{
		buffer += " ";
		attrib->Print( 0, 0, &buffer );
	}

	if ( !element.FirstChild() ) 
	{
		buffer += " />";
		DoLineBreak();
	}
	else 
	{
		buffer += ">";
		if (    element.FirstChild()->ToText()
			  && element.LastChild() == element.FirstChild()
			  && element.FirstChild()->ToText()->CDATA() == false )
		{
			simpleTextPrint = true;
			// no DoLineBreak()!
		}
		else
		{
			DoLineBreak();
		}
	}
	++depth;	
	return true;
}


bool TiXmlPrinter::VisitExit( const TiXmlElement& element )
{
	--depth;
	if ( !element.FirstChild() ) 
	{
		// nothing.
	}
	else 
	{
		if ( simpleTextPrint )
		{
			simpleTextPrint = false;
		}
		else
		{
			DoIndent();
		}
		buffer += "</";
		buffer += element.Value();
		buffer += ">";
		DoLineBreak();
	}
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlText& text )
{
	if ( text.CDATA() )
	{
		DoIndent();
		buffer += "<![CDATA[";
		buffer += text.Value();
		buffer += "]]>";
		DoLineBreak();
	}
	else if ( simpleTextPrint )
	{
		TIXML_STRING str;
		TiXmlBase::EncodeString( text.ValueTStr(), &str );
		buffer += str;
	}
	else
	{
		DoIndent();
		TIXML_STRING str;
		TiXmlBase::EncodeString( text.ValueTStr(), &str );
		buffer += str;
		DoLineBreak();
	}
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlDeclaration& declaration )
{
	DoIndent();
	declaration.Print( 0, 0, &buffer );
	DoLineBreak();
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlComment& comment )
{
	DoIndent();
	buffer += "<!--";
	buffer += comment.Value();
	buffer += "-->";
	DoLineBreak();
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlUnknown& unknown )
{
	DoIndent();
	buffer += "<";
	buffer += unknown.Value();
	buffer += ">";
	DoLineBreak();
	return true;
}
 
// tinyxmlparser.cpp:
/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.
*/

/*
#include <ctype.h>
#include <stddef.h>

#include "tinyxml.h"

//#define DEBUG_PARSER
#if defined( DEBUG_PARSER )
#	if defined( DEBUG ) && defined( _MSC_VER )
#		include <windows.h>
#		define TIXML_LOG OutputDebugString
#	else
#		define TIXML_LOG printf
#	endif
#endif

*/

// Note tha "PutString" hardcodes the same list. This
// is less flexible than it appears. Changing the entries
// or order will break putstring.	
TiXmlBase::Entity TiXmlBase::entity[ NUM_ENTITY ] = 
{
	{ "&amp;",  5, '&' },
	{ "&lt;",   4, '<' },
	{ "&gt;",   4, '>' },
	{ "&quot;", 6, '\"' },
	{ "&apos;", 6, '\'' }
};

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
// Including the basic of this table, which determines the #bytes in the
// sequence from the lead byte. 1 placed for invalid sequences --
// although the result will be junk, pass it through as much as possible.
// Beware of the non-characters in UTF-8:	
//				ef bb bf (Microsoft "lead bytes")
//				ef bf be
//				ef bf bf 

const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

const int TiXmlBase::utf8ByteTable[256] = 
{
	//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x00
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x10
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x20
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x30
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70	End of ASCII range
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 0x80 to 0xc1 invalid
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xa0 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xb0 
		1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xc0 0xc2 to 0xdf 2 byte
		2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xd0
		3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	// 0xe0 0xe0 to 0xef 3 byte
		4,	4,	4,	4,	4,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1	// 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
};


void TiXmlBase::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
{
	const unsigned long BYTE_MASK = 0xBF;
	const unsigned long BYTE_MARK = 0x80;
	const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

	if (input < 0x80) 
		*length = 1;
	else if ( input < 0x800 )
		*length = 2;
	else if ( input < 0x10000 )
		*length = 3;
	else if ( input < 0x200000 )
		*length = 4;
	else
		{ *length = 0; return; }	// This code won't covert this correctly anyway.

	output += *length;

	// Scary scary fall throughs.
	switch (*length) 
	{
		case 4:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 3:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 2:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 1:
			--output; 
			*output = (char)(input | FIRST_BYTE_MARK[*length]);
	}
}


/*static*/ int TiXmlBase::IsAlpha( unsigned char anyByte, TiXmlEncoding /*encoding*/ )
{
	// This will only work for low-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it is quite tricky trying
	// to figure out alhabetical vs. not across encoding. So take a very 
	// conservative approach.

//	if ( encoding == TIXML_ENCODING_UTF8 )
//	{
		if ( anyByte < 127 )
			return isalpha( anyByte );
		else
			return 1;	// What else to do? The unicode set is huge...get the english ones right.
//	}
//	else
//	{
//		return isalpha( anyByte );
//	}
}


/*static*/ int TiXmlBase::IsAlphaNum( unsigned char anyByte, TiXmlEncoding /*encoding*/ )
{
	// This will only work for low-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it is quite tricky trying
	// to figure out alhabetical vs. not across encoding. So take a very 
	// conservative approach.

//	if ( encoding == TIXML_ENCODING_UTF8 )
//	{
		if ( anyByte < 127 )
			return isalnum( anyByte );
		else
			return 1;	// What else to do? The unicode set is huge...get the english ones right.
//	}
//	else
//	{
//		return isalnum( anyByte );
//	}
}


class TiXmlParsingData
{
	friend class TiXmlDocument;
  public:
	void Stamp( const char* now, TiXmlEncoding encoding );

	const TiXmlCursor& Cursor()	{ return cursor; }

  private:
	// Only used by the document!
	TiXmlParsingData( const char* start, int _tabsize, int row, int col )
	{
		assert( start );
		stamp = start;
		tabsize = _tabsize;
		cursor.row = row;
		cursor.col = col;
	}

	TiXmlCursor		cursor;
	const char*		stamp;
	int				tabsize;
};


void TiXmlParsingData::Stamp( const char* now, TiXmlEncoding encoding )
{
	assert( now );

	// Do nothing if the tabsize is 0.
	if ( tabsize < 1 )
	{
		return;
	}

	// Get the current row, column.
	int row = cursor.row;
	int col = cursor.col;
	const char* p = stamp;
	assert( p );

	while ( p < now )
	{
		// Treat p as unsigned, so we have a happy compiler.
		const unsigned char* pU = (const unsigned char*)p;

		// Code contributed by Fletcher Dunn: (modified by lee)
		switch (*pU) {
			case 0:
				// We *should* never get here, but in case we do, don't
				// advance past the terminating null character, ever
				return;

			case '\r':
				// bump down to the next line
				++row;
				col = 0;				
				// Eat the character
				++p;

				// Check for \r\n sequence, and treat this as a single character
				if (*p == '\n') {
					++p;
				}
				break;

			case '\n':
				// bump down to the next line
				++row;
				col = 0;

				// Eat the character
				++p;

				// Check for \n\r sequence, and treat this as a single
				// character.  (Yes, this bizarre thing does occur still
				// on some arcane platforms...)
				if (*p == '\r') {
					++p;
				}
				break;

			case '\t':
				// Eat the character
				++p;

				// Skip to next tab stop
				col = (col / tabsize + 1) * tabsize;
				break;

			case TIXML_UTF_LEAD_0:
				if ( encoding == TIXML_ENCODING_UTF8 )
				{
					if ( *(p+1) && *(p+2) )
					{
						// In these cases, don't advance the column. These are
						// 0-width spaces.
						if ( *(pU+1)==TIXML_UTF_LEAD_1 && *(pU+2)==TIXML_UTF_LEAD_2 )
							p += 3;	
						else if ( *(pU+1)==0xbfU && *(pU+2)==0xbeU )
							p += 3;	
						else if ( *(pU+1)==0xbfU && *(pU+2)==0xbfU )
							p += 3;	
						else
							{ p +=3; ++col; }	// A normal character.
					}
				}
				else
				{
					++p;
					++col;
				}
				break;

			default:
				if ( encoding == TIXML_ENCODING_UTF8 )
				{
					// Eat the 1 to 4 byte utf8 character.
					int step = TiXmlBase::utf8ByteTable[*((const unsigned char*)p)];
					if ( step == 0 )
						step = 1;		// Error case from bad encoding, but handle gracefully.
					p += step;

					// Just advance one column, of course.
					++col;
				}
				else
				{
					++p;
					++col;
				}
				break;
		}
	}
	cursor.row = row;
	cursor.col = col;
	assert( cursor.row >= -1 );
	assert( cursor.col >= -1 );
	stamp = p;
	assert( stamp );
}


const char* TiXmlBase::SkipWhiteSpace( const char* p, TiXmlEncoding encoding )
{
	if ( !p || !*p )
	{
		return 0;
	}
	if ( encoding == TIXML_ENCODING_UTF8 )
	{
		while ( *p )
		{
			const unsigned char* pU = (const unsigned char*)p;
			
			// Skip the stupid Microsoft UTF-8 Byte order marks
			if (	*(pU+0)==TIXML_UTF_LEAD_0
				 && *(pU+1)==TIXML_UTF_LEAD_1 
				 && *(pU+2)==TIXML_UTF_LEAD_2 )
			{
				p += 3;
				continue;
			}
			else if(*(pU+0)==TIXML_UTF_LEAD_0
				 && *(pU+1)==0xbfU
				 && *(pU+2)==0xbeU )
			{
				p += 3;
				continue;
			}
			else if(*(pU+0)==TIXML_UTF_LEAD_0
				 && *(pU+1)==0xbfU
				 && *(pU+2)==0xbfU )
			{
				p += 3;
				continue;
			}

			if ( IsWhiteSpace( *p ) )		// Still using old rules for white space.
				++p;
			else
				break;
		}
	}
	else
	{
		while ( *p && IsWhiteSpace( *p ) )
			++p;
	}

	return p;
}

#ifdef TIXML_USE_STL
/*static*/ bool TiXmlBase::StreamWhiteSpace( std::istream * in, TIXML_STRING * tag )
{
	for( ;; )
	{
		if ( !in->good() ) return false;

		int c = in->peek();
		// At this scope, we can't get to a document. So fail silently.
		if ( !IsWhiteSpace( c ) || c <= 0 )
			return true;

		*tag += (char) in->get();
	}
}

/*static*/ bool TiXmlBase::StreamTo( std::istream * in, int character, TIXML_STRING * tag )
{
	//assert( character > 0 && character < 128 );	// else it won't work in utf-8
	while ( in->good() )
	{
		int c = in->peek();
		if ( c == character )
			return true;
		if ( c <= 0 )		// Silent failure: can't get document at this scope
			return false;

		in->get();
		*tag += (char) c;
	}
	return false;
}
#endif

// One of TinyXML's more performance demanding functions. Try to keep the memory overhead down. The
// "assign" optimization removes over 10% of the execution time.
//
const char* TiXmlBase::ReadName( const char* p, TIXML_STRING * name, TiXmlEncoding encoding )
{
	// Oddly, not supported on some comilers,
	//name->clear();
	// So use this:
	*name = "";
	assert( p );

	// Names start with letters or underscores.
	// Of course, in unicode, tinyxml has no idea what a letter *is*. The
	// algorithm is generous.
	//
	// After that, they can be letters, underscores, numbers,
	// hyphens, or colons. (Colons are valid ony for namespaces,
	// but tinyxml can't tell namespaces from names.)
	if (    p && *p 
		 && ( IsAlpha( (unsigned char) *p, encoding ) || *p == '_' ) )
	{
		const char* start = p;
		while(		p && *p
				&&	(		IsAlphaNum( (unsigned char ) *p, encoding ) 
						 || *p == '_'
						 || *p == '-'
						 || *p == '.'
						 || *p == ':' ) )
		{
			//(*name) += *p; // expensive
			++p;
		}
		if ( p-start > 0 ) {
			name->assign( start, p-start );
		}
		return p;
	}
	return 0;
}

const char* TiXmlBase::GetEntity( const char* p, char* value, int* length, TiXmlEncoding encoding )
{
	// Presume an entity, and pull it out.
    TIXML_STRING ent;
	int i;
	*length = 0;

	if ( *(p+1) && *(p+1) == '#' && *(p+2) )
	{
		unsigned long ucs = 0;
		ptrdiff_t delta = 0;
		unsigned mult = 1;

		if ( *(p+2) == 'x' )
		{
			// Hexadecimal.
			if ( !*(p+3) ) return 0;

			const char* q = p+3;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != 'x' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else if ( *q >= 'a' && *q <= 'f' )
					ucs += mult * (*q - 'a' + 10);
				else if ( *q >= 'A' && *q <= 'F' )
					ucs += mult * (*q - 'A' + 10 );
				else 
					return 0;
				mult *= 16;
				--q;
			}
		}
		else
		{
			// Decimal.
			if ( !*(p+2) ) return 0;

			const char* q = p+2;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != '#' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else 
					return 0;
				mult *= 10;
				--q;
			}
		}
		if ( encoding == TIXML_ENCODING_UTF8 )
		{
			// convert the UCS to UTF-8
			ConvertUTF32ToUTF8( ucs, value, length );
		}
		else
		{
			*value = (char)ucs;
			*length = 1;
		}
		return p + delta + 1;
	}

	// Now try to match it.
	for( i=0; i<NUM_ENTITY; ++i )
	{
		if ( strncmp( entity[i].str, p, entity[i].strLength ) == 0 )
		{
			assert( strlen( entity[i].str ) == entity[i].strLength );
			*value = entity[i].chr;
			*length = 1;
			return ( p + entity[i].strLength );
		}
	}

	// So it wasn't an entity, its unrecognized, or something like that.
	*value = *p;	// Don't put back the last one, since we return it!
	//*length = 1;	// Leave unrecognized entities - this doesn't really work.
					// Just writes strange XML.
	return p+1;
}


bool TiXmlBase::StringEqual( const char* p,
							 const char* tag,
							 bool ignoreCase,
							 TiXmlEncoding encoding )
{
	assert( p );
	assert( tag );
	if ( !p || !*p )
	{
		assert( 0 );
		return false;
	}

	const char* q = p;

	if ( ignoreCase )
	{
		while ( *q && *tag && ToLower( *q, encoding ) == ToLower( *tag, encoding ) )
		{
			++q;
			++tag;
		}

		if ( *tag == 0 )
			return true;
	}
	else
	{
		while ( *q && *tag && *q == *tag )
		{
			++q;
			++tag;
		}

		if ( *tag == 0 )		// Have we found the end of the tag, and everything equal?
			return true;
	}
	return false;
}

const char* TiXmlBase::ReadText(	const char* p, 
									TIXML_STRING * text, 
									bool trimWhiteSpace, 
									const char* endTag, 
									bool caseInsensitive,
									TiXmlEncoding encoding )
{
    *text = "";
	if (    !trimWhiteSpace			// certain tags always keep whitespace
		 || !condenseWhiteSpace )	// if true, whitespace is always kept
	{
		// Keep all the white space.
		while (	   p && *p
				&& !StringEqual( p, endTag, caseInsensitive, encoding )
			  )
		{
			int len;
			char cArr[4] = { 0, 0, 0, 0 };
			p = GetChar( p, cArr, &len, encoding );
			text->append( cArr, len );
		}
	}
	else
	{
		bool whitespace = false;

		// Remove leading white space:
		p = SkipWhiteSpace( p, encoding );
		while (	   p && *p
				&& !StringEqual( p, endTag, caseInsensitive, encoding ) )
		{
			if ( *p == '\r' || *p == '\n' )
			{
				whitespace = true;
				++p;
			}
			else if ( IsWhiteSpace( *p ) )
			{
				whitespace = true;
				++p;
			}
			else
			{
				// If we've found whitespace, add it before the
				// new character. Any whitespace just becomes a space.
				if ( whitespace )
				{
					(*text) += ' ';
					whitespace = false;
				}
				int len;
				char cArr[4] = { 0, 0, 0, 0 };
				p = GetChar( p, cArr, &len, encoding );
				if ( len == 1 )
					(*text) += cArr[0];	// more efficient
				else
					text->append( cArr, len );
			}
		}
	}
	if ( p && *p ) 
		p += strlen( endTag );
	return p;
}

#ifdef TIXML_USE_STL

void TiXmlDocument::StreamIn( std::istream * in, TIXML_STRING * tag )
{
	// The basic issue with a document is that we don't know what we're
	// streaming. Read something presumed to be a tag (and hope), then
	// identify it, and call the appropriate stream method on the tag.
	//
	// This "pre-streaming" will never read the closing ">" so the
	// sub-tag can orient itself.

	if ( !StreamTo( in, '<', tag ) ) 
	{
		SetError( TIXML_ERROR_PARSING_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return;
	}

	while ( in->good() )
	{
		int tagIndex = (int) tag->length();
		while ( in->good() && in->peek() != '>' )
		{
			int c = in->get();
			if ( c <= 0 )
			{
				SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
				break;
			}
			(*tag) += (char) c;
		}

		if ( in->good() )
		{
			// We now have something we presume to be a node of 
			// some sort. Identify it, and call the node to
			// continue streaming.
			TiXmlNode* node = Identify( tag->c_str() + tagIndex, TIXML_DEFAULT_ENCODING );

			if ( node )
			{
				node->StreamIn( in, tag );
				bool isElement = node->ToElement() != 0;
				delete node;
				node = 0;

				// If this is the root element, we're done. Parsing will be
				// done by the >> operator.
				if ( isElement )
				{
					return;
				}
			}
			else
			{
				SetError( TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN );
				return;
			}
		}
	}
	// We should have returned sooner.
	SetError( TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN );
}

#endif

const char* TiXmlDocument::Parse( const char* p, TiXmlParsingData* prevData, TiXmlEncoding encoding )
{
	ClearError();

	// Parse away, at the document level. Since a document
	// contains nothing but other tags, most of what happens
	// here is skipping white space.
	if ( !p || !*p )
	{
		SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	// Note that, for a document, this needs to come
	// before the while space skip, so that parsing
	// starts from the pointer we are given.
	location.Clear();
	if ( prevData )
	{
		location.row = prevData->cursor.row;
		location.col = prevData->cursor.col;
	}
	else
	{
		location.row = 0;
		location.col = 0;
	}
	TiXmlParsingData data( p, TabSize(), location.row, location.col );
	location = data.Cursor();

	if ( encoding == TIXML_ENCODING_UNKNOWN )
	{
		// Check for the Microsoft UTF-8 lead bytes.
		const unsigned char* pU = (const unsigned char*)p;
		if (	*(pU+0) && *(pU+0) == TIXML_UTF_LEAD_0
			 && *(pU+1) && *(pU+1) == TIXML_UTF_LEAD_1
			 && *(pU+2) && *(pU+2) == TIXML_UTF_LEAD_2 )
		{
			encoding = TIXML_ENCODING_UTF8;
			useMicrosoftBOM = true;
		}
	}

    p = SkipWhiteSpace( p, encoding );
	if ( !p )
	{
		SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	while ( p && *p )
	{
		TiXmlNode* node = Identify( p, encoding );
		if ( node )
		{
			p = node->Parse( p, &data, encoding );
			LinkEndChild( node );
		}
		else
		{
			break;
		}

		// Did we get encoding info?
		if (    encoding == TIXML_ENCODING_UNKNOWN
			 && node->ToDeclaration() )
		{
			TiXmlDeclaration* dec = node->ToDeclaration();
			const char* enc = dec->Encoding();
			assert( enc );

			if ( *enc == 0 )
				encoding = TIXML_ENCODING_UTF8;
			else if ( StringEqual( enc, "UTF-8", true, TIXML_ENCODING_UNKNOWN ) )
				encoding = TIXML_ENCODING_UTF8;
			else if ( StringEqual( enc, "UTF8", true, TIXML_ENCODING_UNKNOWN ) )
				encoding = TIXML_ENCODING_UTF8;	// incorrect, but be nice
			else 
				encoding = TIXML_ENCODING_LEGACY;
		}

		p = SkipWhiteSpace( p, encoding );
	}

	// Was this empty?
	if ( !firstChild ) {
		SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding );
		return 0;
	}

	// All is well.
	return p;
}

void TiXmlDocument::SetError( int err, const char* pError, TiXmlParsingData* data, TiXmlEncoding encoding )
{	
	// The first error in a chain is more accurate - don't set again!
	if ( error )
		return;

	assert( err > 0 && err < TIXML_ERROR_STRING_COUNT );
	error   = true;
	errorId = err;
	errorDesc = errorString[ errorId ];

	errorLocation.Clear();
	if ( pError && data )
	{
		data->Stamp( pError, encoding );
		errorLocation = data->Cursor();
	}
}


TiXmlNode* TiXmlNode::Identify( const char* p, TiXmlEncoding encoding )
{
	TiXmlNode* returnNode = 0;

	p = SkipWhiteSpace( p, encoding );
	if( !p || !*p || *p != '<' )
	{
		return 0;
	}

	p = SkipWhiteSpace( p, encoding );

	if ( !p || !*p )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };
	const char* cdataHeader = { "<![CDATA[" };

	if ( StringEqual( p, xmlHeader, true, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Declaration\n" );
		#endif
		returnNode = new TiXmlDeclaration();
	}
	else if ( StringEqual( p, commentHeader, false, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Comment\n" );
		#endif
		returnNode = new TiXmlComment();
	}
	else if ( StringEqual( p, cdataHeader, false, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing CDATA\n" );
		#endif
		TiXmlText* text = new TiXmlText( "" );
		text->SetCDATA( true );
		returnNode = text;
	}
	else if ( StringEqual( p, dtdHeader, false, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown(1)\n" );
		#endif
		returnNode = new TiXmlUnknown();
	}
	else if (    IsAlpha( *(p+1), encoding )
			  || *(p+1) == '_' )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Element\n" );
		#endif
		returnNode = new TiXmlElement( "" );
	}
	else
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown(2)\n" );
		#endif
		returnNode = new TiXmlUnknown();
	}

	if ( returnNode )
	{
		// Set the parent, so it can report errors
		returnNode->parent = this;
	}
	return returnNode;
}

#ifdef TIXML_USE_STL

void TiXmlElement::StreamIn (std::istream * in, TIXML_STRING * tag)
{
	// We're called with some amount of pre-parsing. That is, some of "this"
	// element is in "tag". Go ahead and stream to the closing ">"
	while( in->good() )
	{
		int c = in->get();
		if ( c <= 0 )
		{
			TiXmlDocument* document = GetDocument();
			if ( document )
				document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}
		(*tag) += (char) c ;
		
		if ( c == '>' )
			break;
	}

	if ( tag->length() < 3 ) return;

	// Okay...if we are a "/>" tag, then we're done. We've read a complete tag.
	// If not, identify and stream.

	if (    tag->at( tag->length() - 1 ) == '>' 
		 && tag->at( tag->length() - 2 ) == '/' )
	{
		// All good!
		return;
	}
	else if ( tag->at( tag->length() - 1 ) == '>' )
	{
		// There is more. Could be:
		//		text
		//		cdata text (which looks like another node)
		//		closing tag
		//		another node.
		for ( ;; )
		{
			StreamWhiteSpace( in, tag );

			// Do we have text?
			if ( in->good() && in->peek() != '<' ) 
			{
				// Yep, text.
				TiXmlText text( "" );
				text.StreamIn( in, tag );

				// What follows text is a closing tag or another node.
				// Go around again and figure it out.
				continue;
			}

			// We now have either a closing tag...or another node.
			// We should be at a "<", regardless.
			if ( !in->good() ) return;
			assert( in->peek() == '<' );
			int tagIndex = (int) tag->length();

			bool closingTag = false;
			bool firstCharFound = false;

			for( ;; )
			{
				if ( !in->good() )
					return;

				int c = in->peek();
				if ( c <= 0 )
				{
					TiXmlDocument* document = GetDocument();
					if ( document )
						document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
					return;
				}
				
				if ( c == '>' )
					break;

				*tag += (char) c;
				in->get();

				// Early out if we find the CDATA id.
				if ( c == '[' && tag->size() >= 9 )
				{
					size_t len = tag->size();
					const char* start = tag->c_str() + len - 9;
					if ( strcmp( start, "<![CDATA[" ) == 0 ) {
						assert( !closingTag );
						break;
					}
				}

				if ( !firstCharFound && c != '<' && !IsWhiteSpace( c ) )
				{
					firstCharFound = true;
					if ( c == '/' )
						closingTag = true;
				}
			}
			// If it was a closing tag, then read in the closing '>' to clean up the input stream.
			// If it was not, the streaming will be done by the tag.
			if ( closingTag )
			{
				if ( !in->good() )
					return;

				int c = in->get();
				if ( c <= 0 )
				{
					TiXmlDocument* document = GetDocument();
					if ( document )
						document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
					return;
				}
				assert( c == '>' );
				*tag += (char) c;

				// We are done, once we've found our closing tag.
				return;
			}
			else
			{
				// If not a closing tag, id it, and stream.
				const char* tagloc = tag->c_str() + tagIndex;
				TiXmlNode* node = Identify( tagloc, TIXML_DEFAULT_ENCODING );
				if ( !node )
					return;
				node->StreamIn( in, tag );
				delete node;
				node = 0;

				// No return: go around from the beginning: text, closing tag, or node.
			}
		}
	}
}
#endif

const char* TiXmlElement::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	p = SkipWhiteSpace( p, encoding );
	TiXmlDocument* document = GetDocument();

	if ( !p || !*p )
	{
		if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, 0, 0, encoding );
		return 0;
	}

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}

	if ( *p != '<' )
	{
		if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, p, data, encoding );
		return 0;
	}

	p = SkipWhiteSpace( p+1, encoding );

	// Read the name.
	const char* pErr = p;

    p = ReadName( p, &value, encoding );
	if ( !p || !*p )
	{
		if ( document )	document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding );
		return 0;
	}

    TIXML_STRING endTag ("</");
	endTag += value;

	// Check for and read attributes. Also look for an empty
	// tag or an end tag.
	while ( p && *p )
	{
		pErr = p;
		p = SkipWhiteSpace( p, encoding );
		if ( !p || !*p )
		{
			if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
			return 0;
		}
		if ( *p == '/' )
		{
			++p;
			// Empty tag.
			if ( *p  != '>' )
			{
				if ( document ) document->SetError( TIXML_ERROR_PARSING_EMPTY, p, data, encoding );		
				return 0;
			}
			return (p+1);
		}
		else if ( *p == '>' )
		{
			// Done with attributes (if there were any.)
			// Read the value -- which can include other
			// elements -- read the end tag, and return.
			++p;
			p = ReadValue( p, data, encoding );		// Note this is an Element method, and will set the error if one happens.
			if ( !p || !*p ) {
				// We were looking for the end tag, but found nothing.
				// Fix for [ 1663758 ] Failure to report error on bad XML
				if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}

			// We should find the end tag now
			// note that:
			// </foo > and
			// </foo> 
			// are both valid end tags.
			if ( StringEqual( p, endTag.c_str(), false, encoding ) )
			{
				p += endTag.length();
				p = SkipWhiteSpace( p, encoding );
				if ( p && *p && *p == '>' ) {
					++p;
					return p;
				}
				if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}
			else
			{
				if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}
		}
		else
		{
			// Try to read an attribute:
			TiXmlAttribute* attrib = new TiXmlAttribute();
			if ( !attrib )
			{
				return 0;
			}

			attrib->SetDocument( document );
			pErr = p;
			p = attrib->Parse( p, data, encoding );

			if ( !p || !*p )
			{
				if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding );
				delete attrib;
				return 0;
			}

			// Handle the strange case of double attributes:
			#ifdef TIXML_USE_STL
			TiXmlAttribute* node = attributeSet.Find( attrib->NameTStr() );
			#else
			TiXmlAttribute* node = attributeSet.Find( attrib->Name() );
			#endif
			if ( node )
			{
				if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding );
				delete attrib;
				return 0;
			}

			attributeSet.Add( attrib );
		}
	}
	return p;
}


const char* TiXmlElement::ReadValue( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	TiXmlDocument* document = GetDocument();

	// Read in text and elements in any order.
	const char* pWithWhiteSpace = p;
	p = SkipWhiteSpace( p, encoding );

	while ( p && *p )
	{
		if ( *p != '<' )
		{
			// Take what we have, make a text element.
			TiXmlText* textNode = new TiXmlText( "" );

			if ( !textNode )
			{
			    return 0;
			}

			if ( TiXmlBase::IsWhiteSpaceCondensed() )
			{
				p = textNode->Parse( p, data, encoding );
			}
			else
			{
				// Special case: we want to keep the white space
				// so that leading spaces aren't removed.
				p = textNode->Parse( pWithWhiteSpace, data, encoding );
			}

			if ( !textNode->Blank() )
				LinkEndChild( textNode );
			else
				delete textNode;
		} 
		else 
		{
			// We hit a '<'
			// Have we hit a new element or an end tag? This could also be
			// a TiXmlText in the "CDATA" style.
			if ( StringEqual( p, "</", false, encoding ) )
			{
                // Ceetron modification
                // If whitespace exists, add a text node containing whitespace
                if ( !TiXmlBase::IsWhiteSpaceCondensed() )
                {
                    if (p != pWithWhiteSpace)
                    {
                        TiXmlText* textNode = new TiXmlText( "" );
                        p = textNode->Parse( pWithWhiteSpace, data, encoding );
                        LinkEndChild( textNode );
                    }
                }

                return p;
			}
			else
			{
				TiXmlNode* node = Identify( p, encoding );
				if ( node )
				{
					p = node->Parse( p, data, encoding );
					LinkEndChild( node );
				}				
				else
				{
					return 0;
				}
			}
		}
		pWithWhiteSpace = p;
		p = SkipWhiteSpace( p, encoding );
	}

	if ( !p )
	{
		if ( document ) document->SetError( TIXML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding );
	}	
	return p;
}


#ifdef TIXML_USE_STL
void TiXmlUnknown::StreamIn( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->get();	
		if ( c <= 0 )
		{
			TiXmlDocument* document = GetDocument();
			if ( document )
				document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}
		(*tag) += (char) c;

		if ( c == '>' )
		{
			// All is well.
			return;		
		}
	}
}
#endif


const char* TiXmlUnknown::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	TiXmlDocument* document = GetDocument();
	p = SkipWhiteSpace( p, encoding );

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	if ( !p || !*p || *p != '<' )
	{
		if ( document ) document->SetError( TIXML_ERROR_PARSING_UNKNOWN, p, data, encoding );
		return 0;
	}
	++p;
    value = "";

	while ( p && *p && *p != '>' )
	{
		value += *p;
		++p;
	}

	if ( !p )
	{
		if ( document )	document->SetError( TIXML_ERROR_PARSING_UNKNOWN, 0, 0, encoding );
	}
	if ( *p == '>' )
		return p+1;
	return p;
}

#ifdef TIXML_USE_STL
void TiXmlComment::StreamIn( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->get();	
		if ( c <= 0 )
		{
			TiXmlDocument* document = GetDocument();
			if ( document )
				document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}

		(*tag) += (char) c;

		if ( c == '>' 
			 && tag->at( tag->length() - 2 ) == '-'
			 && tag->at( tag->length() - 3 ) == '-' )
		{
			// All is well.
			return;		
		}
	}
}
#endif


const char* TiXmlComment::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	TiXmlDocument* document = GetDocument();
	value = "";

	p = SkipWhiteSpace( p, encoding );

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	const char* startTag = "<!--";
	const char* endTag   = "-->";

	if ( !StringEqual( p, startTag, false, encoding ) )
	{
		document->SetError( TIXML_ERROR_PARSING_COMMENT, p, data, encoding );
		return 0;
	}
	p += strlen( startTag );

	// [ 1475201 ] TinyXML parses entities in comments
	// Oops - ReadText doesn't work, because we don't want to parse the entities.
	// p = ReadText( p, &value, false, endTag, false, encoding );
	//
	// from the XML spec:
	/*
	 [Definition: Comments may appear anywhere in a document outside other markup; in addition, 
	              they may appear within the document type declaration at places allowed by the grammar. 
				  They are not part of the document's character data; an XML processor MAY, but need not, 
				  make it possible for an application to retrieve the text of comments. For compatibility, 
				  the string "--" (double-hyphen) MUST NOT occur within comments.] Parameter entity 
				  references MUST NOT be recognized within comments.

				  An example of a comment:

				  <!-- declarations for <head> & <body> -->
	*/

    value = "";
	// Keep all the white space.
	while (	p && *p && !StringEqual( p, endTag, false, encoding ) )
	{
		value.append( p, 1 );
		++p;
	}
	if ( p && *p ) 
		p += strlen( endTag );

	return p;
}


const char* TiXmlAttribute::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p ) return 0;

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	// Read the name, the '=' and the value.
	const char* pErr = p;
	p = ReadName( p, &name, encoding );
	if ( !p || !*p )
	{
		if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
		return 0;
	}
	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p || *p != '=' )
	{
		if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
		return 0;
	}

	++p;	// skip '='
	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p )
	{
		if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
		return 0;
	}
	
	const char* end;
	const char SINGLE_QUOTE = '\'';
	const char DOUBLE_QUOTE = '\"';

	if ( *p == SINGLE_QUOTE )
	{
		++p;
		end = "\'";		// single quote in string
		p = ReadText( p, &value, false, end, false, encoding );
	}
	else if ( *p == DOUBLE_QUOTE )
	{
		++p;
		end = "\"";		// double quote in string
		p = ReadText( p, &value, false, end, false, encoding );
	}
	else
	{
		// All attribute values should be in single or double quotes.
		// But this is such a common error that the parser will try
		// its best, even without them.
		value = "";
		while (    p && *p											// existence
				&& !IsWhiteSpace( *p )								// whitespace
				&& *p != '/' && *p != '>' )							// tag end
		{
			if ( *p == SINGLE_QUOTE || *p == DOUBLE_QUOTE ) {
				// [ 1451649 ] Attribute values with trailing quotes not handled correctly
				// We did not have an opening quote but seem to have a 
				// closing one. Give up and throw an error.
				if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
				return 0;
			}
			value += *p;
			++p;
		}
	}
	return p;
}

#ifdef TIXML_USE_STL
void TiXmlText::StreamIn( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->peek();	
		if ( !cdata && (c == '<' ) ) 
		{
			return;
		}
		if ( c <= 0 )
		{
			TiXmlDocument* document = GetDocument();
			if ( document )
				document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}

		(*tag) += (char) c;
		in->get();	// "commits" the peek made above

		if ( cdata && c == '>' && tag->size() >= 3 ) {
			size_t len = tag->size();
			if ( (*tag)[len-2] == ']' && (*tag)[len-3] == ']' ) {
				// terminator of cdata.
				return;
			}
		}    
	}
}
#endif

const char* TiXmlText::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	value = "";
	TiXmlDocument* document = GetDocument();

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}

	const char* const startTag = "<![CDATA[";
	const char* const endTag   = "]]>";

	if ( cdata || StringEqual( p, startTag, false, encoding ) )
	{
		cdata = true;

		if ( !StringEqual( p, startTag, false, encoding ) )
		{
			document->SetError( TIXML_ERROR_PARSING_CDATA, p, data, encoding );
			return 0;
		}
		p += strlen( startTag );

		// Keep all the white space, ignore the encoding, etc.
		while (	   p && *p
				&& !StringEqual( p, endTag, false, encoding )
			  )
		{
			value += *p;
			++p;
		}

		TIXML_STRING dummy; 
		p = ReadText( p, &dummy, false, endTag, false, encoding );
		return p;
	}
	else
	{
		bool ignoreWhite = true;

		const char* end = "<";
		p = ReadText( p, &value, ignoreWhite, end, false, encoding );
		if ( p )
			return p-1;	// don't truncate the '<'
		return 0;
	}
}

#ifdef TIXML_USE_STL
void TiXmlDeclaration::StreamIn( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->get();
		if ( c <= 0 )
		{
			TiXmlDocument* document = GetDocument();
			if ( document )
				document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}
		(*tag) += (char) c;

		if ( c == '>' )
		{
			// All is well.
			return;
		}
	}
}
#endif

const char* TiXmlDeclaration::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding _encoding )
{
	p = SkipWhiteSpace( p, _encoding );
	// Find the beginning, find the end, and look for
	// the stuff in-between.
	TiXmlDocument* document = GetDocument();
	if ( !p || !*p || !StringEqual( p, "<?xml", true, _encoding ) )
	{
		if ( document ) document->SetError( TIXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding );
		return 0;
	}
	if ( data )
	{
		data->Stamp( p, _encoding );
		location = data->Cursor();
	}
	p += 5;

	version = "";
	encoding = "";
	standalone = "";

	while ( p && *p )
	{
		if ( *p == '>' )
		{
			++p;
			return p;
		}

		p = SkipWhiteSpace( p, _encoding );
		if ( StringEqual( p, "version", true, _encoding ) )
		{
			TiXmlAttribute attrib;
			p = attrib.Parse( p, data, _encoding );		
			version = attrib.Value();
		}
		else if ( StringEqual( p, "encoding", true, _encoding ) )
		{
			TiXmlAttribute attrib;
			p = attrib.Parse( p, data, _encoding );		
			encoding = attrib.Value();
		}
		else if ( StringEqual( p, "standalone", true, _encoding ) )
		{
			TiXmlAttribute attrib;
			p = attrib.Parse( p, data, _encoding );		
			standalone = attrib.Value();
		}
		else
		{
			// Read over whatever it is.
			while( p && *p && *p != '>' && !IsWhiteSpace( *p ) )
				++p;
		}
	}
	return 0;
}

bool TiXmlText::Blank() const
{
	for ( unsigned i=0; i<value.length(); i++ )
		if ( !IsWhiteSpace( value[i] ) )
			return false;
	return true;
}
 
// tinyxmlerror.cpp

/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

//#include "tinyxml.h"

// The goal of the seperate error file is to make the first
// step towards localization. tinyxml (currently) only supports
// english error messages, but the could now be translated.
//
// It also cleans up the code a bit.
//

const char* TiXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
    "No error",
    "Error",
    "Failed to open file",
    "Error parsing Element.",
    "Failed to read Element name",
    "Error reading Element value.",
    "Error reading Attributes.",
    "Error: empty tag.",
    "Error reading end tag.",
    "Error parsing Unknown.",
    "Error parsing Comment.",
    "Error parsing Declaration.",
    "Error document empty.",
    "Error null (0) or unexpected EOF found in input stream.",
    "Error parsing CDATA.",
    "Error when TiXmlDocument added to document, because TiXmlDocument can only be at the root.",
}; 

} // end namespace cvf_tinyXML


// End of Doxygen conditional section
/// \endcond

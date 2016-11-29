from cwrap import BaseCClass
from opm import OPMPrototype

class Deck(BaseCClass):
    TYPE_NAME = "deck"
    _alloc              = OPMPrototype("void* deck_alloc()" , bind = False)
    _free               = OPMPrototype("void  deck_free(deck)")
    _size               = OPMPrototype("int   deck_size(deck)")
    _iget_keyword       = OPMPrototype("deck_keyword_ref  deck_iget_keyword(deck , int)")
    _iget_named_keyword = OPMPrototype("deck_keyword_ref  deck_iget_named_keyword(deck , char* , int)")
    _has_keyword        = OPMPrototype("bool  deck_has_keyword(deck , char*)")
    _num_keywords       = OPMPrototype("int   deck_num_keywords(deck , char*)")
    
    
    def __init__(self):
        c_ptr = self._alloc( )
        super(Deck , self).__init__( c_ptr )

        
    def __len__(self):
        """
        Will return the number of keywords in the deck.
        """
        return self._size()


    def __contains__(self , keyword):
        """
        Will return True if the deck has at least one occurence of @keword.
        """
        return self._has_keyword(keyword)


    def numKeywords(self , keyword):
        """
        Will count the number of occurences of @keyword.
        """
        return self._num_keywords(keyword)


    def __igetKeyword(self , index):
        keyword = self._iget_keyword(index )
        keyword.setParent( self )
        return keyword

    
    def __igetNamedKeyword(self , name , index):
        keyword = self._iget_named_keyword( name , index )
        keyword.setParent( self )
        return keyword
        

    def __getitem__(self, index):
        """Implements the [] operator for the deck.

        The @index variable can be either an integegr, i.e. deck[2]
        will return the third keyword in the deck. The integer based
        indexing supports slicing; in that case a normal Python list
        of keywords will be returned.

        Alternatively the @index can be a string: deck["PORO"] will
        return a Python list of all the PORO keywords in the
        deck. Finally the @index can be a tuple of ("STRING",int) to
        get directly get a named keyword instance:

           deck["PORO,0]
 
        will return the first PORO keyword. The tuple syntax supports
        negative indexing.
        """
        if isinstance(index , int):
            if index < 0:
                index += len(self)

            if 0 <= index < len(self):
                return self.__igetKeyword( index )
            else:
                raise IndexError
        elif isinstance(index , slice):
            (start , step, stop) = index.indices( len(self) )
            keyword_list = []
            print start,stop,step
            for i in range(start,stop,step):
                keyword_list.append( self.__igetKeyword( i ))
            return keyword_list
        elif isinstance(index , str):
            if index in self:
                keyword_list = []
                for i in range(self.numKeywords( index )):
                    keyword_list.append( self.__igetNamedKeyword(index , i))
                return keyword_list
            else:
                raise KeyError("No %s keyword in deck" % index)
        elif isinstance(index,tuple):
            if len(index) == 2:
                kw = index[0]
                int_index = index[1]
                if isinstance(kw , str) and isinstance(int_index , int):
                    if kw in self:
                        num_kw = self.numKeywords( kw )
                        if int_index < num_kw:
                            if int_index < 0:
                                int_index += num_kw
                            return self.__igetNamedKeyword(kw , int_index)
                        else:
                            raise IndexError("Not so many %s keywords" % kw)
                    else:
                        raise KeyError("No %s keyword in deck" % kw)
                else:
                    raise TypeError("Tuple must be two elements : (string,int)")
            else:
                raise TypeError("Tuple must be two elements : (string,int)")
        else:
            raise TypeError("Index must integer or string")
        

        
    def free(self):
        self._free(  )


        

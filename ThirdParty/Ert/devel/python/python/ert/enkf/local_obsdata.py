from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, LocalObsdataNode 


class LocalObsdata(BaseCClass):

    def __init__(self, name , obs = None):
        # The obs instance should be a EnkFObs instance; some circular dependency problems
        # by importing it right away. It is not really optional, but it is made optional
        # here to be able to give a decent error message for old call sites which did not
        # supply the obs argument.
        if obs is None:
            msg = """

The LocalObsdata constructor has recently changed, as a second
argument you should pass the EnkFObs instance with all the
observations. You can typically get this instance from the ert main
object as:

    obs = ert.getObservations()
    local_obs = LocalObsData("YOUR-KEY" , obs)

"""
            raise Exception( msg )

        assert isinstance(name, str)
        
        c_pointer = LocalObsdata.cNamespace().alloc(name)
        super(LocalObsdata, self).__init__(c_pointer)
        self.initObservations( obs )

        
    def initObservations(self , obs):
        self.obs = obs
        
    def __len__(self):
        """ @rtype: int """
        return LocalObsdata.cNamespace().size(self)


    def __getitem__(self, key):
        """ @rtype: LocalObsdataNode """
        if isinstance(key , int):
            if int < len:
                return LocalObsdata.cNamespace().iget_node(self, key).setParent(self)
            else:
                raise IndexError("Invalid index")
        else:
            if key in self:
                return LocalObsdata.cNamespace().get_node(self, key).setParent(self)
            else:
                raise KeyError("Unknown key:%s" % key)
        
    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1
                    
    def __contains__(self, item):
        """ @rtype: bool """
        if isinstance(item, str):
            return LocalObsdata.cNamespace().has_node(self, item)
        elif isinstance(item, LocalObsdataNode):
            return LocalObsdata.cNamespace().has_node(self, item.getKey())

        return False
    
    def __delitem__(self, key):
        assert isinstance(key, str)
        if key in self:
            LocalObsdata.cNamespace().del_node(self, key)  
        else:
            raise KeyError("Unknown key:%s" % key)                
    
    def addNode(self, key, add_all_timesteps = True):
        """ @rtype: LocalObsdataNode """           
        assert isinstance(key, str)
        if key in self.obs:
            node = LocalObsdataNode(key , add_all_timesteps) 
            if node not in self:
                node.convertToCReference(self)
                LocalObsdata.cNamespace().add_node(self, node)
                return node
            else:
                raise KeyError("Tried to add existing observation key:%s " % key)
        else:
            raise KeyError("The observation node: %s is not recognized observation key" % key)


    def addNodeAndRange(self, key, step_1, step_2):
        """ @rtype: LocalObsdataNode """        
        """ The time range will be removed in the future... """
        assert isinstance(key, str)
        assert isinstance(step_1, int)
        assert isinstance(step_2, int)                     
        node = self.addNode( key )
        node.addRange(step_1, step_2)
        return node

    
    def clear(self):        
        LocalObsdata.cNamespace().clear(self)        

        
    def addObsVector(self , obs_vector):
        self.addNode( obs_vector.getObservationKey() )

        
    def getName(self):
        """ @rtype: str """
        return LocalObsdata.cNamespace().name(self)
    
    def getActiveList(self, key):
        """ @rtype: ActiveList """
        if key in self:
            return LocalObsdata.cNamespace().active_list(self , key)
        else:
            raise KeyError("Local key:%s not recognized" % key)      

    def free(self):
        LocalObsdata.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("local_obsdata", LocalObsdata)

LocalObsdata.cNamespace().alloc     = cwrapper.prototype("c_void_p local_obsdata_alloc(char*)")
LocalObsdata.cNamespace().free      = cwrapper.prototype("void local_obsdata_free(local_obsdata)")
LocalObsdata.cNamespace().size      = cwrapper.prototype("int local_obsdata_get_size(local_obsdata)")
LocalObsdata.cNamespace().has_node  = cwrapper.prototype("bool local_obsdata_has_node(local_obsdata, char*)")
LocalObsdata.cNamespace().add_node  = cwrapper.prototype("bool local_obsdata_add_node(local_obsdata, local_obsdata_node)")
LocalObsdata.cNamespace().del_node  = cwrapper.prototype("void local_obsdata_del_node(local_obsdata, char*)")
LocalObsdata.cNamespace().clear     = cwrapper.prototype("void local_dataset_clear(local_obsdata)")
LocalObsdata.cNamespace().iget_node = cwrapper.prototype("local_obsdata_node_ref local_obsdata_iget(local_obsdata, int)")
LocalObsdata.cNamespace().get_node  = cwrapper.prototype("local_obsdata_node_ref local_obsdata_get(local_obsdata, char*)")
LocalObsdata.cNamespace().name      = cwrapper.prototype("char* local_obsdata_get_name(local_obsdata)")
LocalObsdata.cNamespace().active_list    = cwrapper.prototype("active_list_ref local_obsdata_get_node_active_list(local_obsdata, char*)")




from ert.test import ExtendedTestCase, ErtTestContext,TestAreaContext
from ert.config import *
from ert.enkf import PlotSettings
from ert.enkf import EnkfPrototype

#_plot_settings_init = EnkfPrototype("void plot_settings_init( plot_settings, config_content)", bind = False)
_plot_settings_config = EnkfPrototype("void plot_settings_add_config_items( config_parser)", bind = False)




class PlotSettingsTest(ExtendedTestCase):


    def test_create(self):
        ps = PlotSettings( )
        

    def test_keys(self):
        ps = PlotSettings( )
        keys = ps.keys( )
        self.assertFalse( "XXX" in keys )
        self.assertTrue( "PATH" in keys )
        self.assertTrue( "SHOW_REFCASE" in keys )
    

    def test_set_get(self):
        ps = PlotSettings( )
        with self.assertRaises(KeyError):
            ps["UNKNOWN_KEY"] = 1000

        with self.assertRaises(TypeError):
            ps["SHOW_REFCASE"] = "Don-know"

        ps["SHOW_REFCASE"] = False
        self.assertEqual( ps["SHOW_REFCASE"] , False )

        ps["SHOW_REFCASE"] = True
        self.assertEqual( ps["SHOW_REFCASE"] , True )


        
    def test_config(self):
        parser = ConfigParser( )
        ps = PlotSettings( )

        with TestAreaContext("plot_config"):
            with open("config_file" , "w") as f:
                f.write("PLOT_SETTING PATH abc\n");
                f.write("PLOT_SETTING SHOW_REFCASE False\n")
                f.write("PLOT_SETTING UnknownKey Value\n")

            #_plot_settings_config( parser )
            content = parser.parse( "config_file" )
            ps.apply( content )
            #_plot_settings_init( ps , content )

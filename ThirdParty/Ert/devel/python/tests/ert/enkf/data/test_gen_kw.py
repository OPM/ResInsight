import os.path
from ert.enkf.data import GenKw
from ert.enkf.config import GenKwConfig
from ert.test import TestAreaContext, ExtendedTestCase


def create_gen_kw():
    parameter_file ="MULTFLT.txt"
    template_file ="MULTFLT.tmpl"
    with open(parameter_file, "w") as f:
        f.write("MULTFLT1  NORMAL  0   1\n")
        f.write("MULTFLT2  RAW \n")
        f.write("MULTFLT3  NORMAL  0   1\n")

    with open(template_file, "w") as f:
        f.write("<MULTFLT1> <MULTFLT2> <MULTFLT3>\n")
        f.write("/\n")
        

    gen_kw_config = GenKwConfig("MULTFLT", template_file , parameter_file)
    gen_kw = GenKw( gen_kw_config )
    
    return (gen_kw_config , gen_kw)



class GenKwTest(ExtendedTestCase):

    def test_gen_kw_get_set(self):
        with TestAreaContext("enkf/data/gen_kwt"):
            
            (gen_kw_config , gen_kw) = create_gen_kw()
            self.assertIsInstance(gen_kw, GenKw)

            gen_kw[0] = 3.0
            self.assertEqual(gen_kw[0], 3.0)

            gen_kw["MULTFLT1"] = 4.0
            self.assertEqual(gen_kw["MULTFLT1"], 4.0)
            self.assertEqual(gen_kw[0], 4.0)

            gen_kw["MULTFLT2"] = 8.0
            self.assertEqual(gen_kw["MULTFLT2"], 8.0)
            self.assertEqual(gen_kw[1], 8.0)

            gen_kw["MULTFLT3"] = 12.0
            self.assertEqual(gen_kw["MULTFLT3"], 12.0)
            self.assertEqual(gen_kw[2], 12.0)

            self.assertEqual(len(gen_kw), 3)

            with self.assertRaises(IndexError):
                gen_kw[4]

            with self.assertRaises(TypeError):
                gen_kw[1.5]

            with self.assertRaises(KeyError):
                gen_kw["MULTFLT_2"]

            self.assertTrue("MULTFLT1" in gen_kw )



    def test_gen_kw_get_set_vector(self):
        with TestAreaContext("enkf/data/gen_kwt"):
            
            (gen_kw_config , gen_kw) = create_gen_kw()
            with self.assertRaises(ValueError):
                gen_kw.setValues([0])

            with self.assertRaises(TypeError):
                gen_kw.setValues(["A","B","C"])
                
            gen_kw.setValues([0,1,2])
            self.assertEqual(gen_kw[0], 0)
            self.assertEqual(gen_kw[1], 1)
            self.assertEqual(gen_kw[2], 2)

            self.assertEqual(gen_kw["MULTFLT1"] , 0)
            self.assertEqual(gen_kw["MULTFLT2"] , 1)
            self.assertEqual(gen_kw["MULTFLT3"] , 2)
    

        
    

    def test_gen_kw_ecl_write(self):
        with TestAreaContext("enkf/data/gen_kwt"):
            (gen_kw_config , gen_kw) = create_gen_kw()
    
            with self.assertRaises(IOError):
                gen_kw.eclWrite( "tmp" , "file.txt")

            gen_kw.eclWrite( None , "file.txt")
            self.assertTrue( os.path.isfile( "file.txt" ))
            
            os.mkdir("tmp")
            gen_kw.eclWrite( "tmp" , "file.txt")
            self.assertTrue( os.path.isfile( "tmp/file.txt" ))

            gen_kw.eclWrite( "tmp" , "file.txt" , "tmp/export.txt")
            self.assertTrue( os.path.isfile( "tmp/export.txt" ))






















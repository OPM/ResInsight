#set( ENV{PYTHONPATH}  ${pbd}/python)
execute_process(COMMAND cmake -E copy ${ccsd}/conf.py ${pbd}/tmp_doc/conf.py )


# The api documentation is reeferenced from code/python/index.rst, i.e. the output path used when calling
# sphinx-apidoc must match this.
execute_process(COMMAND sphinx-apidoc -e -o API/python ${pbd}/python WORKING_DIRECTORY ${pbd}/tmp_doc/)
execute_process(COMMAND sphinx-build -b html -d _build/doctrees . _build WORKING_DIRECTORY ${pbd}/tmp_doc/)
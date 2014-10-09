set( ENV{PYTHONPATH}  ${pbd}/python)

execute_process(COMMAND cmake -E copy ${ccsd}/conf.py ${pbd}/tmp_doc/conf.py )
execute_process(COMMAND cmake -E copy ${ccsd}/index.rst ${pbd}/tmp_doc/index.rst )
execute_process(COMMAND sphinx-apidoc -e -o python ${pbd}/python WORKING_DIRECTORY ${pbd}/tmp_doc/)
execute_process(COMMAND sphinx-build -b html -d _build/doctrees . _build WORKING_DIRECTORY ${pbd}/tmp_doc/)
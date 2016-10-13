set(PYTHON_SOURCES
    __init__.py
    activelabel.py
    analysismoduleselector.py
    analysismodulevariablespanel.py
    caselist.py
    caseselector.py
    checklist.py
    closabledialog.py
    customdialog.py
    legend.py
    listeditbox.py
    pathchooser.py
    searchbox.py
    stringbox.py
    summarypanel.py
    validateddialog.py
    validationsupport.py
)

add_python_package("python.ert_gui.ertwidgets" ${PYTHON_INSTALL_PREFIX}/ert_gui/ertwidgets "${PYTHON_SOURCES}" True)

add_subdirectory(models)

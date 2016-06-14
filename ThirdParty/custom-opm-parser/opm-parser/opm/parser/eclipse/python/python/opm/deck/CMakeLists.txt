set(PYTHON_SOURCES
    __init__.py
    deck.py
    deck_keyword.py
    deck_record.py
    deck_item.py
    item_type_enum.py
)

add_python_package("opm.deck"  ${PYTHON_INSTALL_PREFIX}/opm/deck "${PYTHON_SOURCES}" True)

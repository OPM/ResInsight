set (TEST_SOURCES
    __init__.py
    test_deck.py
    test_deck_keyword.py
    test_deck_record.py
    test_deck_item.py )

add_python_package("python.tests.deck" "${PYTHON_INSTALL_PREFIX}/tests/deck" "${TEST_SOURCES}" False)

addPythontest( python.test_deck tests.deck.test_deck.DeckTest )
addPythontest( python.test_deck_keyword tests.deck.test_deck_keyword.DeckKeywordTest )
addPythontest( python.test_deck_record tests.deck.test_deck_record.DeckRecordTest )
addPythontest( python.test_deck_item tests.deck.test_deck_item.DeckItemTest )

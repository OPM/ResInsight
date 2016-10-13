.. _ert_magic_strings_full_doc:

Magic Strings
===================================

Magic strings are special keywords that can be used in configuration files and templates which have special meanings.

ERT Config
----------

These magic strings are available in the ERT config file.

**<CONFIG_FILE>**

Inserts the config file name with extension into the config file.

::

	ENSPATH storage/<CONFIG_FILE>/ensemble

If the config file is named *config.ert* then the resulting storage path will be:

::

	storage/config.ert/ensemble


**<CONFIG_FILE_BASE>**

Same as <CONFIG_FILE> but inserts the config file name without the extension into the config file.

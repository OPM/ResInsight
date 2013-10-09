import os
import re

class SourceEnumerator(object):
    @classmethod
    def findDevRoot(cls, root_directory_name = "devel"):
        dev_root = os.path.dirname(os.path.realpath(__file__))

        while os.path.basename(dev_root) != root_directory_name:
            head, tail = os.path.split(dev_root)
            dev_root = head
            if tail == "":
                raise ValueError("Source root: '%s' not found!" % root_directory_name)

        return dev_root

    @classmethod
    def findSourceFile(cls, path):
        dev_root = SourceEnumerator.findDevRoot()

        source_file = os.path.join(dev_root, path)

        if not os.path.exists(source_file):
            raise ValueError("File not found: %s" % path)

        return source_file

    @classmethod
    def removeComments(cls, code_string):
        code_string = re.sub(re.compile("/\*.*?\*/",re.DOTALL ) ,"" ,code_string) # remove all occurance streamed comments (/*COMMENT */) from string
        code_string = re.sub(re.compile("//.*?\n" ) ,"" ,code_string) # remove all occurance singleline comments (//COMMENT\n ) from string
        return code_string

    @classmethod
    def findEnum(cls, enum_name, full_source_file_path):
        with open(full_source_file_path, "r") as f:
            text = f.read()

        text = SourceEnumerator.removeComments(text)

        enum_pattern = re.compile("typedef\s+enum\s+\{(.*?)\}\s*(\w+?);", re.DOTALL)

        for enum in enum_pattern.findall(text):
            if enum[1] == enum_name:
                return enum[0]

        raise ValueError("Enum with name: '%s' not found!" % enum_name)


    @classmethod
    def findEnumerators(cls, enum_name, source_file_path):
        enum_text = SourceEnumerator.findEnum(enum_name, SourceEnumerator.findSourceFile(source_file_path))

        enumerator_pattern = re.compile("(\w+?)\s*?=\s*?(\d+)")

        enumerators = []
        for enumerator in enumerator_pattern.findall(enum_text):
            enumerators.append((enumerator[0], int(enumerator[1])))

        return enumerators

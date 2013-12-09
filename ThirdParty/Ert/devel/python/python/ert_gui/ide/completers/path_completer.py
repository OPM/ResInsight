import os


class PathItem(object):
    def __init__(self, value):
        super(PathItem, self).__init__()
        #: :type: str
        self.value = value


    def __str__(self):
        return self.value

    def __gt__(self, other):
        assert isinstance(other, PathItem)

        if self.value.endswith("/") and not other.value.endswith("/"):
            return True
        elif not self.value.endswith("/") and other.value.endswith("/"):
            return False
        else:
            return self.value.lower() > other.value.lower()

    def __eq__(self, other):
        return self.value == other.value

    def __lt__(self, other):
        return not (self == other or self > other)

    def __le__(self, other):
        return self == other or not self > other

    def __ge__(self, other):
        return self == other or self > other



class PathCompleter(object):
    def __init__(self):
        super(PathCompleter, self).__init__()


    def completeOptions(self, path_prefix):

        root, entry_prefix = os.path.split(path_prefix)

        if os.path.isdir(root):
            if not root.startswith("/"):
                root_path = "./%s/" % root
            else:
                root_path = "%s/" % root
        else:
            if root == "":
                root_path = "./"
            else:
                root_path = root

        entries = os.listdir(root_path)


        result = []
        for entry in entries:
            if entry.startswith(entry_prefix):
                full_path = os.path.join(root, entry)
                if os.path.isdir(full_path):
                    full_path += "/"
                result.append(PathItem(full_path))


        result = sorted(result)
        return [str(item) for item in result]

import fnmatch
from math import floor
import shlex


def autoCompleteList(text, items):
    if not text:
        completions = items
    else:
        completions = [item for item in items if str(item).lower().startswith(text.lower())]
    return completions

def containsAny(string, chars):
    return True in [char in string for char in chars]

def findRightMostSeparator(text, separators):
    max_pos = 0

    for separator in separators:
        pos = text.rfind(separator)
        max_pos = max(pos, max_pos)

    return max_pos

def autoCompleteListWithSeparator(text, items, separators=":,@"):
    if containsAny(text, separators):
        auto_complete_list = autoCompleteList(text, items)
        separator_pos = findRightMostSeparator(text, separators)
        auto_complete_list = [item[separator_pos + 1:] for item in auto_complete_list]
    else:
        auto_complete_list = autoCompleteList(text, items)

    return auto_complete_list


def extractFullArgument(line, endidx):
    last_space_index = line.rfind(" ", 0, endidx)
    newstart = 0 if last_space_index == -1 else last_space_index
    return line[newstart:endidx].strip()


def matchItems(line, items):
    patterns = shlex.split(line)

    result_items = set()

    for pattern in patterns:
        pattern_matches = set()
        for item in items:
            if fnmatch.fnmatch(item.lower(), pattern.lower()): # case-insensitive matching
                pattern_matches.add(item)

        if len(pattern_matches) == 0:
            print("Error: Name/Pattern '%s' does not match anything." % pattern)
        else:
            result_items = result_items | pattern_matches

    return result_items


def boolValidator(model, value):
    trueness = value.lower() in ("yes", "true", "t", "1")
    falseness = value.lower() in ("no", "false", "f", "0")

    if not (trueness or falseness):
        raise ValueError("Unable to convert '%s' into a boolean expression true|false" % value)

    return trueness


def createFloatValidator(minimum=None, maximum=None):
    def validate(model, value):
        value = float(value)
        if minimum is not None:
            value = max(value, minimum)

        if maximum is not None:
            value = min(value, maximum)

        return value
    return validate


def createListValidator(items):
    def validate(model, value):
        value = value.lower()
        for item in items:
            if value == str(item).lower():
                return item

        raise ValueError("Value '%s' not in collection: %s" % (value, items))

    return validate

def splitArguments(line):
    """ @rtype: list of str """
    return shlex.split(line)

def widthAsPercentageOfConsoleWidth(percentage):
    width, height = getTerminalSize()
    if width == 0:
        width = 80

    return int(floor(percentage * width / 100.0))


def getTerminalSize():
    """
     @rtype: tuple of (int,int)
     @return: Console dimensions as: width, height
    """
    import os
    env = os.environ

    def ioctl_GWINSZ(fd):
        try:
            import fcntl, termios, struct, os
            cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
        except:
            return
        return cr

    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)

    if not cr:
        try:
            fd = os.open(os.ctermid(), os.O_RDONLY)
            cr = ioctl_GWINSZ(fd)
            os.close(fd)
        except:
            pass

    if not cr:
        cr = (env.get('LINES', 25), env.get('COLUMNS', 80))
    return int(cr[1]), int(cr[0])

